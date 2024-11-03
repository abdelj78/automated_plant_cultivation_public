#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "fdacoefs.h"

#define MAX_ROWS 5000

int main() {
    int data[MAX_ROWS];
    double data_double[MAX_ROWS];
    double filtered_data[MAX_ROWS];
    int filtered_data_int[MAX_ROWS];

    int row_count = 0;

    FILE *file = fopen("ADC_test4_data2.csv", "r");

    if (file == NULL) {
        printf("Could not open file.\n");
        return 1;
    }

    char buffer[1024];
    fgets(buffer, sizeof(buffer), file);

    // Read the second column of every row
    while (fscanf(file, "%*d,%d", &data[row_count]) != EOF && row_count < MAX_ROWS) {
        row_count++;
    }

    fclose(file);
    printf("data read done\n");
    // Print the data to verify it was loaded correctly
    /*for (int i = 0; i < row_count; i++) {
        printf("%d\n", data[i]);
    }*/

    //////////SIGNAL PROCESSING///////
    // Start the timer
    clock_t start = clock();
    //DC OFFSET REMOVAL, Calculate the mean of the data
    double mean = 0.0;
    for (int i = 0; i < row_count; i++) {
        mean += data[i];
    }
    mean /= row_count;

    // Subtract the mean from each data point to remove the DC offset
    for (int i = 0; i < row_count; i++) {
        data[i] -= mean;
    }

    printf("DC removal done\n");

    //FIR FILTER
    // Convert the data to double
    for (int i = 0; i < row_count; i++) {
        data_double[i] = (double)data[i];
    }

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
    //NORMALIZATION
    
    // Stop the timer
    clock_t end = clock();

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
    char valuename[] = "value";
    char filteredname[] = "filtered";


    fprintf(fileout, "%s, %s, %s\n", index, valuename,filteredname);
    for(int i=0; i<MAX_ROWS; i++)
    {
        //printf("%d\n", i);
        fprintf(fileout, "%d, %d, %d\n", i, data[i], filtered_data_int[i]);
        //fprintf(file, "%d\n", values[i]);
        if (ferror(fileout))
        {
            printf("error writing to file.\n");
            return 1;
        }
    }
    printf("writing done\n");

    fclose(fileout);

    system("python3 SigFFT.py");
    
    return 0;
}