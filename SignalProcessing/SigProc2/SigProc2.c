#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "fdacoefs.h"
#include <bcm2835.h>
#include <math.h>

#define MAX_ROWS 5000

int main() {
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
    bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
    //bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_65536); // The default
    uint32_t freq = 8000000;
    bcm2835_spi_set_speed_hz(freq);
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default



    int samplemax= 5000;
    int sample=0;
    double start, end; 
    char tbuf[] = {0x01, 0x02}; // Data to send
    char rbuf[] = {0x00, 0x00};
    uint16_t values[samplemax];
    double voltages[samplemax];
    double filtered_data[MAX_ROWS];
    int filtered_data_int[MAX_ROWS];

    // record the data
    for(int i = 0; i<samplemax; i++) 
    {
        bcm2835_spi_transfernb(tbuf, rbuf, sizeof(tbuf));
        values[i] = 0x0FFF & ((((uint16_t)(rbuf[0])<<8) | ((uint16_t)rbuf[1]))>>2);
    }

    // convert the data to voltages
    for(int i = 0; i<samplemax; i++) 
    {
        voltages[i] = (values[i] * 3.3) / 4095;
    }

    printf("data read done\n");
    // Print the data to verify it was loaded correctly
    /*for (int i = 0; i < row_count; i++) {
        printf("%d\n", data[i]);
    }*/

    //////////SIGNAL PROCESSING///////
    // Start the timer
    clock_t start1 = clock();
    //DC OFFSET REMOVAL, Calculate the mean of the data
    double mean = 0.0;
    for (int i = 0; i < MAX_ROWS; i++) {
        mean += voltages[i];
    }
    mean /= MAX_ROWS;

    // Subtract the mean from each data point to remove the DC offset
    for (int i = 0; i < MAX_ROWS; i++) {
        voltages[i] -= mean;
    }

    printf("DC removal done\n");

    //FIR FILTER
    // Convert the data to double
    /*for (int i = 0; i < MAX_ROWS; i++) {
        data_double[i] = (double)voltages[i];
    }*/

    // Apply the FIR filter
    for (int i = 0; i < MAX_ROWS; i++) {
    filtered_data[i] = 0; // Initialize to 0

        for (int j = 0; j < BL; j++) {
            if (i - j < 0) continue; // Skip if out of bounds
            filtered_data[i] += voltages[i - j] * B[j];
        }
    }

    // Convert the filtered data back to int if necessary
    for (int i = 0; i < MAX_ROWS; i++) {
        filtered_data_int[i] = (int)filtered_data[i];
    }
    //NORMALIZATION
    
    // Stop the timer
    clock_t end1 = clock();

    // Calculate the time taken in seconds
    double time_taken = ((double)end - start) / CLOCKS_PER_SEC * 1000000;

    printf("Time taken: %f microseconds\n", time_taken);
    //write data to csv file
    FILE *fileout; //file pointer variable
    fileout = fopen("CleanData2.csv","w"); //fopen is going to return a pointer that is gonna be stored in file

    if (fileout==NULL) //check if file is opened successfuly 
    {
        printf("error opening file.\n");
        return 1;
    }
    char index[] = "index";
    char valuename[] = "voltages";
    char filteredname[] = "filtered";


    fprintf(fileout, "%s, %s\n", valuename, filteredname);
    for(int i=0; i<MAX_ROWS; i++)
    {
        //printf("%d\n", i);
        //fprintf(fileout, "%f, %d\n", voltages[i], filtered_data_int[i]);
        fprintf(fileout, "%f, %f\n", voltages[i], filtered_data[i]);
        //fprintf(file, "%d\n", values[i]);
        if (ferror(fileout))
        {
            printf("error writing to file.\n");
            return 1;
        }
    }
    printf("writing done\n");

    fclose(fileout);

    system("python3 sigFFT2.py");
    
    return 0;
}