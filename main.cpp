/* Description: McLab15_SensorAnaMic_OS6_tk1
 * Hardware: L432KC or similar.
 * For testing a microphone sensor 
 * Grove Analog Microphone or similar. 
 * The microphone signal is zero centered!
 * Connect:
 * L432KC A6 -- sensor output (Out)
 * GND rail  -- senror GND (GND)
 * 3.3V rail -- sensor Power (5V)
 * 
 * L432KC LD3 is permanently connected to D13
 * and can be named LED1 in the code.
 * Operating system OS 6.0...6.16
 * Timo Karppinen 13.12.2022  Apache-2.0
 */

#include "mbed.h"
// Sampling rate n milliseconds
#define SAMPLING_RATE     1ms
// Output change every n seconds
#define OUTPUT_UPDATE     100ms
// Microphone signal gain in decibel
//  x dB => gain = 20*log(x)
//  6 dB => gain = 2.0
// 20 dB => gain = 10.0
// 30 dB => gain = 31.62 
// 40 dB => gain = 100.0
#define micGain  31.62

float mSampleMean = 0.0;  // from 0.0 to 1.0
int mSampleMeanInt = 0;   // from 0 to 100

// An other thread for timing the sensor signal sampling
Thread thread2; 

void sample(){
    // Initialise the analog pin A5 as an analog input
   
    AnalogIn mSen(A6);
    // Variables for the microwave signal samples
    float iSample;                  // from 0.000 to 1.0
    float aSample;
    static float mSamples[100];     // from 0.0 to 1.0 
    static float mSampleSum = 0;    // better initialize with 0
    static int index =0;
    
    while (true) {
        iSample = mSen.read();      // 12bit ADC Resolution 1/4095=0.00025 
        aSample = micGain * iSample;  
        // 0.5 + micGain * (iSample - 0.5)  // if Zero level is 0.5
        // For inverting peaks when peaks are negative. Zero level is 0.5 
        // Remove the next if statement if inverting peaks is not needed. 
        //if(aSample < 0.5){
        //    aSample = 1 - aSample;
        //}
        mSamples[index] = aSample;
        // Printing for first tests. Sampling 100 ms, update 10 s
        // Comment the print line. Change sample and out update periods
        // into about a few millisseconds and one second. 
        // printf("iSample%d mwSample%d at%d\n",(int)(1000*iSample), (int)(1000*mwSamples[index]), index);
        if(index < 99){
            mSampleSum = mSampleSum + mSamples[index] - mSamples[index+1];
        }
        if(index >= 99){
            mSampleSum = mSampleSum + mSamples[99] - mSamples[0];
            index = -1;
        }
        index = index +1;
        mSampleMean = mSampleSum/100;
        ThisThread::sleep_for(SAMPLING_RATE);
    }
}

int main()
{
    // Initialise the digital pin LED1 as an output
    DigitalOut ledA(LED1);
    int ledAState = false;

    thread2.start(callback(sample));

    while (true) {
        ledA.write(ledAState);
        mSampleMeanInt = (int)(1000*mSampleMean);
        if(mSampleMeanInt > 200){    // if zero level 0.5 then > (500+200)
            ledAState = true;
        }  else{
            ledAState = false; 
        }
        ledA.write(ledAState);
        printf("Mic Sensor mean reading  %d Peak %d\n", mSampleMeanInt, ledAState );
        ThisThread::sleep_for(OUTPUT_UPDATE);
    }
}