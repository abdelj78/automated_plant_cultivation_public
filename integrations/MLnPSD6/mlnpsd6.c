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

#define samplemax 10000 //for 20ms)
//#define samplemax2 2048 //for 4.096ms (2^11)
#define samplemax2 1024 //for 2.048ms

#define BUFFER_SIZE 1001  // Set this to the desired buffer size
#define CENTER_INDEX (BUFFER_SIZE / 2)

//int samplemax= 5000; // 5000 for 10ms //1000000; //using too large values creates semgmentation faults
int sample=0;
//double start, end; 
char tbuf[] = {0x01, 0x02}; // Data to send
char rbuf[] = {0x00, 0x00};

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
int index_trig = 0;
int index_end = 0;
int index_start = 0;
bool csv_free = true;

//function prototypes
void *setupbcm2835(void *arg);
void handle_sigint(int sig);
void removeDCOffset(double* buffer, double average);
void csvsave(double* buffer);
// Define the signal handler function
void handle_sigusr1(int signum) {
    // Add your routine here...
    printf("\nC/ Received SIGUSR1\n");
    csv_free = true;
}

int main(int argc, char *argv[])
{
    printf("C/ Starting the C program\n");
    // Check if a PID was provided
    if (argc < 2) {
        printf("\nC/ Please provide the PID of the Python script");
        return 1;
    }
    // Convert the PID to an integer
    pid = atoi(argv[1]);
    printf("\nC/ PID of python in main: %d", pid);
    sleep(5); //wait for python to be ready

    // Set up the signal handler
    signal(SIGINT, handle_sigint);
    signal(SIGUSR1, handle_sigusr1);
    setupbcm2835(NULL);

    // Allocate FFTW complex output array
    fft_result = fftw_alloc_complex(BUFFER_SIZE);

    // //threshold calculation in ADC value 
    // thresholdv = 0.5;  //V , 140mV should be the max value of plant sound 
    // threshold = thresholdv*4095/3.3;  // equivalent ADC value 
    // printf("\nC/ Threshold(V): %f", thresholdv);
    // printf("\nC/ Threshold(ADC): %f\n\n", threshold);
    
    double buffer[BUFFER_SIZE];
    double bufferc[BUFFER_SIZE];
    int buffer_index = 0;
    bool triggered = false;

    double sum = 0;
    int count = 0;
    double average;
    double percentage = 1.5;  // 1+percentage/100 because signal not centered on 0 
    double sample=0;
    uint16_t sample16=0;
    int pltcnt = 0;
    int trignb =0;
    

    while(1){

        bcm2835_spi_transfernb(tbuf, rbuf, sizeof(tbuf));
        sample16 = (0x0FFF & ((((uint16_t)(rbuf[0])<<8) | ((uint16_t)rbuf[1]))>>2));
        sample = (double) sample16;  
        
        buffer[buffer_index] = sample;
        buffer_index = (buffer_index + 1) % BUFFER_SIZE;

        sum += sample;
        count = count < BUFFER_SIZE ? count + 1 : BUFFER_SIZE;
        average = sum / count;

        if (!triggered && sample > average * percentage) {
            triggered = true;
            index_trig = (buffer_index-1+BUFFER_SIZE) % BUFFER_SIZE;
            //record target index
            index_end = (index_trig-200 + BUFFER_SIZE) % BUFFER_SIZE;
            index_start = (index_end +1) % BUFFER_SIZE;
        }

        // if buffer has finised filling and the trigger has been activated
        if (triggered && buffer_index == index_end) {
            // The buffer now contains the signal centered around the peak.
            // You can process or save the buffer here.
            trignb++;
            printf("trig nb %d and centered\n", trignb);
            // center the buffer in new array
            for (int i = 0; i < BUFFER_SIZE; i++) {
                bufferc[i] = buffer[(index_start + i) % BUFFER_SIZE];
            }
            
            removeDCOffset(bufferc, average);
            //send values to python by writing to a csv file
            printf("C/ DC removal done waiting for access to csv\n");
            while(!csv_free){/*wait for python to free the csv file*/}
            csvsave(bufferc);
            
            //send signal to python
            csv_free = false;
            kill(pid, SIGUSR1); 
            printf("C/ csv file saved and signal sent\n");

            // Reset for the next trigger
            triggered = false;
        }

        // Remove the oldest sample from the sum when the buffer is full
        if (count == BUFFER_SIZE) {
            sum -= buffer[buffer_index];
        }

    }
    return 0;
}

void removeDCOffset(double* bufferc, double average_given) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        bufferc[i] -= average_given;
    }
}


void csvsave(double* buffer_given){

    sprintf(filename, "relay.csv");  // Format the filename
    FILE *file; //file pointer variable
    file = fopen(filename,"w"); //fopen is going to return a pointer that is gonna be stored in file

    if (file==NULL) //check if file is opened successfuly 
    {
        printf("error opening file.\n");
        return;
    }
    //char audioname[] = "raw_audio";
    char valuename[] = "raw_audio";

    fprintf(file, "%s\n", valuename);
    
    if (ferror(file))
    {
        printf("error writing to file.\n");
        return;
    }

    for(int i=0; i<BUFFER_SIZE; i++)
    {
        fprintf(file, "%f\n", buffer_given[i]);
        if (ferror(file))
        {
        printf("error writing to file.\n");
        return;
        }
    }

    fclose(file);
    return;  
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

