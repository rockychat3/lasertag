#include <LaserUtils.h>                                                // the file that implements most of the behind-the-scenes code 
#include <LiquidCrystal_I2C.h>                                         // the file/library for the display screen
                                                                       //Pin assignments:
#define IR_RX 2                                                        // pin the IR receiver (left data pin) is connected to
#define IR_TX 13                                                       // pin the IR transmitter is connected to
#define BLASTER_TRIGGER 12                                             // pin the trigger is connected to
#define SPEAKER 11                                                     // pin the piezo speaker is connected to
                                                                       //Other constants:
#define MAX_PLAYERS 5                                                  // max number of new players that could hit you in a game (save memory if lower)
#define FIRE_RATE 250                                                  // milliseconds since the last time a shot started
#define DEAD_DELAY 3000                                                // milliseconds you stay dead for after being hit
                                                                       //Variables used throughout the code:
int im_hit = false;                                                    // a true/false variable that records if you're being hit
unsigned long last_fired = 0;                                          // a number that records when the last shot was fired
bool shooting_now = false;                                             // a true/false variable that records if you're shooting
PlayerManager player_manager = PlayerManager(MAX_PLAYERS);             // an object that keeps track of the players in the game 
LaserRxTx laser = LaserRxTx(IR_RX, IR_TX);                             // an object that lets you fire and receive IR signals
LiquidCrystal_I2C lcd(0x27, 16, 2);                                    // Set the LCD address to 0x27 for a 16 chars and 2 line display

void setup() {                                                         //Function run once when Arduino is powered on
  Serial.begin(9600);                                                  // so we can receive messages / watch for errors
  lcd.begin();                                                         // initialize the LCD
  pinMode(BLASTER_TRIGGER, INPUT_PULLUP);                              // trigger as an input that defaults to high (5V)
  pinMode(SPEAKER, OUTPUT);                                            // setup speaker pin as an output 
  pinMode(IR_TX, OUTPUT);                                              // setup IR sending pin as an output 
  pinMode(IR_RX, INPUT);                                               // setup IR receiving pin as an input 
  attachInterrupt(digitalPinToInterrupt(IR_RX),irInterrupt,FALLING);   // the function irInterrupt is called when an IR signal is being received

  LaserMsg::setMyShotMessage("AP"); ///////TESTING ONLY
  lcd.print("Welcome,        ");                                       // looks up your username in EEPROM memory and greets you
  lcd.print(LaserMsg::getMyName());
}

void irInterrupt() {                                                   //Function called when the IR receiver sees a new message
  if (not shooting_now)                                                // ignore this interrupt if you're in the middle of sending (avoid self-interference)
    im_hit = true;                                                     // records that we have been hit, but doesn't take any action immediately, like leaving a note on the fridge
}

void shot(char* attack_msg) {                                          //Function called when you get shot (it gets sent the shooter's codename)
  if (attack_msg == "")                                                // if no message received, cancel the shot
    return;                                                             

  if (LaserMsg::getMyTeam()) {                                         // if you're playing a team game (team is not 0)
    if (LaserMsg::getMyTeam() == LaserMsg::getTeam(attack_msg)) {      // ...no getting shot by your teammates
      Serial.print("friendly fire from: ");
      Serial.println(LaserMsg::getName(attack_msg));
      return;                                                           
    }
  }
  
  lcd.clear();                                                         // all LCD messages should first clear the screen and turn on the backlight
  lcd.backlight();
  lcd.print("Attacked by: ");
  lcd.print(LaserMsg::getName(attack_msg));                            // parses apart the attack message to find the name of the attacker

  Player attacker = 
    player_manager.lookupPlayer(LaserMsg::getName(attack_msg));        // get the player with attributes to update
  attacker.shot_count += 1;                                            // tally one more shot for the attacker
  attacker.damage_inflicted += 1;                                      // tally one more damage point
  
  tone(SPEAKER, 392);                                                  // "dead" sound played when you're shot 
  delay(200);
  tone(SPEAKER, 261);
  delay(200);
  noTone(SPEAKER);
                              
  delay(DEAD_DELAY-400);                                               // disable shooting for however long you are "dead" (subtract tone time above)
  
  lcd.clear();                                                         // clear the screen and back to action!
  lcd.noBacklight();
}

// The main loop of code always running
void loop() {                                                          //Function run in a loop forever: checks if you're shooting and/or shot
 
  if (im_hit) {                                                        // when it checks here, if you were hit... 
    shot(laser.laserRecv());                                           // starts the irRecv() function and return the attacker's code to a function to do the shot logic
    im_hit = false;                                                    // reset the hit flag to false since you are no longer hit
  }
                                                                       // next, check if you're trying to shoot
  if ((digitalRead(BLASTER_TRIGGER) == LOW) &&                         // when it checks here, if the trigger is down...
      (millis()-last_fired > FIRE_RATE)) {                             // ...AND if it has been FIRE_RATE milliseconds since the last shot...
    last_fired = millis();                                             // store the current time as the new "last_fired" time
    shooting_now = true;                                               // tell the program you're shooting so it doesn't cause issues with your own receiver
    laser.fireLaser(LaserMsg::getMyShotMessage());                     // fire off a message (the username)
    shooting_now = false;                                              // tell the program you're done shooting
  }
}
