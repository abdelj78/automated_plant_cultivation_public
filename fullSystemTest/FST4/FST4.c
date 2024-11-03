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

#define samplemax 10000 //for 20ms)
//#define samplemax2 2048 //for 4.096ms (2^11)
#define samplemax2 1024 //for 2.048ms
//int samplemax= 5000; // 5000 for 10ms //1000000; //using too large values creates semgmentation faults
int sample=0;
//double start, end; 
char tbuf[] = {0x01, 0x02}; // Data to send
char rbuf[] = {0x00, 0x00};
uint16_t audio[2][samplemax];
double audio_short[samplemax2]; //circular buffer
bool amptrigger = false;
int r = 0; //row index
int tar=0; //target row index
double thresholdv = 0;  //  threshold voltage
double threshold = 0;  // threshold value ADC
bool trigger = false;
int file_number = 0;  // This is the integer you want to add to the filename
char filename[256];  // Buffer to hold the filename
int indextrig = 0;
double filtered_data[samplemax];
double audio_double[samplemax];
fftw_plan plan;
fftw_complex* fft_result;
pid_t pid;
float avg = 0.0;

//function prototypes
void *dataRead(void *arg);
void *datapreprocess(void *arg);
void *setupbcm2835(void *arg);
void handle_sigint(int sig);


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
    setupbcm2835(NULL);

    // Allocate FFTW complex output array
    fft_result = fftw_alloc_complex(samplemax);
    // Create a DFT plan
    

    //threshold calculation in ADC value 
    thresholdv = 0.1;  //V , 140mV should be the max value of plant sound 
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

void *dataRead(void *arg) {
    //printf("Reading data\n");
    //struct timespec start, end;
    //clock_t start = clock();
    //clock_gettime(CLOCK_MONOTONIC, &start);
    // normal double buffer method 
    for(int i=0; i<samplemax; i++) //uncomment to record readings over a limited time
    {  
        bcm2835_spi_transfernb(tbuf, rbuf, sizeof(tbuf));
        audio[r][i] = 0x0FFF & ((((uint16_t)(rbuf[0])<<8) | ((uint16_t)rbuf[1]))>>2);   
    }

    /*// circular buffer method not finished too many issues and compromises to make 
    while (!amptrigger) {
        bcm2835_spi_transfernb(tbuf, rbuf, sizeof(tbuf));
        audio_cb[sample] = 0x0FFF & ((((uint16_t)(rbuf[0])<<8) | ((uint16_t)rbuf[1]))>>2);
        
        //avg calc using last 10 values
        avg = 0.0;
        for(int i=0; i<100; i++){
            avg += (double)audio_cb[(sample-i+samplemax)%samplemax];
        }
        avg /= 100;


        if ((audio_cb[sample] > (threshold+avg) || audio_cb[sample] < (threshold-avg))) {
            amptrigger = true;
            printf("\nC/ Triggered at index %d with value %d", sample, audio_cb[sample]);
        }
        sample = (sample + 1) % samplemax;
    }*/

    //printf("Data read from row %d done\n", r);
    //clock_gettime(CLOCK_MONOTONIC, &end);

    // double total_time_taken = (end.tv_sec - start.tv_sec) * 1e9;
    // total_time_taken = (total_time_taken + (end.tv_nsec - start.tv_nsec)) * 1e-6;  // Convert to microseconds

    // printf("reading Time taken: %f milliseconds\n", total_time_taken);
    // clock_t end = clock();
    // double total_time_taken = ((double)end - start) / CLOCKS_PER_SEC * 1000000;
    // //double single_time_taken = total_time_taken/samplemax; //time for a single transaction

    // printf("read Time taken: %f microseconds\n", total_time_taken);
    
    return NULL;
}


void *datapreprocess(void *arg) {
    //clock_t start2 = clock();
    // struct timespec start2, end2;
    // //clock_t start = clock();
    // clock_gettime(CLOCK_MONOTONIC, &start2);
    //printf("Preprocessing data\n");
    if (r == 0) {
        tar = 1;
    }else {
        tar = 0;
    }

    for (int i = 0; i < samplemax; i++) {
        audio_double[i] = (double)audio[tar][i];
    }
    
    //DC OFFSET REMOVAL, Calculate the mean of the data
    double mean = 0.0;
    for (int i = 0; i < samplemax; i++) {
        mean += audio_double[i];
    }
    mean /= samplemax;

    for (int i = 0; i < samplemax; i++) {
        audio_double[i] -= mean;
        //filtered_data[i] = audio_double[i];
    }


    // Apply the FIR filter
    for (int i = 0; i < samplemax; i++) {
        filtered_data[i] = 0; // Initialize to 0

        for (int j = 0; j < BL; j++) {
            if (i - j < 0) continue; // Skip if out of bounds
            filtered_data[i] += audio_double[i - j] * B[j];
        }
    }

    //find peak value and its index
    double max_value = filtered_data[0];
    int max_index = 0;

    for (int i = 1; i < samplemax; i++) {
        if (filtered_data[i] > max_value) {
            max_value = filtered_data[i];
            max_index = i;
        }
    }

    if (max_value > threshold) {
        trigger = true;
        indextrig = max_index;
    }

    if (trigger) {

        int trigstartindex = 100;
        //first part of signal 
        for (int i = 0; i < 100; i++) {
            if (max_index-trigstartindex+i < 0) {
                audio_short[i] = 0;            
            } else {
                audio_short[i] = filtered_data[max_index-trigstartindex+i]; //first part of the signal
            }
        }
        //second part of signal
        for (int i = 100; i < samplemax2; i++) {
            if (max_index+i < samplemax) {
                audio_short[i] = 0;            
            } else {
                audio_short[i] = filtered_data[max_index+i]; //second part of the signal
            }
        }


        // //just check if the value is above the threshold to save time and reduce noise chances of triggering
        // for (int i = 0; i < samplemax; i++) {
        //     if (filtered_data[i] > threshold) {
        //         //save small section of signal
        //         trigger = true;
        //         indextrig = i;
        //         break;
        //     }
        // }

        //SAVE RECORDING IF TRIGGERED
        
        //plan = fftw_plan_dft_r2c_1d(samplemax, filtered_data, fft_result, FFTW_ESTIMATE);
        plan = fftw_plan_dft_r2c_1d(samplemax2, audio_short, fft_result, FFTW_ESTIMATE);
        // Execute the plan
        fftw_execute(plan);
        //find the index of the maximum value in the FFT result
        double maxval = 0;
        double maxval2=0;
        int freqmaxindex = 0;
        double val[samplemax2];
        for (int i = 0; i < samplemax2/2; i++) {
            //printf("\n%d",i);
            // Only need to check the first half of the array for positive frequencies
            // Calculate the magnitude of the complex number, no sqrt to save time
            val[i] = sqrt(fft_result[i][0] * fft_result[i][0] + fft_result[i][1] * fft_result[i][1]);
            if (val[i] > maxval) {
                maxval = val[i];
                freqmaxindex = i;
            }
        }
        //convert index to frequency
        double freqmax = (double)freqmaxindex * (500000/2) / (samplemax2/2);


        if (45000<freqmax && freqmax<55000){
            if(maxval > 7000){
                printf("\nC/ Triggered at index %d with frequency %.2f kHz and mag %.2f k unit", indextrig, freqmax*1e-3, maxval*1e-3);
                //printf("\nC/ TRIGGERED Sending signal to Python script");
                kill(pid, SIGUSR1);   
            }
        }
                // clock_gettime(CLOCK_MONOTONIC, &end2);

        // double total_time_taken2 = (end2.tv_sec - start2.tv_sec) * 1e9;
        // total_time_taken2 = (total_time_taken2 + (end2.tv_nsec - start2.tv_nsec)) * 1e-6;  // Convert to microseconds

        // printf("porcessing Time taken: %f milliseconds\n", total_time_taken2);

        trigger=false;        
    }
    //     clock_gettime(CLOCK_MONOTONIC, &end2);

    // double total_time_taken2 = (end2.tv_sec - start2.tv_sec) * 1e9;
    // total_time_taken2 = (total_time_taken2 + (end2.tv_nsec - start2.tv_nsec)) * 1e-6;  // Convert to microseconds

    // printf("porcessing Time taken: %f milliseconds\n", total_time_taken2);

    // clock_t end2 = clock();
    // double total_time_taken2 = ((double)end2 - start2) / CLOCKS_PER_SEC * 1000000;

    //printf("Data preprocessing from row %d done\n", tar);
        
    // printf("processing Time taken: %f microseconds\n", total_time_taken2);
    
    return NULL;
}