//THIS CODE IS USED TO TEST THE ADC BUT ALSO TO MEASURE DIFFERENT MERTICS OF THE SIGNAL

#include <stdio.h>
#include <bcm2835.h>
#include <math.h>

//#define PIN RPI_GPIO_P1_15
#define SAMPLES 5000

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

    // calculate min, max, average, and difference
    uint16_t min_value = values[0], max_value = values[0];
    double min_voltage = voltages[0], max_voltage = voltages[0];
    double sum_values = 0, sum_voltages = 0;

    for(int i = 0; i<samplemax; i++) 
    {
        if (values[i] < min_value) min_value = values[i];
        if (values[i] > max_value) max_value = values[i];
        sum_values += values[i];

        if (voltages[i] < min_voltage) min_voltage = voltages[i];
        if (voltages[i] > max_voltage) max_voltage = voltages[i];
        sum_voltages += voltages[i];
    }

    double avg_value = sum_values / samplemax;
    double avg_voltage = sum_voltages / samplemax;

    uint16_t diff_value = max_value - min_value;
    double diff_voltage = max_voltage - min_voltage;

    // calculate standard deviation of values
    double sum_values2 = 0, sum_squared_values = 0;

    for(int i = 0; i<samplemax; i++) 
    {
        sum_values2 += values[i];
        sum_squared_values += values[i] * values[i];
    }

    double avg_value2 = sum_values2 / samplemax;
    double variance = sum_squared_values / samplemax - avg_value2 * avg_value2;
    double std_dev = sqrt(variance);

    // calculate standard deviation of voltages
    double sum_voltages2 = 0, sum_squared_voltages = 0;

    for(int i = 0; i<samplemax; i++) 
    {
        sum_voltages2 += voltages[i];
        sum_squared_voltages += voltages[i] * voltages[i];
    }

    double avg_voltage2 = sum_voltages2 / samplemax;
    double variance_voltage = sum_squared_voltages / samplemax - avg_voltage2 * avg_voltage2;
    double std_dev_voltage = sqrt(variance_voltage);

    //RMS
    double sum_squared_voltages_rms = 0;

    for(int i = 0; i<samplemax; i++) 
    {
        sum_squared_voltages_rms += (voltages[i]-avg_voltage2) * (voltages[i]-avg_voltage2);
    }

    double rms_voltage = sqrt(sum_squared_voltages_rms / samplemax);

    

    printf("Values:\nAvg = %f, Std Dev = %f\n", avg_value, std_dev);
    printf("Min = %d, Max = %d, Difference = %d\n", min_value, max_value, diff_value);
    
    printf("Voltages:\nAvg = %f, std dev = %f\n",  avg_voltage, std_dev_voltage);
    printf("Min = %f, Max = %f, Difference = %f\n", min_voltage, max_voltage, diff_voltage);
    printf("RMS = %f\n", rms_voltage);

    FILE *file = fopen("ADC_test4_data.csv", "w");
    if (file == NULL) {
        printf("Error opening file!\n");
        return 1;
    }

    // Add header
    fprintf(file, "Values, Voltage\n");

    for(int i = 0; i<samplemax; i++) 
    {
        fprintf(file, "%f, %f\n", (double)values[i], voltages[i]);
    }

    fclose(file);

    bcm2835_spi_end();
    bcm2835_close();

    system("python3 ADC_test4_plot.py");

    return 0;
}