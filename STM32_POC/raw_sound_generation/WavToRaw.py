import numpy as np
import wave
import matplotlib.pyplot as plt

# Open the .wav file
wav_file = wave.open('C:/Users/abdel/Documents/GitHub/year4_project/STM32_POC/raw_sound_generation/wavFiles/id_1_sound_1.wav', 'r')

# Extract Raw Audio from Wav File
signal = wav_file.readframes(-1)
signal = np.frombuffer(signal, dtype='int16')

# Normalize the audio data to the range -1 to 1
max_value = np.max(np.abs(signal))
signal = signal / max_value

# Shift and scale the data to range from 0 to 4095
signal = (signal + 1) * 2047

# Convert the audio data to uint32_t
signal = signal.astype('uint32')

# Plot the final stage of the signal
plt.figure(figsize=(10, 4))
plt.plot(signal)
plt.title('Final Stage of Signal')
plt.xlabel('Sample')
plt.ylabel('Amplitude')
plt.grid()
plt.show()

# Open the .h file
h_file = open('C:/Users/abdel/Documents/GitHub/year4_project/STM32_POC/raw_sound_generation/audio_data.h', 'w')

# Write the array to the .h file
h_file.write('const int16_t audio_data[] = {')
for sample in signal:
    h_file.write(str(sample) + ', ')
h_file.write('};\n')

# Close the .h file
h_file.close()