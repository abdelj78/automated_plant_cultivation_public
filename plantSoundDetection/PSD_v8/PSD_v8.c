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
#define BUFFER_SIZE 4096  // Set this to the desired buffer size
//double thresh_fmax_mag = 100000000;  //magnitude threshold for the frequency of the plant sound, trial and error
double sampling_freq = 500000; //500kHz
double thresholdv = 0.08;  //V , 140mV should be the max value of plant sound 
double threshold = 0;  // threshold value ADC is calculated in the main function
double lower_freq = 36000; //based on Quartile 1
double upper_freq = 60000; //based on Quartile 3

// other variables
char tbuf[] = {0x01, 0x02}; // Data to send
char rbuf[] = {0x00, 0x00};

bool trigger = false;

fftw_plan plan;
fftw_complex* fft_result;

uint16_t audio[2][BUFFER_SIZE];
double audio_d[BUFFER_SIZE];

int r = 0; //row index
int tar=0; //target row index

int buffer_index = 0;
bool triggered = false;
int trignb = 0;

char filename[256];  // Buffer to hold the filename

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
void csvsave(double* buffer, int file_number, double threshold);

int main(int argc, char *argv[])
{

    // Set up the signal handler
    signal(SIGINT, handle_sigint);
    setupbcm2835(NULL);
    // Allocate FFTW complex output array
    fft_result = fftw_alloc_complex(BUFFER_SIZE);
    // if (fft_result == NULL) {
    //     printf("Failed to allocate memory for FFTW output.\n");
    //     exit(1);
    // }
    //threshold calculation in ADC value 
    threshold = thresholdv*4095/3.3;  // equivalent ADC value 
    printf("\nC/ Threshold(V): %f", thresholdv);
    printf("\nC/ Threshold(ADC): %f\n\n", threshold);
    
    pthread_t thread1, thread2;
    int result1;
    int result2;
    while(1){
        result1=pthread_create(&thread1, NULL, dataRead, NULL);
        if (result1 != 0) {
            printf("C/ Error creating thread 1: %s\n", strerror(result1));
            exit(0);
        }   
        result2=pthread_create(&thread2, NULL, datapreprocess, NULL);
        if (result2 != 0) {
            printf("C/ Error creating thread 2: %s\n", strerror(result2));
            exit(0);
        }
        pthread_join(thread1, NULL);
        pthread_join(thread2, NULL);

        if (r == 0) {
            r = 1;
        } else {
            r = 0;
        }
    }
    return 0;
}

void *dataRead(void *arg) {
    // normal double buffer method 
    for(int i=0; i<BUFFER_SIZE; i++) //uncomment to record readings over a limited time
    {  
        bcm2835_spi_transfernb(tbuf, rbuf, sizeof(tbuf));
        audio[r][i] = 0x0FFF & ((((uint16_t)(rbuf[0])<<8) | ((uint16_t)rbuf[1]))>>2);   
    }
    
    return NULL;
}

void *datapreprocess(void *arg) {
    if (r == 0) {
        tar = 1;
    }else {
        tar = 0;
    }

    for (int i = 0; i < BUFFER_SIZE; i++) {
        audio_d[i] = (double)audio[tar][i];
    }
    
    //DC OFFSET REMOVAL, Calculate the mean of the data
    double mean = 0.0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        mean += audio_d[i];
    }
    mean /= BUFFER_SIZE;

    for (int i = 0; i < BUFFER_SIZE; i++) {
        audio_d[i] -= mean;
    }

    //check if the signal is above the threshold
    for(int i=0; i<BUFFER_SIZE; i++){
        if (audio_d[i] > threshold){
            trigger = true;
            break;
        }
    }
    
    if (trigger) {
        //amplitude triggered now check for frequency
        
        plan = fftw_plan_dft_r2c_1d(BUFFER_SIZE, audio_d, fft_result, FFTW_ESTIMATE);
        fftw_execute(plan);

        //find the index of the maximum value in the FFT result
        double maxval = 0; //max magnitude of fftout
        int freqmaxindex = 0; //index of max magnitude of fftout
        double fftout[BUFFER_SIZE]; //output of fft
        double freqmax = 0; //freq with max magnitude
        double fmax_mag = 0; //max magnitude of freqmax

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
        freqmax = (double)freqmaxindex * sampling_freq / BUFFER_SIZE;
        fmax_mag = maxval;

        int freq20 = 


        if (lower_freq<freqmax && freqmax<upper_freq){
            if(maxval > 1/*7000*/){
                printf("\nC/ Triggered at index %d with frequency %.2f kHz and mag %.2f k unit", indextrig, freqmax*1e-3, maxval*1e-3);
                //printf("\nC/ TRIGGERED Sending signal to Python script");
                            //write data to csv file
            file_number++;
            sprintf(filename, "Triggered/trig%d.csv", file_number);  // Format the filename
            FILE *file; //file pointer variable
            file = fopen(filename,"w"); //fopen is going to return a pointer that is gonna be stored in file

            if (file==NULL) //check if file is opened successfuly 
            {
                printf("error opening file.\n");
                return NULL;
            }
            //char audioname[] = "raw_audio";
            char valuename[] = "filtered_data";
            char freqname[] = "frequency";
            char freqmagname[] = "frequency_magnitude";
            //char freqmagnamenotfilt[] = "frequency_magnitude";
            char threshname[] = "threshold";

            fprintf(file, "%s, %s, %s, %s\n", valuename, freqname, freqmagname, threshname);
            
            if (ferror(file))
            {
                printf("error writing to file.\n");
                return NULL;
            }

            for(int i=0; i<BUFFER_SIZE2; i++)
            {
                fprintf(file, "%f, %f, %f, %f\n", audio_short[i], (double)i*(500000/2)/(BUFFER_SIZE2/2), val[i], threshold);
                if (ferror(file))
                {
                printf("error writing to file.\n");
                return NULL;
                }
            }

            fclose(file);
            }

            trigger=false;
            if (file_number>10){
                printf("%d files have been saved. Exiting program.\n", file_number-1);
                /*printf("FFT Result:\n");
                for (int i = 0; i < BUFFER_SIZE/2; i++) {
                    printf("Index %d: Real = %f, Imaginary = %f\n", i, fft_result[i][0], fft_result[i][1]);
                }*/
                // Free memory
                fftw_free(fft_result);
                fftw_cleanup();
                // End SPI and close BCM2835
                sleep(1); //seems to correct the issue with segmentation fault when closing bcm2835
                bcm2835_spi_end();
                bcm2835_close();
                printf("everything is closed\n");   
                exit(0);
                }   
        }
        
        trigger=false;        
    }
    
    return NULL;
}

void csvsave(double* buffer_given, int file_number, double threshold){

    printf("\ntrig nb: %d\n", file_number);
    sprintf(filename, "Triggered_plant/trig%d.csv", file_number);  // Format the filename
    FILE *file; //file pointer variable
    file = fopen(filename,"w"); //fopen is going to return a pointer that is gonna be stored in file

    if (file==NULL) //check if file is opened successfuly 
    {
        printf("error opening file.\n");
        return;
    }
    //char audioname[] = "raw_audio";
    char valuename[] = "raw_audio";
    char thresholdname[] = "threshold";

    fprintf(file, "%s, %s\n", valuename, thresholdname);
    
    if (ferror(file))
    {
        printf("error writing to file.\n");
        return;
    }

    for(int i=0; i<BUFFER_SIZE; i++)
    {
        fprintf(file, "%f, %f\n", buffer_given[i], threshold);
        if (ferror(file))
        {
        printf("error writing to file.\n");
        return;
        }
    }

    fclose(file);


    return; 
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
            for (int i = 0; i < BUFFER_SIZE/2; i++) {
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
