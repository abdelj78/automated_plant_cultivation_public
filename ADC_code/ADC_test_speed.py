#this code will be measuring the sampling speed 

import spidev
import time
import binascii

spi = spidev.SpiDev() # create spi object
spi.open(0, 0) # open spi port 0, device (CS) 0

spi.mode = 0 
spi.max_speed_hz=8000000
#spi.bits_per_word=12
#set max speed clock
#set mode if needed
#set bit per word

valuedec=[]
try:
 while True:
    to_send = [0x00, 0x00]
    i=0

    start_time=time.time()

    while (time.time()-start_time) < 10: #record of 10 seconds
      
      #resp= spi.xfer(to_send, int(8000000), int(0.01)) # (message, clock speed, delay between blocks,bits per words)
      resp= spi.xfer(to_send) # (message, clock speed, delay between blocks,bits per words)
      valuedec.append(((resp[0] << 8) | resp[1]) >>2)  # moving first part to msb of 16bits then adding remaining lsb then moving first 2 bits as are not used
      i += 1

    final_time=time.time()

    print("number of recordings = ",i )
    print("recording frequency = ", i/(final_time-start_time))
    #print('received : 0x{0}' .format(binascii.hexlify(bytearray(resp))))
    #print(type(resp[0]))
    time.sleep(0.5) # sleep for 0.1 seconds
    #end while
except KeyboardInterrupt: # Ctrl+C pressed, so…
 spi.close() # … close the port before exit
#end try