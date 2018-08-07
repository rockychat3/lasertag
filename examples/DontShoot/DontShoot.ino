#include <LaserUtils.h>                                                // the file that implements most of the behind-the-scenes code 
                                                                      //Pin assignments:
#define IR_RX 2                                                        // pin the IR receiver (left data pin) is connected to
#define SPEAKER 11                                                     // pin the piezo speaker is connected to

#define MSG_LENGTH 16                                                  // length of transmitted and received messages (must be same on ALL blasters)
                                                                       //Variables used throughout the code:
int im_hit = false;                                                    // a true/false variable that records if you're being hit
LaserRxTx laser = LaserRxTx(IR_RX, 13);                             // an object that lets you fire and receive IR signals

void setup() {                                                         //Function run once when Arduino is powered on
  Serial.begin(9600);                                                  // so we can receive messages / watch for errors
  pinMode(SPEAKER, OUTPUT);                                            // setup speaker pin as an output 
  pinMode(13, OUTPUT);
  pinMode(IR_RX, INPUT);                                               // setup IR receiving pin as an input 
  attachInterrupt(digitalPinToInterrupt(IR_RX),irInterrupt,FALLING);   // the function irInterrupt is called when an IR signal is being received
}


void irInterrupt() {                                                   //Function called when the IR receiver sees a new message
  im_hit = true;                                                       // records that we have been hit, but doesn't take any action immediately, like leaving a note on the fridge
}


void shot(char* secret_msg) {                                          //Function called when you get shot (it gets sent the shooter's codename)
  if (secret_msg == "")                                                // if no message received, cancel the shot
    return;                                                             
  digitalWrite(13, HIGH);
  for (int i=0; i<10; i++) {
	tone(SPEAKER, 392);                                              // sound played when you get the message 
    delay(200);
    tone(SPEAKER, 261);
    delay(200);
  }
  noTone(SPEAKER);
  digitalWrite(13, LOW);  
}


void loop() {                                                          //Function run in a loop forever: checks if you're shooting and/or shot
  if (im_hit) {                                                        // when it checks here, if you were hit... 
    shot(laser.irRecv(MSG_LENGTH));                                    // starts the irRecv() function and return the sender's code to a function to do the shot logic
    im_hit = false;                                                    // reset the hit flag to false since you are no longer hit
  }
}
