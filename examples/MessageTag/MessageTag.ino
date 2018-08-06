#include <LaserUtils.h>                                                // the file that implements most of the behind-the-scenes code 
#include <LiquidCrystal_I2C.h>                                         // the file/library for the display screen
                                                                       //Pin assignments:
#define IR_RX 2                                                        // pin the IR receiver (left data pin) is connected to
#define IR_TX 13                                                       // pin the IR transmitter is connected to
#define BLASTER_TRIGGER 12                                             // pin the trigger is connected to
#define SPEAKER 11                                                     // pin the piezo speaker is connected to

#define MSG_LENGTH 16                                                  // length of transmitted and received messages (must be same on ALL blasters)
                                                                       //Variables used throughout the code:
int im_hit = false;                                                    // a true/false variable that records if you're being hit
unsigned long last_fired = 0;                                          // a number that records when the last shot was fired
bool shooting_now = false;                                             // a true/false variable that records if you're shooting
LaserRxTx laser = LaserRxTx(IR_RX, IR_TX);                             // an object that lets you fire and receive IR signals
LiquidCrystal_I2C lcd(0x27, 16, 2);                                    // Set the LCD address to 0x27 for a 16 chars and 2 line display


char* my_secret_message = "TYPE A MESSAGE HERE";                       // Note that the message cuts off at 16 characters long, including spaces


void setup() {                                                         //Function run once when Arduino is powered on
  Serial.begin(9600);                                                  // so we can receive messages / watch for errors
  lcd.begin();                                                         // initialize the LCD
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


char* messageCleaner(String uncleaned) {                                // clean up messages
  String cleaned = "";
  for (int i=0; i<MSG_LENGTH; i++) {
    if (i < uncleaned.length()) 
      cleaned += uncleaned.charAt(i);  // copy anything up to the max character limit (set by loop)
    else 
      cleaned += ' ';  // pad any short strings
  }
  char cleaned_formatted[MSG_LENGTH+1];
  cleaned.toCharArray(cleaned_formatted, MSG_LENGTH+1);
  return cleaned_formatted;
}


void shot(char* secret_msg) {                                          //Function called when you get shot (it gets sent the shooter's codename)
  if (secret_msg == "")                                                // if no message received, cancel the shot
    return;                                                             

  lcd.clear();                                                         // all LCD messages should first clear the screen
  lcd.print(secret_msg);                                               // prints a message to the LCD screen
    
  tone(SPEAKER, 392);                                                  // sound played when you get the message 
  delay(200);
  tone(SPEAKER, 261);
  delay(200);
  noTone(SPEAKER);           
}


void loop() {                                                          //Function run in a loop forever: checks if you're shooting and/or shot
 
  if (im_hit) {                                                        // when it checks here, if you were hit... 
    shot(laser.irRecv(MSG_LENGTH));                                    // starts the irRecv() function and return the sender's code to a function to do the shot logic
    im_hit = false;                                                    // reset the hit flag to false since you are no longer hit
  }
                                                                       
  if (digitalRead(BLASTER_TRIGGER) == LOW) {                           // next, check if you're trying to shoot
    shooting_now = true;                                               // tell the program you're shooting so it doesn't cause issues with your own receiver
    laser.fireLaser(messageCleaner(my_secret_message));                // fire off the cleaned up secret message (exactly 32 characters after cleaning)
    delay(1000);
    shooting_now = false;                                              // tell the program you're done shooting
  }
}
