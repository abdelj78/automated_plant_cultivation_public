#this code shows that the ADC seem to work and binary value increses as voltage increases 
#but the reading is too slow 
import spidev
import time
import binascii

spi = spidev.SpiDev() # create spi object
spi.open(0, 0) # open spi port 0, device (CS) 0

spi.mode = 0 
#spi.max_speed_hz=8000000
#spi.bits_per_word=12
#set max speed clock
#set mode if needed
#set bit per word


try:
 while True:
    start_time=time.time()
    resp = spi.readbytes(2) # transfer one byte
    timetotal=time.time()-start_time
    print(timetotal)
    #print('received : 0x{0}' .format(binascii.hexlify(bytearray(resp))))
    print('received : {0:08b}'.format(resp[0]), end=" ")
    print('{0:08b}'.format(resp[1]))
    #print(type(resp[0]))
    time.sleep(0.5) # sleep for 0.1 seconds
    #end while
except KeyboardInterrupt: # Ctrl+C pressed, so…
 spi.close() # … close the port before exit
#end try