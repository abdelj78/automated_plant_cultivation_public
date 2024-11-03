#include <stdio.h>
#include <math.h>
#include <bcm2835.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>


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
    uint16_t clkdiv = bcm2835_aux_spi_CalcClockDivider(freq);
    bcm2835_spi_set_speed_hz(freq);
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default
    

    char tbuf[] = {0x00, 0x00}; // Data to send
    char rbuf[] = {0x00, 0x00};
    


    FILE *gnuplotPipe = popen ("gnuplot -persistent", "w");

    if (gnuplotPipe == NULL) {
        printf("Error opening pipe to GNU plot. Install with 'sudo apt-get install gnuplot'\n");
        return 1;
    }

    fprintf(gnuplotPipe, "set title 'Sine Wave'\n");
    fprintf(gnuplotPipe, "plot '-' with lines\n");
    double x = 0;  // x-axis value, sample number
    uint16_t values; // y-axis value, ADC reading
    while (1) {
        for(int i = 0; i < 5000; i++) {  // send 1000 data points at a time
            x++;
            bcm2835_spi_transfernb(tbuf, rbuf, sizeof(tbuf)); //should delay by 2.01 us
            values = 0x0FFF & ((((uint16_t)(rbuf[0])<<8) | ((uint16_t)rbuf[1]))>>2);

            fprintf(gnuplotPipe, "%f %f\n", x, (double)values);
            fflush(gnuplotPipe);
            usleep(1000); // sleep for 1 ms (1000 us)
        }

        // Tell gnuplot that's the end of the data
        fprintf(gnuplotPipe, "e\n");
        fflush(gnuplotPipe);

        // Tell gnuplot to plot a new set of data
        fprintf(gnuplotPipe, "plot '-' with lines\n");
        
    }
    return 0;
}