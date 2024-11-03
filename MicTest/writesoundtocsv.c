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
    bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
    //bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_65536); // The default
    uint32_t freq = 8000000;
    bcm2835_spi_set_speed_hz(freq);
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default

    // Send a some bytes to the slave and simultaneously read 
    // some bytes back from the slave
    // Most SPI devices expect one or 2 bytes of command, after which they will send back
    // some data. In such a case you will have the command bytes first in the buffer,
    // followed by as many 0 bytes as you expect returned data bytes. After the transfer, you 
    // Can the read the reply bytes from the buffer.
    // If you tie MISO to MOSI, you should read back what was sent.
    int samplemax= 5000; // 5000 for 10ms //1000000; //using too large values creates semgmentation faults
    int sample=0;
    //double start, end; 
    char tbuf[] = {0x01, 0x02}; // Data to send
    char rbuf[] = {0x00, 0x00};
    uint16_t values[samplemax];
    float voltages[samplemax];
    //struct timespec remaining,request = {20,20};
    //double timetest = 5;
    //clock_t startc, endc;
    //double total_t;
    //double timebetweenblock = 0.000000010;
    //int count;
    //printf("clock per sec = %f \n", (double)CLOCKS_PER_SEC);
    //start=time(NULL);
    //startc= clock();

    clock_t start = clock();
    while(sample < samplemax) //uncomment to record readings over a limited time
    {//printf("%d\n", sample);
        
        bcm2835_spi_transfernb(tbuf, rbuf, sizeof(tbuf));
        //bcm2835_st_delay(0,1); //doesn't seem to have much effect investigate later 
        //but so far the timing limit between cs is respected

        //uncomment the following 2 lines to record readings 
        values[sample] = 0x0FFF & ((((uint16_t)(rbuf[0])<<8) | ((uint16_t)rbuf[1]))>>2);
        ++sample;
        
    }
    clock_t end = clock();
    double total_time_taken = ((double)end - start) / CLOCKS_PER_SEC * 1000000;
    double single_time_taken = total_time_taken/samplemax; //time for a single transaction

    printf("Time taken: %f microseconds\n", single_time_taken);
    //end=time(NULL);
    //endc=clock();
    // buf will now be filled with the data that was read from the slave
    //printf("Read from SPI: %02X  %02X \n", rbuf[0], rbuf[1]);
       printf("# %d samples in %.1f microseconds (%.0f/s)\n",
      sample, total_time_taken, (float)sample/(total_time_taken));
      printf("decimal = %d\n", values[0]);
      printf("voltage = %.3f\n", (values[0]*3.3/4095));
      
        size_t sizeval = sizeof(values) / sizeof(values[0]);
        //size_t sizevol = sizeof(voltages) / sizeof(voltages[0]);
        size_t sizevol = sizeof(voltages[0]);
        printf("size of values: %d\n", sizeval);
        printf("size of voltages: %d\n", sizevol);

      /*for(int i=0; i<(samplemax-1); i++)
      {
        printf("%d\n", i);
          voltages[i]= (((float)values[i])*3.3/4095);
          //printf("%.3f\n", (float)voltages[i]);
      }*/

      
    //printf("time taken according to clock = %f \n", ((double)(endc-startc)/CLOCKS_PER_SEC));

    
    //write data to csv file
    FILE *file; //file pointer variable
    file = fopen("newPItest3.csv","w"); //fopen is going to return a pointer that is gonna be stored in file

    if (file==NULL) //check if file is opened successfuly 
    {
        printf("error opening file.\n");
        return 1;
    }
    char index[] = "index";
    char valuename[] = "value";

    fprintf(file, "%s, %s\n", index, valuename);
    for(int i=0; i<samplemax; i++)
    {
        //printf("%d\n", i);
        fprintf(file, "%d, %d\n", i, values[i]);
        //fprintf(file, "%d\n", values[i]);
        //if (ferror(file))
        //{
        //    printf("error writing to file.\n");
        //    return 1;
        //}
    }

  

    fclose(file);
        bcm2835_spi_end();
    bcm2835_close();
    printf("done!!!\n");
    
    return 0;
}