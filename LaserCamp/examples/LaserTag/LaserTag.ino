#include <LaserUtils.h>

// Pin assignments
#define IR_RX 2                                                            // pin the IR receiver (left data pin) is connected to
#define IR_TX 13                                                           // pin the IR transmitter is connected to
#define BLASTER_TRIGGER 12                                                 // pin the trigger is connected to
#define SPEAKER 11														   // pin the piezo speaker is connected to

// Other constants
#define IS_TEAM_GAME false                                                 // true or false to state team or solo mode
#define USERNAME "BAP6"                                                    // a name with MSG_LEN characters unique to you /* CHANGE */
#define MAX_PLAYERS 5                                                      // max number of new players that could hit you in a game (save memory if lower)
#define FIRE_RATE 400                                                      // milliseconds since the last time a shot started
#define DEAD_DELAY 2000                                                    // milliseconds you stay dead for after being hit

int im_hit = false;                                                        // a true/false variable that records if you're being hit
unsigned long last_fired = 0;                                              // a number that records when the last shot was fired
PlayerManager player_manager = PlayerManager(MAX_PLAYERS);  
LaserRxTx laser = LaserRxTx(IR_RX, IR_TX);

void setup() {
  Serial.begin(9600);                                                     // so we can receive messages / watch for errors
  pinMode(BLASTER_TRIGGER, INPUT_PULLUP);                                 // trigger as an input that defaults to high (5V)
  pinMode(SPEAKER, OUTPUT);												  // setup speaker pin as an output 
  attachInterrupt(digitalPinToInterrupt(IR_RX),irInterrupt,FALLING);      // the function irInterrupt is called when the trigger is pressed (that pin FALLS from high to low voltage)
}

void irInterrupt() {                                                      // the function called when the IR receiver sees a new message
  im_hit = true;                                                          // tells the main loop that we have been hit, but doesn't bother it immediately, like leaving a note on the fridge
}

bool shot(String attacker_name) { 
  if (!attacker_name)                                                     // if no message received, cancel the shot
    return false;                                                             
  if (attacker_name == USERNAME) {                                        // no shooting yourself!
    Serial.println("Your own shot hit you");
    return false;                                                             
  }
  if (IS_TEAM_GAME) {
    if (USERNAME[0] == attacker_name[0]) {                                // no getting shot by your team
      Serial.print("friendly fire from: ");
      Serial.println(attacker_name);
      return false;                                                           
    }
  }
  Player attacker = player_manager.lookupPlayer(attacker_name);           // get the player with attributes to update
  attacker.shot_count += 1;                                               // tally one more shot for the attacker
  attacker.damage_inflicted += 1;                                         // tally one more damage point
  
  tone(SPEAKER, 440);													  // sound played when you're shot 
  delay(200);
  noTone(SPEAKER);
                                                                          // Display attacker name on screen
																		
  delay(DEAD_DELAY);                                                 	  // disable shooting for however long you are "dead"
  
  return true;                                                            // back to the logic on what to do if shot
}

// The main loop of code always running
void loop() {                                                             // in each loop, first check if you got shot:
 
  if (im_hit) {                                                           // when it checks here, if you were hit... 
    shot(laser.irRecv());                           					  // starts the irRecv() function and return the attacker's code to a function to do the shot logic
    im_hit = false;                                                       // reset the hit flag to false since you are no longer hit
  }
                                                                          // next, check if you're trying to shoot
  if ((digitalRead(BLASTER_TRIGGER) == LOW) &&                            // when it checks here, if the trigger is down...
      (millis()-last_fired > FIRE_RATE)) {                                // ...AND if it has been FIRE_RATE milliseconds since the last shot...
    last_fired = millis();                                                // store the current time as the new "last_fired" time
    laser.fireLaser(USERNAME);                                            // fire off a message (the username)
  }
}

