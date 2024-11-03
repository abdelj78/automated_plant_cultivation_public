//THIS CODE IS USED TO TEST THE ADC BUT ALSO TO MEASURE DIFFERENT MERTICS OF THE SIGNAL

#include <stdio.h>
#include <bcm2835.h>
#include <math.h>
#include "fdacoefs.h"

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
    double valuesd[samplemax];
    double voltages[samplemax];

    // record the data
    for(int i = 0; i<samplemax; i++) 
    {
        bcm2835_spi_transfernb(tbuf, rbuf, sizeof(tbuf));
        values[i] = 0x0FFF & ((((uint16_t)(rbuf[0])<<8) | ((uint16_t)rbuf[1]))>>2);
    }

    // convert values from int to double
    for(int i = 0; i<samplemax; i++) 
    {
        valuesd[i] = (double)values[i];
    }

    // convert the data to voltages
    for(int i = 0; i<samplemax; i++) 
    {
        voltages[i] = (valuesd[i] * 3.3) / 4095;
    }

    //calculate pre-filtering 
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

    // calculate standard deviation of values and voltages
    double sum_squared_values = 0, sum_squared_voltages = 0;

    for(int i = 0; i<samplemax; i++) 
    {
        sum_squared_values += values[i] * values[i];
        sum_squared_voltages += voltages[i] * voltages[i];
    }

    double variance = sum_squared_values / samplemax - avg_value * avg_value;
    double std_dev = sqrt(variance);

    double variance_voltage = sum_squared_voltages / samplemax - avg_voltage * avg_voltage;
    double std_dev_voltage = sqrt(variance_voltage);
    //print the results pre-filtering
    printf("\n\nPRE-FILTERING\n");
    printf("Values:\nAvg = %f, Std Dev = %f\n", avg_value, std_dev);
    printf("Min = %d, Max = %d, Difference = %d\n", min_value, max_value, diff_value);
    
    printf("Voltages:\nAvg = %f, std dev = %f\n",  avg_voltage, std_dev_voltage);
    printf("Min = %f, Max = %f, Difference = %f\n", min_voltage, max_voltage, diff_voltage);

    
    double filtered_values[samplemax];
    double filtered_voltages[samplemax];    
    
    //filter the data
    //dc removal
    double meanval = 0.0;
    double meanvolt = 0.0;
    for (int i = 0; i < samplemax; i++) {
        meanval += valuesd[i];
        meanvolt += voltages[i];

    }
    meanval /= samplemax;
    meanvolt /= samplemax;

    double valuesddc[samplemax], voltagesdc[samplemax];
    for (int i = 0; i < samplemax; i++) {
        valuesddc[i] = valuesd[i]-meanval;
        voltagesdc[i] =  voltages[i]-meanvolt;
    }
    // Apply the FIR filter
    for (int i = 0; i < samplemax; i++) {
        filtered_values[i] = 0; // Initialize to 0
        filtered_voltages[i] = 0;

        for (int j = 0; j < BL; j++) {
            if (i - j < 0) continue; // Skip if out of bounds
            filtered_values[i] += valuesddc[i - j] * B[j];
            filtered_voltages[i] += voltagesdc[i - j] * B[j];

        }
    }

    //calculate post-filtering
    //calculate min, max, average, and difference

    // calculate min, max, average, and difference
    double min_value2 = filtered_values[0], max_value2 = filtered_values[0];
    double min_voltage2 = filtered_voltages[0], max_voltage2 = filtered_voltages[0];
    double sum_values2 = 0, sum_voltages2 = 0;

    for(int i = 0; i<samplemax; i++) 
    {
        if (filtered_values[i] < min_value2) min_value2 = filtered_values[i];
        if (filtered_values[i] > max_value2) max_value2 = filtered_values[i];
        sum_values2 += filtered_values[i];

        if (filtered_voltages[i] < min_voltage2) min_voltage2 = filtered_voltages[i];
        if (filtered_voltages[i] > max_voltage2) max_voltage2 = filtered_voltages[i];
        sum_voltages2 += filtered_voltages[i];
    }

    double avg_value2 = sum_values2 / samplemax;
    double avg_voltage2 = sum_voltages2 / samplemax;

    double diff_value2 = max_value2 - min_value2;
    double diff_voltage2 = max_voltage2 - min_voltage2;

    // calculate standard deviation of values and voltages
    double sum_squared_values2 = 0, sum_squared_voltages2 = 0;

    for(int i = 0; i<samplemax; i++) 
    {
        sum_squared_values2 += filtered_values[i] * filtered_values[i];
        sum_squared_voltages2 += filtered_voltages[i] * filtered_voltages[i];
    }

    double variance2 = sum_squared_values2 / samplemax - avg_value2 * avg_value2;
    double std_dev2 = sqrt(variance2);

    double variance_voltage2 = sum_squared_voltages2 / samplemax - avg_voltage2 * avg_voltage2;
    double std_dev_voltage2 = sqrt(variance_voltage2);

    //print resuslts post-filtering
    printf("\n\nPOST-FILTERING\n");
    printf("Values:\nAvg = %f, Std Dev = %f\n", avg_value2, std_dev2);
    printf("Min = %f, Max = %f, Difference = %f\n", min_value2, max_value2, diff_value2);
    
    printf("Voltages:\nAvg = %f, std dev = %f\n",  avg_voltage2, std_dev_voltage2);
    printf("Min = %f, Max = %f, Difference = %f\n", min_voltage2, max_voltage2, diff_voltage2);


    FILE *file = fopen("noise_calculation_data.csv", "w");
    if (file == NULL) {
        printf("Error opening file!\n");
        return 1;
    }

    // Add header
    fprintf(file, "Values, Voltage, Values_filtered, Voltages_filtered\n");

    for(int i = 0; i<samplemax; i++) 
    {
        fprintf(file, "%f, %f, %f, %f\n", valuesd[i], voltages[i], filtered_values[i], filtered_voltages[i]);
    }

    fclose(file);

    bcm2835_spi_end();
    bcm2835_close();

    system("python3 noiseCalculationPlotter.py");

    return 0;
}