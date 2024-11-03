#include <bcm2835.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <fftw3.h>
#include <signal.h>
#include <math.h>
#include <sys/types.h>
#include "fdacoefs.h"

// Global variables
//settings variables
#define BUFFER_SIZE 1024  // Set this to the desired buffer size
double thresh_fmax_mag = 100000000;  //magnitude threshold for the frequency of the plant sound, trial and error
double sampling_freq = 500000; //500kHz
double thresholdv = 0.4;  //V , 140mV should be the max value of plant sound 
double threshold = 0;  // threshold value ADC is calculated in the main function
double lowerfreq = 36000;
double upperfreq = 60000;


// other variables
char tbuf[] = {0x01, 0x02}; // Data to send
char rbuf[] = {0x00, 0x00};

bool trigger = false;

fftw_plan plan;
fftw_complex* fft_result;

pid_t pid;

double buffer[BUFFER_SIZE];
double bufferc[BUFFER_SIZE]; //buffer centred
int buffer_index = 0;
bool triggered = false;

double sum = 0;
int count = 0;
double average;
double sample=0;
uint16_t sample16=0;


int index_trig = 0;
int index_end = 0;
int index_start = 0;

double freqmaxmain = 0;
double fmax_magmain = 0;
bool pumpdone = true;

//function prototypes
void *setupbcm2835(void *arg);
void handle_sigint(int sig);
void removeDCOffset(double* buffer, double average);
void fftprocess(double* bufferc, double* freqmax, double* fmax_mag);
// Define the signal handler function
void handle_sigusr1(int signum) {
    // Add your routine here...
    
    pumpdone = !pumpdone;
    printf("\nC/ Received SIGUSR1, pump done = %s\n", pumpdone ? "true" : "false");
}


int main(int argc, char *argv[])
{
    // Check if a PID was provided
    if (argc < 2) {
        printf("\nC/ Please provide the PID of the Python script");
        return 1;
    }
    // Convert the PID to an integer
    pid = atoi(argv[1]);
    printf("\nC/ PID of python in main: %d", pid);

    // Set up the signal handler
    signal(SIGINT, handle_sigint);
    signal(SIGUSR1, handle_sigusr1);
    setupbcm2835(NULL);
    sleep(5); //wait for python to be ready
    // Allocate FFTW complex output array
    fft_result = fftw_alloc_complex(BUFFER_SIZE);
    // if (fft_result == NULL) {
    //     printf("Failed to allocate memory for FFTW output.\n");
    //     exit(1);
    // }
    //threshold calculation in ADC value 
    threshold = thresholdv*4095/3.3;  // equivalent ADC value 
    printf("\nC/ Threshold(V): %f", thresholdv);
    printf("\nC/ Threshold(ADC): %f", threshold);
    printf("\nSYSTEM READY\n\n");
    
    while(1){
        while(pumpdone){

            bcm2835_spi_transfernb(tbuf, rbuf, sizeof(tbuf));
            sample16 = (0x0FFF & ((((uint16_t)(rbuf[0])<<8) | ((uint16_t)rbuf[1]))>>2));
            sample = (double) sample16;  
            
            buffer[buffer_index] = sample;
            buffer_index = (buffer_index + 1) % BUFFER_SIZE;

            sum += sample;
            count = count < BUFFER_SIZE ? count + 1 : BUFFER_SIZE;
            average = sum / count;

            // Check if the signal is above the threshold
            if (!triggered && (sample > (average + threshold))) {
                // printf("average: %f\n", average);
                // printf("threshold: %f\n", threshold);
                // printf("average + threshold: %f\n", average + threshold);
                // printf("sample: %f\n", sample);
                //printf("\nC/ Triggered by amp time signal\n");
                triggered = true;
                index_trig = (buffer_index-1+BUFFER_SIZE) % BUFFER_SIZE;
                //record target index
                index_end = (index_trig-200 + BUFFER_SIZE) % BUFFER_SIZE;
                index_start = (index_end +1) % BUFFER_SIZE;
            }

            //this will run if data has fininshed recording enough samples after the trigger
            if (triggered && buffer_index == index_end) {
                
                //copy the buffer to a new buffer that is centred around the trigger
                for (int i = 0; i < BUFFER_SIZE; i++) {
                    bufferc[i] = buffer[(index_start + i) % BUFFER_SIZE];
                }
                
                removeDCOffset(bufferc, average);
                
                fftprocess(bufferc, &freqmaxmain, &fmax_magmain);

                //check if correspond to the plant sound frequency
                if (lowerfreq<freqmaxmain && freqmaxmain<upperfreq){

                    //printf("\n\nC/ Triggered by right freq band");
                    if (fmax_magmain>thresh_fmax_mag)
                    {
                        printf("\nC/ Plant sound detected\n");
                        //printf("\nC/ Triggered by freq magnitude so plant sound");
                        //printf("\nC/ Frequency of max : %f kHz", freqmaxmain/1000);
                        //printf("\nC/ Magnitude of max : %f k amplitude square", fmax_magmain/1000);
                        //sleep(1);
                        kill(pid, SIGUSR1); //send signal to python

                    }

                }
                
                // Reset for the next trigger
                triggered = false;
            }

            // Remove the oldest sample from the sum when the buffer is full
            if (count == BUFFER_SIZE) {
                sum -= buffer[buffer_index];
            }
        }
    }
    return 0;
}

void removeDCOffset(double* bufferc, double average_given) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        bufferc[i] -= average_given;
    }
}

void fftprocess(double* bufferc, double* freqmax, double* fmax_mag) {

    plan = fftw_plan_dft_r2c_1d(BUFFER_SIZE, bufferc, fft_result, FFTW_ESTIMATE);
    fftw_execute(plan);

    //find the index of the maximum value in the FFT result
    double maxval = 0; //max magnitude of fftout
    int freqmaxindex = 0; //index of max magnitude of fftout
    double fftout[BUFFER_SIZE]; //output of fft

    for (int i = 0; i < BUFFER_SIZE/2; i++) {
        // Only need to check the first half of the array for positive frequencies
        // Calculate the magnitude of the complex number, no sqrt to save time
        //fftout[i] = sqrt(fft_result[i][0] * fft_result[i][0] + fft_result[i][1] * fft_result[i][1]);
        fftout[i] = fft_result[i][0] * fft_result[i][0] + fft_result[i][1] * fft_result[i][1];
        if (fftout[i] > maxval) {
            maxval = fftout[i];
            freqmaxindex = i;
        }
    }
    //convert index to frequency
    *freqmax = (double)freqmaxindex * sampling_freq / BUFFER_SIZE;
    *fmax_mag = maxval;

}

void *setupbcm2835(void *arg) {
    if (!bcm2835_init())
    {
      printf("C/ bcm2835_init failed. Are you running as root??\n");
      return NULL;
    }

    if (!bcm2835_spi_begin())
    {
      printf("C/ bcm2835_spi_begin failed. Are you running as root??\n");
      return NULL;
    }

    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
    //bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_65536); // The default
    uint32_t freq = 8000000;
    bcm2835_spi_set_speed_hz(freq);
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default

    return NULL;
}

// This function will be called when SIGINT is received
void handle_sigint(int sig) {
    printf("\nC/ Caught signal %d, will exit safely\n", sig);
    //sleep(1);
               /* printf("FFT Result:\n");
            for (int i = 0; i < samplemax/2; i++) {
                printf("Index %d: Real = %f, Imaginary = %f\n", i, fft_result[i][0], fft_result[i][1]);
            }*/

    // Free memory
    fftw_free(fft_result);
    fftw_cleanup();
    // End SPI and close BCM2835
    sleep(1); //seems to correct the issue with segmentation fault when closing bcm2835
    bcm2835_spi_end();
    bcm2835_close();
    printf("\nC/ everything is closed\n");
    // Exit the program
    exit(0);
}
