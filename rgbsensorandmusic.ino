/*
 Based off code from:
 Spark Fun Electronics 2011
 Nathan Seidle
 */
#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include "RGBConverter.h"
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3); //Soft TX on 3, we don't use RX in this code
 
byte note = 0; //The MIDI note value to be played
byte resetMIDI = 4; //Tied to VS1053 Reset line
byte ledPin = 13; //MIDI traffic inidicator
int instrument = 0;

/* Initialise with default values (int time = 2.4ms, gain = 1x) */
Adafruit_TCS34725 tcs = Adafruit_TCS34725();
uint16_t r, g, b, c, colorTemp, lux;
RGBConverter rgbConverter = RGBConverter();
double hsv[3];
double h,s,v,ex_h=0,ex_s=0,ex_v=0;
/*music loop array*/
// point to no.j element in the array 
int j;
int a_min_chord[] = {58, 60, 61, 63, 65, 66, 68, 70, 72, 73, 75, 77, 78, 80};
int a_maj_chord[] = {58, 60, 62, 63, 65, 67, 69, 70, 72, 74, 75, 77, 79, 81};
int b_min_chord[] = {60, 62, 63, 65, 67, 68, 70, 72, 74, 75, 77, 79, 80, 82};
int b_maj_chord[] = {60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79, 81, 83};
int c_min_chord[] = {61, 63, 64, 66, 68, 69, 71, 72, 75, 76, 78, 80, 81, 83};
int c_maj_chord[] = {61, 63, 65, 66, 68, 70, 72, 73, 75, 77, 78, 80, 82, 84};
int d_min_chord[] = {60, 61, 63, 65, 66, 68, 70, 71, 73, 75, 76, 78, 80, 81};
int d_maj_chord[] = {60, 62, 63, 65, 67, 68, 70, 72, 74, 75, 77, 79, 80, 82};
int e_min_chord[] = {62, 63, 65, 67, 68, 70, 72, 73, 75, 77, 78, 80, 82, 83};
int e_maj_chord[] = {62, 64, 65, 67, 69, 70, 72, 74, 76, 77, 79, 81, 82, 83};
int f_min_chord[] = {61, 63, 65, 67, 68, 70, 72, 73, 75, 77, 78, 80, 82, 83};
int f_maj_chord[] = {61, 63, 65, 66, 68, 70, 71, 73, 75, 77, 78, 80, 82, 84};
int g_min_chord[] = {62, 63, 65, 66, 68, 70, 71, 73, 75, 76, 78, 80, 81, 83};
int g_maj_chord[] = {62, 63, 65, 67, 68, 70, 72, 73, 75, 77, 79, 80, 82, 84};
//int a_min_chord[] = {58, 60, 61, 63, 65, 66, 68};
//int a_maj_chord[] = {58, 60, 62, 63, 65, 67, 69};
//int b_min_chord[] = {60, 62, 63, 65, 67, 68, 70};
//int b_maj_chord[] = {60, 62, 64, 65, 67, 69, 71};
//int c_min_chord[] = {61, 63, 64, 66, 68, 69, 71};
//int c_maj_chord[] = {61, 63, 65, 66, 68, 70, 72};
//int d_min_chord[] = {63, 65, 66, 68, 70, 71, 73};
//int d_maj_chord[] = {63, 65, 67, 68, 70, 72, 74};
//int e_min_chord[] = {65, 67, 68, 70, 72, 73, 75};
//int e_maj_chord[] = {65, 67, 69, 70, 72, 74, 76};
//int f_min_chord[] = {66, 68, 69, 71, 73, 74, 76};
//int f_maj_chord[] = {66, 68, 70, 71, 73, 75, 77};
//int g_min_chord[] = {68, 70, 71, 73, 75, 76, 78};
//int g_maj_chord[] = {68, 70, 72, 73, 75, 77, 79};
int *p = &a_min_chord[0];//set a pointer
int interval = 0;
void setup() {
  Serial.begin(57600);
 
  //Setup soft serial for MIDI control
  mySerial.begin(31250);
 
  //Reset the VS1053
  pinMode(resetMIDI, OUTPUT);
  digitalWrite(resetMIDI, LOW);
  delay(100);
  digitalWrite(resetMIDI, HIGH);
  delay(100);

  //RGB Sensor start
  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
}
 
void loop() {

  senseRGB();
  talkMIDI(0xB0, 0x07, 126); //0xB0 is channel message, set channel volume to near max (127)
 
  Serial.println("Basic Instruments");
  talkMIDI(0xB0, 0, 0x00); //Default bank GM1

 
  playLoop();
//  play_note(0, 84, 90, 150);
//  play_note(1, 88, 90, 150);
//  play_note(2, 91, 90, 150);
  delay(10); //Delay between instruments
}

void playLoop(){
  h = hsv[0]*60*6;
  s = hsv[1]*100;
  v = hsv[2]*100;
  interval++;
  //every 4 second if the color changed then change HS
  if(interval == 23 && (h != ex_h || s != ex_s)){
    changeHS();
    ex_h = h;
    ex_s = s;
    interval = 0;
  }
  //as long as the v changed, play different note
  Serial.print("v="+(String)v);
  if(v != ex_v){
   changeV();
   ex_v = v; 
  } 
  Serial.print("j=");
  Serial.println(j);
  Serial.println(*(p+j));
  Serial.print(".........");
  if(h<5 && s<5 && v<5){
    note = 0;
  }else{
    note = *(p+j);
  }
  
  instrument = 0;
  //  instrument = 41;
  //Change to different instrument
  talkMIDI(0xC0, instrument, 0); //Set instrument number. 0xC0 is a 1 data byte command
  play_note(0, note, 90, 100);
}
void changeHS(){
  Serial.println("changeHS");
  if(s < 50){
    if(h < 50){
      p = &a_min_chord[0];
    }else if(h < 70){
      p = &b_min_chord[0];
    }else if(h < 150){
      p = &c_min_chord[0];
    }else if(h < 190){
      p = &d_min_chord[0];
    }else if(h < 270){
      p = &e_min_chord[0];
    }else if(h < 330){
      p = &f_min_chord[0];
    }else if(h < 330){
      p = &f_min_chord[0];
    }
  }else{
    if(h < 50){
      p = &a_maj_chord[0];
    }else if(h < 70){
      p = &b_maj_chord[0];
    }else if(h < 150){
      p = &c_maj_chord[0];
    }else if(h < 190){
      p = &d_maj_chord[0];
    }else if(h < 270){
      p = &e_maj_chord[0];
    }else if(h < 330){
      p = &f_maj_chord[0];
    }else if(h < 330){
      p = &f_maj_chord[0];
    }
  } 
}
void changeV(){
  Serial.println("changeV");
  // v controls the number
//  if(v < 14){
//    j = 0;  
//  }else if(v < 28){
//    j = 1;
//  }else if(v < 42){
//    j = 2;
//  }else if(v < 56){
//    j = 3;
//  }else if(v < 70){
//    j = 4;
//  }else if(v < 84){
//    j = 5;
//  }else{
//    j = 6;
//  }
  if(v < 7){
    j = 0;  
  }else if(v < 14){
    j = 1;
  }else if(v < 21){
    j = 2;
  }else if(v < 28){
    j = 3;
  }else if(v < 35){
    j = 4;
  }else if(v < 42){
    j = 5;
  }else if(v < 49){
    j = 6;
  }else if(v < 56){
    j = 7;
  }else if(v < 63){
    j = 8;
  }else if(v < 70){
    j = 9;
  }else if(v < 77){
    j = 10;
  }else if(v < 84){
    j = 11;
  }else if(v < 91){
    j = 12;
  }else{
    j = 13;
  }
}
void play_note(byte channel, byte note, byte attack_velocity, byte length) {
  //Note on channel 1 (0x90), some note value (note), middle velocity (0x45):
  noteOn(channel, note, attack_velocity);
  delay(length);
  //Turn off the note with a given off/release velocity
  noteOff(channel, note, attack_velocity);
  delay(10);  
}

//Send a MIDI note-on message.  Like pressing a piano key
//channel ranges from 0-15
void noteOn(byte channel, byte note, byte attack_velocity) {
  talkMIDI( (0x90 | channel), note, attack_velocity);
}
 
//Send a MIDI note-off message.  Like releasing a piano key
void noteOff(byte channel, byte note, byte release_velocity) {
  talkMIDI( (0x80 | channel), note, release_velocity);
}
 
//Plays a MIDI note. Doesn't check to see that cmd is greater than 127, or that data values are less than 127
void talkMIDI(byte cmd, byte data1, byte data2) {
  
  digitalWrite(ledPin, HIGH);
  mySerial.write(cmd);
  mySerial.write(data1);
 
  //Some commands only have one data byte. All cmds less than 0xBn have 2 data bytes 
  //(sort of: http://253.ccarh.org/handout/midiprotocol/)
  if( (cmd & 0xF0) <= 0xB0)
    mySerial.write(data2);
 
  digitalWrite(ledPin, LOW);
}

//sense RGB
void senseRGB(){
  tcs.getRawData(&r, &g, &b, &c);
  colorTemp = tcs.calculateColorTemperature(r, g, b);
  lux = tcs.calculateLux(r, g, b);
  rgbConverter.rgbToHsv(r, g, b, hsv);
  Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
  Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
  Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
  Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
  Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
  Serial.print("H: "); Serial.print(hsv[0]*60*6, DEC); Serial.print(" ");
  Serial.print("S: "); Serial.print(hsv[1], DEC); Serial.print(" ");
  Serial.print("V: "); Serial.print(hsv[2], DEC); Serial.print(" ");
  Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
  Serial.println(" ");
}

//convert RGB to HSB

