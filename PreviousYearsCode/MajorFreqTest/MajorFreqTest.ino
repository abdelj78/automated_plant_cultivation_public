

#include <arduinoFFT.h>

#define SAMPLES 1024
#define SAMPLING_FREQUENCY 100000
#define AUDIO_IN_PIN 34

arduinoFFT FFT=arduinoFFT();
unsigned int samplingPeriod;
unsigned long newTime;

double vReal[SAMPLES];
double vImag[SAMPLES];

void setup()
{
  Serial.begin(9600);
  samplingPeriod=round(1000000*(1.0/SAMPLING_FREQUENCY));
 
}

void loop()
{
  for(int i=0; i<SAMPLES; i++)

  {
  newTime=micros();

  vReal[i]=analogRead(AUDIO_IN_PIN);
  vImag[i]=0;

  while(micros()< (newTime+samplingPeriod))
    {
    //do nothing
    }
  }

FFT.Windowing(vReal,SAMPLES, FFT_WIN_TYP_HAMMING,FFT_FORWARD);
FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

double peak=FFT.MajorPeak(vReal,SAMPLES, SAMPLING_FREQUENCY);
Serial.println(peak);
//while(1);
}
