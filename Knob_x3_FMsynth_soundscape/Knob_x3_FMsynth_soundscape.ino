/*
   Mozzi is licensed under the GNU Lesser General Public Licence (LGPL) Version 2.1 or later.
*/

#define MOZZI_ANALOG_READ_RESOLUTION 10 // code below assumes readings to be in the classic 10-bit (0-1023) range
#include <Mozzi.h>
#include <Oscil.h> // oscillator
#include <tables/cos2048_int8.h> // table for Oscils to play
#include <Smooth.h>
#include <AutoMap.h> // maps unpredictable inputs to a range
#include "Ultrasonic.h"

Ultrasonic ultrasonic(7);

// desired carrier frequency max and min, for AutoMap
const int MIN_CARRIER_FREQ = 22;
const int MAX_CARRIER_FREQ = 440;

// desired intensity max and min, for AutoMap, note they're inverted for reverse dynamics
const int MIN_INTENSITY = 700;
const int MAX_INTENSITY = 10;

// desired mod speed max and min, for AutoMap, note they're inverted for reverse dynamics
const int MIN_MOD_SPEED = 10000;
const int MAX_MOD_SPEED = 1;

AutoMap kMapCarrierFreq(0,1023,MIN_CARRIER_FREQ,MAX_CARRIER_FREQ);
AutoMap kMapIntensity(0,1023,MIN_INTENSITY,MAX_INTENSITY);
AutoMap kMapModSpeed(0,1023,MIN_MOD_SPEED,MAX_MOD_SPEED);

 // set the input for the knob to analog pin 1
const int KNOB2_PIN = 2; // set the analog input for fm_intensity to pin 2
const int KNOB3_PIN = 3; // set the analog input for mod rate to pin 3

Oscil<COS2048_NUM_CELLS, MOZZI_AUDIO_RATE> aCarrier(COS2048_DATA);
Oscil<COS2048_NUM_CELLS, MOZZI_AUDIO_RATE> aModulator(COS2048_DATA);
Oscil<COS2048_NUM_CELLS, MOZZI_CONTROL_RATE> kIntensityMod(COS2048_DATA);

int mod_ratio = 5; // brightness (1 - 10)
long fm_intensity; // carries control info from updateControl to updateAudio

// smoothing for intensity to remove clicks on transitions
float smoothness = 0.95f;
Smooth <long> aSmoothIntensity(smoothness);



void setup(){
  Serial.begin(115200); // set up the Serial output so we can look at the piezo values // set up the Serial output so we can look at the light level
  startMozzi(); // :))
}


void updateControl(){

  long RangeInCentimeters;

  // read the knob
  int knob_1 = ultrasonic.MeasureInCentimeters()/5; // value is 0-1023

  // map the knob to carrier frequency
  int carrier_freq = kMapCarrierFreq(knob_1);

  //calculate the modulation frequency to stay in ratio
  int mod_freq = carrier_freq * mod_ratio;

  // set the FM oscillator frequencies
  aCarrier.setFreq(carrier_freq);
  aModulator.setFreq(mod_freq);

  // read the knob2 on the width Analog input pin
  int knob_2 = 500;//mozziAnalogRead(KNOB2_PIN); // value is 0-1023

  int knob2_calibrated = kMapIntensity(knob_2);

 // calculate the fm_intensity
  fm_intensity = ((long)knob2_calibrated * (kIntensityMod.next()+128))>>8; // shift back to range after 8 bit multiply

  // read the light dependent resistor on the speed Analog input pin
  int knob_3 = 500;//mozziAnalogRead(KNOB3_PIN); // value is 0-1023

  // use a float here for low frequencies
  float mod_speed = (float)kMapModSpeed(knob_3)/1000;
  Serial.print("   mod_speed = ");
  Serial.print(mod_speed);
  kIntensityMod.setFreq(mod_speed);

  Serial.println();
}



AudioOutput updateAudio(){
  long modulation = aSmoothIntensity.next(fm_intensity) * aModulator.next();
  return MonoOutput::from8Bit(aCarrier.phMod(modulation));
}


void loop(){
  audioHook();
}
