/*
   Mozzi is licensed under the GNU Lesser General Public Licence (LGPL) Version 2.1 or later.
*/

#define MOZZI_ANALOG_READ_RESOLUTION 10 // code below assumes readings to be in the classic 10-bit (0-1023) range
#include <Mozzi.h>
#include <WavePacket.h>
#include <RollingAverage.h>
#include <AutoMap.h>
#include <IntMap.h>

const int KNOB1_PIN = 1; // set the input for the knob to analog pin 0
const int KNOB2_PIN = 2; // set the analog input for fm_intensity to pin 1
const int KNOB3_PIN = 3; // set the analog input for mod rate to pin 2

// min and max values of synth parameters to map AutoRanged analog inputs to
const int MIN_F = 20;
const int MAX_F = 150;

const int MIN_BW = 20;
const int MAX_BW = 600;

const int MIN_CF = 60;
const int MAX_CF = 600;


// for smoothing the control signals
// use: RollingAverage <number_type, how_many_to_average> myThing
RollingAverage <int, 16> kAverageF;
RollingAverage <int, 16> kAverageBw;
RollingAverage <int, 16> kAverageCf;

// Intmap is a pre-calculated faster version of Arduino's map, OK for pots
IntMap kMapF(0,1023,MIN_F,MAX_F);
// AutoMap adapts to range of input as it arrives, useful for LDR's
AutoMap kMapBw(0,1023,MIN_BW,MAX_BW);
AutoMap kMapCf(0,1023,MIN_CF,MAX_CF);

WavePacket <DOUBLE> wavey; // DOUBLE selects 2 overlapping streams

void setup(){
  //Serial.begin(9600); // for Teensy 3.1, beware printout can cause glitches
  Serial.begin(115200);
  // wait before starting Mozzi to receive analog reads, so AutoRange will not get 0
  delay(200);
  startMozzi();
}


void updateControl(){
  int fundamental = mozziAnalogRead(KNOB1_PIN)+1;
  fundamental = kMapF(fundamental);
  //Serial.print("f=");
  //Serial.print(fundamental);

  int bandwidth = mozziAnalogRead(KNOB2_PIN);
  bandwidth = kMapBw(bandwidth);
  //Serial.print("\t bw=");
  //Serial.print(bandwidth);

  int centre_freq = mozziAnalogRead(KNOB3_PIN);
  centre_freq = kMapCf(centre_freq);
  //Serial.print("\t cf=");
  //Serial.print(centre_freq);

  Serial.println();

  wavey.set(fundamental, bandwidth, centre_freq);
}



AudioOutput updateAudio(){
  return MonoOutput::from16Bit(wavey.next());
}



void loop(){
  audioHook(); // required here
}
