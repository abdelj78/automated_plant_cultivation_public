#include <bcm2835.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "fdacoefs.h"

#define MAX_ROWS 5000

int main(int argc, char **argv)
{
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

    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
    //bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_65536); // The default
    uint32_t freq = 8000000;
    bcm2835_spi_set_speed_hz(freq);
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default

    char tbuf[] = {0x01, 0x02}; // Data to send
    char rbuf[] = {0x00, 0x00};

    int row_count = 0;

    uint16_t data[MAX_ROWS];
    double data_double[MAX_ROWS];
    double filtered_data[MAX_ROWS];
    int filtered_data_int[MAX_ROWS];


    clock_t start = clock();
    for(int i=0; i<MAX_ROWS; i++) //uncomment to record readings over a limited time
    {  
        bcm2835_spi_transfernb(tbuf, rbuf, sizeof(tbuf));
        data[i] = 0x0FFF & ((((uint16_t)(rbuf[0])<<8) | ((uint16_t)rbuf[1]))>>2);
    }
    clock_t end = clock();

    double total_time_taken = ((double)end - start) / CLOCKS_PER_SEC * 1000000;
    double single_time_taken = total_time_taken/MAX_ROWS; //time for a single transaction

    printf("Time taken to read 5000 samples: %f microseconds\n", total_time_taken);

    //////////SIGNAL PROCESSING///////
    // Start the timer
    clock_t start2 = clock();

    // Calculate the mean of the data
    double mean = 0.0;
    for (int i = 0; i < row_count; i++) {
        mean += data[i];
    }
    mean /= row_count;

    for (int i = 0; i < row_count; i++) {
    data_double[i] = (double)data[i];
    }

    // Subtract the mean from each element in the data
    for (int i = 0; i < row_count; i++) {
        data_double[i] -= mean;
    }

    printf("DC removal done\n");

    //FIR FILTER
    // Convert the data to double
   /* for (int i = 0; i < row_count; i++) {
        data_double[i] = (double)data[i];
    }*/

    // Apply the FIR filter
    for (int i = 0; i < row_count; i++) {
    filtered_data[i] = 0; // Initialize to 0

        for (int j = 0; j < BL; j++) {
            if (i - j < 0) continue; // Skip if out of bounds
            filtered_data[i] += data_double[i - j] * B[j];
        }
    }

    // Convert the filtered data back to int if necessary
    for (int i = 0; i < row_count; i++) {
        filtered_data_int[i] = (int)filtered_data[i];
    }

    int data_int[MAX_ROWS];
    // Convert the data back to int if necessary
    for (int i = 0; i < row_count; i++) {
        data_int[i] = (int)data_double[i];
    }
    //NORMALIZATION
    
    // Stop the timer
    clock_t end2 = clock();

    // Calculate the time taken in seconds
    double time_taken = ((double)end2 - start2) / CLOCKS_PER_SEC * 1000000;

    printf("Time taken to process sig: %f microseconds\n", time_taken);
    //write data to csv file
    FILE *fileout; //file pointer variable
    fileout = fopen("FSTdata.csv","w"); //fopen is going to return a pointer that is gonna be stored in file

    if (fileout==NULL) //check if file is opened successfuly 
    {
        printf("error opening file.\n");
        return 1;
    }

    char valuename[] = "value";
    char filteredname[] = "filtered";


    fprintf(fileout, "%s, %s\n", valuename, filteredname);

    for(int i=0; i<MAX_ROWS; i++)
    {
        //printf("%d\n", i);
        fprintf(fileout, "%d, %d\n", data_int[i], filtered_data_int[i]);
        //fprintf(file, "%d\n", values[i]);
        if (ferror(fileout))
        {
            printf("error writing to file.\n");
            return 1;
        }
    }
    printf("writing done\n");

    fclose(fileout);

    system("python3 FST.py");
    
    return 0;
}