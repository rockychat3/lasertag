#include <LaserUtils.h>                                                // the file that implements most of the behind-the-scenes code 
                                                                       //Pin assignments:
#define IR_RX 2                                                        // pin the IR receiver (left data pin) is connected to
#define IR_TX 13                                                       // pin the IR transmitter is connected to
#define BLASTER_TRIGGER 12                                             // pin the trigger is connected to
#define SPEAKER 11                                                     // pin the piezo speaker is connected to

int im_hit = false;                                                    // a true/false variable that records if you're being hit
bool shooting_now = false;                                             // a true/false variable that records if you're shooting
LaserRxTx laser = LaserRxTx(IR_RX, IR_TX);                             // an object that lets you fire and receive IR signals

void setup() {                                                         //Function run once when Arduino is powered on
  Serial.begin(9600);                                                  // so we can receive messages / watch for errors
  pinMode(BLASTER_TRIGGER, INPUT_PULLUP);                              // trigger as an input that defaults to high (5V)
  pinMode(SPEAKER, OUTPUT);                                            // setup speaker pin as an output 
  pinMode(IR_TX, OUTPUT);                                              // setup IR sending pin as an output 
  pinMode(IR_RX, INPUT);                                               // setup IR receiving pin as an input 
  attachInterrupt(digitalPinToInterrupt(IR_RX),irInterrupt,FALLING);   // the function irInterrupt is called when an IR signal is being received
}

void irInterrupt() {                                                   //Function called when the IR receiver sees a new message
  if (not shooting_now)                                                // ignore this interrupt if you're in the middle of sending (avoid self-interference)
    im_hit = true;                                                     // records that we have been hit, but doesn't take any action immediately, like leaving a note on the fridge
}

bool shot(char* attack_msg) {                                          //Function called when you get shot (it gets sent the shooter's codename)
  if (attack_msg == "")                                                // if no message received, cancel the shot
    return;  
  Serial.print("Attacked by: ");
  Serial.println(LaserMsg::getName(attack_msg));                       
}

// The main loop of code always running
void loop() {                                                          //Function run in a loop forever: checks if you're shooting and/or shot
 
  if (im_hit) {                                                        // when it checks here, if you were hit... 
    shot(laser.laserRecv());                                           // starts the irRecv() function and return the attacker's code to a function to do the shot logic
    im_hit = false;                                                    // reset the hit flag to false since you are no longer hit
  }
                                                                       
  if (digitalRead(BLASTER_TRIGGER) == LOW) {                           // next, check if you're trying to shoot
    Serial.println("Trigger pressed!");
    shooting_now = true;                                               // tell the program you're shooting so it doesn't cause issues with your own receiver
    laser.fireLaser("TEST");                                           // fire off a message
    tone(SPEAKER, 392);                                                // testing the sound 
    delay(200);
    tone(SPEAKER, 261);
    delay(200);
    noTone(SPEAKER); 
	delay(2000);
    // display test (simultaneous w/ piezo)
    shooting_now = false;                                              // tell the program you're done shooting
  }
}
