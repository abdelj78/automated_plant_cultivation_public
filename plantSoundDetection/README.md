# This directory is dedicated for code to be used for the detection and recording of plant sounds only 
03/03/2024: Using all the other work, this directory should aim to listen and identify plant sounds to be recorded. 

PSD_v1: was just early practice, nothing tested
PSD_v2: has for aim to record the 5000 samples if the trigger occurs anytime during the 5000 samples
PSD_v3: has for aim to record 5000 samples, uses multithreading and double buffer method, FFT analysis to save only plant sound
PSD_v4: 

PSD_v7: same as v6 but design for long time experiment and count number of plant sounds (uses circular buffer)
PSD_v8: ultimate plant sound detection system post-viva. Try to use double buffer for simplicity and other checking methods to filter other ultrasound noise.



PLANT SOUND DETECTED!!!
Real plant sound detection achieved with PSD_v7 and data of experiment saved in /Triggered_plant_exp1_results
results are analysed and compared in "exp1_results_analysis.ipynp notebook