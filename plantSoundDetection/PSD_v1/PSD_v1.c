// This code has for aim to record small parts of sound if the amplitude exceeds a certain level

#include <bcm2835.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include </usr/include/time.h>
//#include </home/pi/Downloads/fftw-3.3.10/api/fftw3.h>
 
int main(int argc, char **argv)
{
    // If you call this, it will not actually access the GPIO
    // Use for testing
    // bcm2835_set_debug(1);
    
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
    uint16_t clkdiv = bcm2835_aux_spi_CalcClockDivider(freq);
    printf("clock divider is = %d\n", clkdiv);
    bcm2835_spi_set_speed_hz(freq);
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default
    
    int samplemax= 5000; //=10ms
    int sample=0;
    double start, end; 
    char tbuf[] = {0x00, 0x00}; // Data to send
    char rbuf[] = {0x00, 0x00};
    uint16_t values[samplemax];
    uint16_t final[samplemax];
    float voltages[samplemax];
    int recnb = 0; //recording index
    int recmax = 49; //50 recoding max before stopping 
    int thrmin = 2326; //avg at around 2356 
    int thrmax = 2386;
    int triggerid=0;
    bool trigger= false;
    int count=0;
    while(recnb < recmax)
    {

        while(!trigger) //wile not triggered
        {
            bcm2835_spi_transfernb(tbuf, rbuf, sizeof(tbuf));
            
            //uncomment the following 2 lines to record readings 
            values[sample] = 0x0FFF & ((((uint16_t)(rbuf[0])<<8) | ((uint16_t)rbuf[1]))>>2);
            if ((values[sample] > thrmax || values[sample] < thrmin) && !trigger && sample !=0)
            {
                trigger=true;
                triggerid = sample;
                printf("triggered because values = %d and sample= %d", values[sample], sample);
            }
            sample = (sample + 1) % samplemax;
            count++;
        }

        //record the data if triggered
        //for now just record from moment triggered, sort out later to have good recordings
        //if out of the while loop then it has been triggered
        for(int i=0; i<4000; i++)//record next 4000 samples
        {
            bcm2835_spi_transfernb(tbuf, rbuf, sizeof(tbuf));
        
            //uncomment the following 2 lines to record readings 
            values[sample] = 0x0FFF & ((((uint16_t)(rbuf[0])<<8) | ((uint16_t)rbuf[1]))>>2);
            sample = (sample + 1) % samplemax;
        }

        //store all values in order including previous 1000 samples
        for(int i=0; i<samplemax; i++)
        {
            final[i]= values[sample];
            sample = (sample + 1) % samplemax;
            //printf("sample nb %d \n", sample);
        }

        //analyse main frequencies

        //if main frequencies are above 20kHz save the array "final" in csv
        
    trigger = false;
    recnb=50;
    printf("count = %d", count);

    }
    /*

    // buf will now be filled with the data that was read from the slave
    //printf("Read from SPI: %02X  %02X \n", rbuf[0], rbuf[1]);
    printf("# %d samples in %.1f seconds (%.0f/s)\n",
    sample, end-start, (float)sample/(end-start));
    printf("decimal = %d\n", values[0]);
    printf("voltage = %.3f\n", (values[0]*3.3/4095));
      
        size_t sizeval = sizeof(values) / sizeof(values[0]);
        //size_t sizevol = sizeof(voltages) / sizeof(voltages[0]);
        size_t sizevol = sizeof(voltages[0]);
        printf("size of values: %d\n", sizeval);
        printf("size of voltages: %d\n", sizevol);*/

      /*for(int i=0; i<(samplemax-1); i++)
      {
        printf("%d\n", i);
          voltages[i]= (((float)values[i])*3.3/4095);
          //printf("%.3f\n", (float)voltages[i]);
      }*/

      
    //printf("time taken according to clock = %f \n", ((double)(endc-startc)/CLOCKS_PER_SEC));

    
    //write data to csv file
    /*FILE *file; //file pointer variable
    file = fopen("file.csv","w"); //fopen is going to return a pointer that is gonna be stored in file

    if (file==NULL) //check if file is opened successfuly 
    {
        printf("error opening file.\n");
        return 1;
    }

    for(int i=0; i<samplemax; i++)
    {
        //printf("%d\n", i);
        fprintf(file, "%d, %d\n", i, values[i]);
        */
        /*if (ferror(file))
        {
            printf("error writing to file.\n");
            return 1;
        }*/
    /*}

    fclose(file);
        bcm2835_spi_end();
    bcm2835_close();*/
    printf("\ndone!!!\n");
    
    return 0;
}