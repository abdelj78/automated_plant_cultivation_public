import wave
import numpy as np
import matplotlib.pyplot as plt

# Open the .wav file
wav_file = wave.open('C:/Users/abdel/Documents/GitHub/year4_project/STM32_POC/raw_sound_generation/wavFiles/id_1_sound_1.wav', 'r')

# Print the properties of the .wav file
print("Number of channels:", wav_file.getnchannels())
print("Sample width (bytes):", wav_file.getsampwidth())
print("Frame rate (samples per second):", wav_file.getframerate())
print("Number of frames:", wav_file.getnframes())
print("Compression type:", wav_file.getcomptype())
print("Compression name:", wav_file.getcompname())
# Calculate the recording time
recording_time = wav_file.getnframes() / wav_file.getframerate()
print("Recording time (s):", recording_time)

# Extract Raw Audio from Wav File
signal = wav_file.readframes(-1) #-1 means read all frames
signal = np.frombuffer(signal, dtype='int16')

# Compute the FFT and the frequency axis
fft_result = np.fft.rfft(signal)
sample_rate = 500000
freq = np.fft.rfftfreq(len(signal), 1/sample_rate)

# Find the peak frequency
peak_freq = freq[np.argmax(np.abs(fft_result))]

print("Peak frequency (Hz):", peak_freq)

# Get the sound wave time array
time = np.linspace(0., len(signal) / wav_file.getframerate(), num=len(signal))

# Create a figure with two subplots
fig, axs = plt.subplots(2, 1, figsize=(15, 10))

# Plot the audio signal in the first subplot
axs[0].plot(time, signal)
axs[0].set_title('Original Audio Signal')
axs[0].set_xlabel('Time (s)')
axs[0].set_ylabel('Amplitude')
axs[0].grid()

# Plot the frequency response in the second subplot
axs[1].plot(freq, np.abs(fft_result))
axs[1].set_title('Frequency Response')
axs[1].set_xlabel('Frequency (Hz)')
axs[1].set_ylabel('Magnitude')
axs[1].grid()

# Show the plots
plt.tight_layout()
plt.show()