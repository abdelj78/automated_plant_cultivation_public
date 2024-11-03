Full System Test

FST1: simply doing some data reading once and plotting with pyhton once (no trigger check)
FST2: reading data with trigger on amplitude then fft done in c, then if plant sound signal to python file to count the frequency of the sounds
FST3:
FST4:
FST5: after realisind ML doesn't work, this will try to implement the final code without the real pump but with circular buffer method (can be found in PSD6), and without passing data to python, just sending signal
FST6: DO NOT EDIT FST6 ANYMORE, VERSION WORKING IN MY ROOM FINE Integrate FST5 with real pump but as pump make noise and trigger we will try to stop sound readings during pump work using signal communication 
FST7: is an attempt to optimise FST6 without modifying it before the viva
FST8: try to use pigpio for more accurate volume measurement