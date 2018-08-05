#include <LaserUtils.h>                                                // the file that implements most of the behind-the-scenes code 
#include <EEPROM.h>                                                    // used to read in attributes unique to you 
                                                                       //Pin assignments:
#define IR_RX 2                                                        // pin the IR receiver (left data pin) is connected to
#define IR_TX 13                                                       // pin the IR transmitter is connected to
#define BLASTER_TRIGGER 12                                             // pin the trigger is connected to
#define SPEAKER 11                                                     // pin the piezo speaker is connected to
                                                                       //Other constants:
#define IS_TEAM_GAME false                                             // true or false to state team or solo mode
#define USERNAME "BAP6"   // remove after eeprom update                // a name with MSG_LEN characters unique to you
#define MSG_LEN 4         // remove after eeprom update                // a name with MSG_LEN characters unique to you
#define MAX_PLAYERS 5                                                  // max number of new players that could hit you in a game (save memory if lower)
#define FIRE_RATE 250                                                  // milliseconds since the last time a shot started
#define DEAD_DELAY 3000                                                // milliseconds you stay dead for after being hit
                                                                       //Variables used throughout the code:
int im_hit = false;                                                    // a true/false variable that records if you're being hit
unsigned long last_fired = 0;                                          // a number that records when the last shot was fired
bool shooting_now = false;                                             // a true/false variable that records if you're shooting
char username[MSG_LEN];                                                        // a word variable that will store your codename
PlayerManager player_manager = PlayerManager(MAX_PLAYERS);             // an object that keeps track of the players in the game 
LaserRxTx laser = LaserRxTx(IR_RX, IR_TX);                             // an object that lets you fire and receive IR signals

void setup() {                                                         //Function run once when Arduino is powered on
  Serial.begin(9600);                                                  // so we can receive messages / watch for errors
  pinMode(BLASTER_TRIGGER, INPUT_PULLUP);                              // trigger as an input that defaults to high (5V)
  pinMode(SPEAKER, OUTPUT);                                            // setup speaker pin as an output 
  pinMode(IR_TX, OUTPUT);                                              // setup IR sending pin as an output 
  pinMode(IR_RX, INPUT);                                               // setup IR receiving pin as an input 
  attachInterrupt(digitalPinToInterrupt(IR_RX),irInterrupt,FALLING);   // the function irInterrupt is called when the trigger is pressed (that pin FALLS from high to low voltage)
  
  for (int i=0; i<MSG_LEN; i++) {  // move to test file
    EEPROM.put(i, USERNAME[i]);    // move to test file
  }                                // move to test file
  for (int i=0; i<MSG_LEN; i++) {  // move to Utils
    char name_character;           // move to Utils
    EEPROM.get(i, name_character); // move to Utils
    username[i] = name_character;  // move to Utils
  }                                // move to Utils
  Serial.print("Welcome, ");
  Serial.println(username);
}

void irInterrupt() {                                                   //Function called when the IR receiver sees a new message
  if (not shooting_now)                                                // ignore this interrupt if you're in the middle of sending (avoid self-interference)
    im_hit = true;                                                     // records that we have been hit, but doesn't take any action immediately, like leaving a note on the fridge
}

bool shot(String attacker_name) {                                      //Function called when you get shot (it gets sent the shooter's codename)
  if (attacker_name == "")                                             // if no message received, cancel the shot
    return false;                                                             
  if (attacker_name == String(username)) {                                     // no shooting yourself!
    Serial.println("Your own shot hit you");
    return false;                                                         
  }
  if (IS_TEAM_GAME) {                                                  // if you're playing a team game...
    if (username[0] == attacker_name[0]) {                             // ...no getting shot by your teammates
      Serial.print("friendly fire from: ");
      Serial.println(attacker_name);
      return false;                                                           
    }
  }
  
  Serial.print("Attacked by: ");
  Serial.println(attacker_name);

  Player attacker = player_manager.lookupPlayer(attacker_name);        // get the player with attributes to update
  attacker.shot_count += 1;                                            // tally one more shot for the attacker
  attacker.damage_inflicted += 1;                                      // tally one more damage point
  
  tone(SPEAKER, 392);                                                  // sound played when you're shot 
  delay(200);
  tone(SPEAKER, 261);
  delay(200);
  noTone(SPEAKER);
  
  //DISPLAY CODE                                                       // Display attacker name on screen
                                    
  delay(DEAD_DELAY-400);                                               // disable shooting for however long you are "dead" (subtract tone time above)
  
  return true;                                                         // back to the logic on what to do if shot
}

// The main loop of code always running
void loop() {                                                          //Function run in a loop forever: checks if you're shooting and/or shot
 
  if (im_hit) {                                                        // when it checks here, if you were hit... 
    shot(laser.irRecv());                                              // starts the irRecv() function and return the attacker's code to a function to do the shot logic
    im_hit = false;                                                    // reset the hit flag to false since you are no longer hit
  }
                                                                       // next, check if you're trying to shoot
  if ((digitalRead(BLASTER_TRIGGER) == LOW) &&                         // when it checks here, if the trigger is down...
      (millis()-last_fired > FIRE_RATE)) {                             // ...AND if it has been FIRE_RATE milliseconds since the last shot...
    last_fired = millis();                                             // store the current time as the new "last_fired" time
    shooting_now = true;                                               // tell the program you're shooting so it doesn't cause issues with your own receiver
    laser.fireLaser(username);                                         // fire off a message (the username)
    shooting_now = false;                                              // tell the program you're done shooting
  }
}
