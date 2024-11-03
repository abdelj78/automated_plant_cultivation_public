# This directory is dedicated for code to be used for collection and recoding of audio signal from the microphone
03/03/2024: The code is based on the bcm2835.h library for data communication. The signal is then written to a csv file.
            The signal is also visualised using pyhton and matplotlib (this last one should have its own directory)
            Different test samples have been recorded. These are samples of sin waves of different frequencies.

BCM2835:
    the library version used is bcm2835-1.68 (other versions might not work)
        so far this version gave the best results (assessed visually), further investigation can be done later
        version 1.75 seems to generate noisy signal, this noise is most likely caused by wrong reading in the spi protocol
    
    Use the following command to compile and run:
        sudo gcc -o writesoundtocsv writesoundtocsv.c -l bcm2835
        sudo ./writesoundtocsv 

    