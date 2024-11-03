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

#define samplemax 2048 //for 4.096ms (2^11)

//int samplemax= 5000; // 5000 for 10ms //1000000; //using too large values creates semgmentation faults
int sample=0;
//double start, end; 
char tbuf[] = {0x01, 0x02}; // Data to send
char rbuf[] = {0x00, 0x00};
uint16_t audio[2][samplemax];
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

//function prototypes
void *dataRead(void *arg);
void *datapreprocess(void *arg);
void *setupbcm2835(void *arg);
void handle_sigint(int sig);


int main(int argc, char *argv[])
{
    // Check if a PID was provided
    if (argc < 2) {
        printf("Please provide the PID of the Python script\n");
        return 1;
    }
    // Convert the PID to an integer
    pid = atoi(argv[1]);
    printf("PID in main: %d\n", pid);

    // Set up the signal handler
    signal(SIGINT, handle_sigint);
    setupbcm2835(NULL);

    // Allocate FFTW complex output array
    fft_result = fftw_alloc_complex(samplemax);
    // Create a DFT plan
    

    //threshold calculation in ADC value 
    thresholdv = 0.8;  //V , 140mV should be the max value of plant sound 
    threshold = thresholdv*4095/3.3;  // equivalent ADC value 
    printf("Threshold: %f\n", threshold);
    
    pthread_t thread1, thread2;
    int result1;
    int result2;
    while(1){
        result1=pthread_create(&thread1, NULL, dataRead, NULL);
        if (result1 != 0) {
            printf("Error creating thread 1: %s\n", strerror(result1));
            exit(0);
        }   
        result2=pthread_create(&thread2, NULL, datapreprocess, NULL);
        if (result2 != 0) {
            printf("Error creating thread 2: %s\n", strerror(result2));
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
      printf("bcm2835_init failed. Are you running as root??\n");
      return NULL;
    }

    if (!bcm2835_spi_begin())
    {
      printf("bcm2835_spi_begin failed. Are you running as root??\n");
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
    printf("\nCaught signal %d, will exit safely\n", sig);
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
    printf("everything is closed\n");
    // Exit the program
    exit(0);
}

void *dataRead(void *arg) {
    //printf("Reading data\n");
    //struct timespec start, end;
    //clock_t start = clock();
    //clock_gettime(CLOCK_MONOTONIC, &start);
    for(int i=0; i<samplemax; i++) //uncomment to record readings over a limited time
    {   
        bcm2835_spi_transfernb(tbuf, rbuf, sizeof(tbuf));
        audio[r][i] = 0x0FFF & ((((uint16_t)(rbuf[0])<<8) | ((uint16_t)rbuf[1]))>>2);   
    }
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


    //just check if the value is above the threshold to save time and reduce noise chances of triggering
    for (int i = 0; i < samplemax; i++) {
        if (filtered_data[i] > threshold) {
            trigger = true;
            indextrig = i;
            break;
        }
    }

    //SAVE RECORDING IF TRIGGERED
    if (trigger) {
        plan = fftw_plan_dft_r2c_1d(samplemax, filtered_data, fft_result, FFTW_ESTIMATE);
        // Execute the plan
        fftw_execute(plan);
        //find the index of the maximum value in the FFT result
        double maxval = 0;
        int freqmaxindex = 0;
        double val[samplemax];
        for (int i = 0; i < samplemax/2; i++) {
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
        double freqmax = (double)freqmaxindex * (500000/2) / (samplemax/2);


        if (45000<freqmax && freqmax<55000){
            
            plan = fftw_plan_dft_r2c_1d(samplemax, audio_double, fft_result, FFTW_ESTIMATE);
            // Execute the plan
            fftw_execute(plan);
            //find the index of the maximum value in the FFT result
            double valnotfilt[samplemax];
            for (int i = 0; i < samplemax/2; i++) {
                //printf("\n%d",i);
                // Only need to check the first half of the array for positive frequencies
                // Calculate the magnitude of the complex number, no sqrt to save time
                valnotfilt[i] = sqrt(fft_result[i][0] * fft_result[i][0] + fft_result[i][1] * fft_result[i][1]);
            }
            printf("Triggered at index %d with frequency %.2f kHz and mag %.2f k unit\n", indextrig, freqmax*1e-3, maxval*1e-3);
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
            char audioname[] = "raw_audio";
            char valuename[] = "filtered_data";
            char freqname[] = "frequency";
            char freqmagname[] = "frequency_magnitude";
            char freqmagnamenotfilt[] = "frequency_magnitude";
            char threshname[] = "threshold";

            fprintf(file, "%s, %s, %s, %s, %s, %s\n",audioname, valuename, freqname, freqmagname,freqmagnamenotfilt, threshname);
            
            if (ferror(file))
            {
                printf("error writing to file.\n");
                return NULL;
            }

            for(int i=0; i<samplemax; i++)
            {
                fprintf(file, "%f, %f, %f, %f, %f, %f\n", audio_double[i], filtered_data[i],(double)i*(500000/2)/(samplemax/2), val[i], valnotfilt[i], threshold);
                if (ferror(file))
                {
                printf("error writing to file.\n");
                return NULL;
                }
            }

            fclose(file);
            printf("C/ Sending signal to Python script\n");
            kill(pid, SIGUSR1);   

        }
                // clock_gettime(CLOCK_MONOTONIC, &end2);

        // double total_time_taken2 = (end2.tv_sec - start2.tv_sec) * 1e9;
        // total_time_taken2 = (total_time_taken2 + (end2.tv_nsec - start2.tv_nsec)) * 1e-6;  // Convert to microseconds

        // printf("porcessing Time taken: %f milliseconds\n", total_time_taken2);

        trigger=false;
        if (file_number>10){
            printf("%d files have been saved. Exiting program.\n", file_number-1);
            /*printf("FFT Result:\n");
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
            printf("everything is closed\n");   
            exit(0);
        }
        
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