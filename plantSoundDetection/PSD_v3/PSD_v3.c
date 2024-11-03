// spin.c
//
// Example program for bcm2835 library
// Shows how to interface with SPI to transfer a number of bytes to and from an SPI device
//
// After installing bcm2835, you can build this 
// with something like:
// gcc -o spin spin.c -l bcm2835
// sudo ./spin
//
// Or you can test it before installing with:
// gcc -o spin -I ../../src ../../src/bcm2835.c spin.c
// sudo ./spin
//
// Author: Mike McCauley
// Copyright (C) 2012 Mike McCauley
// $Id: RF22.h,v 1.21 2012/05/30 01:51:25 mikem Exp $
 
#include <bcm2835.h>
#include <stdio.h>
#include <unistd.h>
//#include </usr/include/time.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "fdacoefs.h"

#define samplemax 5000

//int samplemax= 5000; // 5000 for 10ms //1000000; //using too large values creates semgmentation faults
int sample=0;
//double start, end; 
char tbuf[] = {0x01, 0x02}; // Data to send
char rbuf[] = {0x00, 0x00};
uint16_t values[2][samplemax];
int r = 0; //row index
int tar=0; //target row index
double thresholdv = 0;  //  threshold voltage
double threshold = 0;  // threshold value ADC
double upperthreshold = 0.0;
double lowerthreshold = 0.0;
bool trigger = false;
int file_number = 1;  // This is the integer you want to add to the filename
char filename[256];  // Buffer to hold the filename
int indextrig = 0;
double filtered_data[samplemax];
double values_double[samplemax];

//function prototypes
void *dataRead(void *arg);
void *datapreprocess(void *arg);


int main(int argc, char **argv)
{
    // If you call this, it will not actually access the GPIO
// Use for testing
//        bcm2835_set_debug(1);
    if (!bcm2835_init())
    {
      printf("bcm2835_init failed. Are you running as root??\n");
      return 1;
    }

    if (!bcm2835_spi_begin())
    {
      printf("bcm2835_spi_begin failed. Are you running as root??\n");
      return 1;
    }



    //bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
    //bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_65536); // The default
    uint32_t freq = 8000000;
    bcm2835_spi_set_speed_hz(freq);
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default

    
    thresholdv = 0.2;  //V , 140mV should be the max value of plant sound 
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
        //printf("hi");
        //sleep(1);
        if (r == 0) {
            r = 1;
        } else {
            r = 0;
        }
    }



    bcm2835_spi_end();
    bcm2835_close();
    printf("done!!!\n");
    
    return 0;
}



void *dataRead(void *arg) {
    //printf("Reading data\n");
    //clock_t start = clock();
    sample = 0;
    while(sample < samplemax) //uncomment to record readings over a limited time
    {   
        bcm2835_spi_transfernb(tbuf, rbuf, sizeof(tbuf));
        values[r][sample] = 0x0FFF & ((((uint16_t)(rbuf[0])<<8) | ((uint16_t)rbuf[1]))>>2);
        ++sample;    
    }
    //printf("Data read from row %d done\n", r);

    // clock_t end = clock();
    // double total_time_taken = ((double)end - start) / CLOCKS_PER_SEC * 1000000;
    // //double single_time_taken = total_time_taken/samplemax; //time for a single transaction

    // printf("read Time taken: %f microseconds\n", total_time_taken);
    
    return NULL;
}


void *datapreprocess(void *arg) {
    //clock_t start2 = clock();

    //printf("Preprocessing data\n");
    if (r == 0) {
        tar = 1;
    }else {
        tar = 0;
    }

    for (int i = 0; i < samplemax; i++) {

            values_double[i] = (double)values[tar][i];
        
    }
    
    //DC OFFSET REMOVAL, Calculate the mean of the data
    // double mean = 0.0;
    // for (int i = 0; i < samplemax; i++) {
    //     mean += values[tar][i];
    // }
    // mean /= samplemax;

    double mean = 0.0;
    for (int i = 0; i < samplemax; i++) {
        mean += values_double[i];
    }
    mean /= samplemax;

    //can remove this step to save time and check threshold from mean calculated
    // Subtract the mean from each data point to remove the DC offset
    // for (int i = 0; i < samplemax; i++) {
    //     values[tar][i] -= mean;
    // }

    for (int i = 0; i < samplemax; i++) {
        values_double[i] -= mean;
    }

    // Apply the FIR filter
    for (int i = 0; i < samplemax; i++) {
        filtered_data[i] = 0; // Initialize to 0

        for (int j = 0; j < BL; j++) {
            if (i - j < 0) continue; // Skip if out of bounds
            filtered_data[i] += values_double[i - j] * B[j];
        }
    }

    //this section is to avoid filtering the data so we can test output of stm32
    // for (int i = 0; i < samplemax; i++) {

    //         filtered_data[i] = values_double[i];
        
    // }
    //upperthreshold = mean + threshold;
    //lowerthreshold = mean - threshold;
    //printf("Upper threshold: %f\n", upperthreshold);
    
    //THRESHOLDING
    /*for (int i = 0; i < samplemax; i++) {
        if (values[tar][i] > upperthreshold || values[tar][i] < lowerthreshold) {
            trigger = true;
            break;
        }
    }*/
    
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
        printf("Triggered at index %d\n", indextrig);
        if (file_number>50){
            printf("50 files have been saved. Exiting program.\n");
            exit(0);
        }
        //write data to csv file
        sprintf(filename, "Triggered/trig%d.csv", file_number);  // Format the filename
        FILE *file; //file pointer variable
        file = fopen(filename,"w"); //fopen is going to return a pointer that is gonna be stored in file

        if (file==NULL) //check if file is opened successfuly 
        {
            printf("error opening file.\n");
            return NULL;
        }
        //char index[] = "index";
        char valuename[] = "value";
        char threshname[] = "threshold";

        fprintf(file, "%s, %s\n", valuename, threshname);
        
        if (ferror(file))
        {
            printf("error writing to file.\n");
            return NULL;
        }

        for(int i=0; i<samplemax; i++)
        {
            fprintf(file, "%f, %f\n", filtered_data[i], threshold);
            if (ferror(file))
            {
               printf("error writing to file.\n");
               return NULL;
            }
        }

        fclose(file);
        trigger=false;
        file_number++;
    }

    // clock_t end2 = clock();
    // double total_time_taken2 = ((double)end2 - start2) / CLOCKS_PER_SEC * 1000000;

    //printf("Data preprocessing from row %d done\n", tar);
        
    // printf("processing Time taken: %f microseconds\n", total_time_taken2);
    
    return NULL;
}