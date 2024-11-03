#include <bcm2835.h>
#include <stdio.h>
#include <unistd.h>
#include </usr/include/time.h>


int main(int argc, char **argv)
{
    uint16_t array[] = {4,5,7,6};

    FILE *file; //file pointer variable
    file = fopen("test.csv","w"); //fopen is going to return a pointer that is gonna be stored in file

    if (file==NULL) //check if file is opened successfuly 
    {
        printf("error opening file.\n");
        return 1;
    }

    for(int i=0; i<4; i++)
    {
        printf("%d\n", i);
        fprintf(file, "%d, %d\n", i, array[i]);
        
        if (ferror(file))
        {
            printf("error writing to file.\n");
            return 1;
        }
    }

    fclose(file);
    printf("done!!!\n");
    
    return 0;
}