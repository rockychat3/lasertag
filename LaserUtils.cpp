#include "LaserUtils.h"


Player::Player() {
  username = "";
  shot_count = 0;
  damage_inflicted = 0;
}


Attack::Attack() {
  name = "";
  shortcut = '\0';
  damage = 0;
  shot_delay = 0;
}


Team::Team() {
  name = "";
}


GameManager::GameManager() {
  _player_count = 0;
  _attack_count = 0;
  _team_count = 0;
  this->addTeam("Solo");     // hardcode the "Solo" team 
  
  hp = 1;
  lives_used = 0;
}


void GameManager::addAttack(char* name_, int damage_, int shot_delay_) {
  if (_attack_count >= 10) {
    Serial.println("Max attack types reached");
    return;
  }
  
  attacks[_attack_count].name = name_;                   
  attacks[_attack_count].shortcut = name_[0];                   
  attacks[_attack_count].damage = damage_;                   
  attacks[_attack_count].shot_delay = shot_delay_;                   
  _attack_count++;                                                 
  return;
}


void GameManager::addTeam(char* team) {
  if (_team_count >= 10) {
    Serial.println("Max team types reached");
    return;
  }
  
  teams[_team_count].name = team;                   
  _team_count++;                                                 
  
  return;
}


Player GameManager::lookupPlayer(String player_name) {  
  for(int i=0; i<_player_count; i++) {                             // search through existing players:
    if (player_name == players[i].username)  return players[i];
  }

  if (_player_count >= 10) {
    Serial.println("Max player count reached, returning first added user to avoid crash");
    return players[0];
  }
  
  players[_player_count].username = player_name;                   // add the new player
  _player_count++;                                                 // increase your player count
  return players[_player_count-1];                                 // return a reference to the player you just created
}


char* GameManager::listAttackName(int index) {
  return attacks[index % _attack_count].name;                      // mod to prevent overflow for indecisive people
}


char* GameManager::listTeamName(int index) {
  Serial.println(index);
  Serial.println(_team_count);
  return teams[index % _team_count].name;                           // mod to prevent overflow for indecisive people
}


int GameManager::lookupShotDelay(char attack_shortcut) {
  for (int i=0; i<_attack_count; i++) {
	Serial.println("index, attack_shortcut, attacks[i].shortcut, delay)");
	Serial.println(i);
	Serial.println(attack_shortcut);
	Serial.println(attacks[i].shortcut);
	Serial.println(attacks[i].shot_delay);
	
	if (attacks[i].shortcut == attack_shortcut)  return attacks[i].shot_delay;
  }
  return 0;
}


int GameManager::lookupDamage(char attack_shortcut) {
  for (int i=0; i<_attack_count; i++) {
	if (attacks[i].shortcut == attack_shortcut)  return attacks[i].damage;
  }
  return 0;
}


int GameManager::recordHit(char* attacker_name, int damage) {
  Player attacker = this->lookupPlayer(attacker_name);
  attacker.shot_count += 1;                                            // tally one more shot for the attacker
  attacker.damage_inflicted += damage;
  
  hp -= damage;
  return hp;
}


void GameManager::revive(int hp_) {
  hp = hp_;
  lives_used += 1;
}


void GameManager::dataDump(LiquidCrystal_I2C lcd) {
  for (int i=0; i<_player_count; i++) {
	lcd.clear();
    lcd.print(players[i].username);
	lcd.setCursor(0,1);
    lcd.print("Hits: ");
    lcd.print(players[i].shot_count);
    lcd.print(", HP: ");
    lcd.print(players[i].damage_inflicted);
  }
}


/////////////////////////////////////////////////////////////////////////////////

#define PULSE_TIME 500                                             // minimum length (microseconds) of a pulse of carrier frequency / delay in the transmission
#define ERROR_TIME 12000                                           // microseconds in a single loop before a transmission times out

LaserRxTx::LaserRxTx(byte ir_rx, byte ir_tx) {
  _ir_rx = ir_rx;
  _ir_tx = ir_tx;
  pinMode(ir_tx, OUTPUT);                                          // transmission pin as an output
  pinMode(ir_rx, INPUT);                                           // receiver pin as an input
  
}


// sends a pulse of 38khz square waves for a while (delay1), then nothing for a while (delay2)
void LaserRxTx::sendPulse(int delay1, int delay2) {
  for (int i=0; i<delay1/26; i++) {                                // generates the right number of cycles of a 38khz (38000hz) square wave on the transmit pin
    digitalWrite(_ir_tx, HIGH);
    delayMicroseconds(9);
    digitalWrite(_ir_tx, LOW);
    delayMicroseconds(9);
  }
  
  delayMicroseconds(delay2);                                       // waits for a moment sending nothing
}


// a zero looks like a short pulse of 38khz followed by an equal wait doing nothing
void LaserRxTx::sendZero(){
  sendPulse(PULSE_TIME, PULSE_TIME);
}


// a one looks like a short pulse of 38khz followed by a 3x longer wait doing nothing
void LaserRxTx::sendOne(){
  sendPulse(PULSE_TIME, 3*PULSE_TIME);
}


// reads through all 8 bits of a byte and calls for 0's and 1's to be transmitted
void LaserRxTx::sendCharacter(char character) {
  for(int i = 7; i >= 0; i--) {                                    // loops from index 7,6,5,...1,0
    if(bitRead(character, i))  sendOne();                          // checks if this bit (i) is a 1 or 0    
    else  sendZero();
  }
}


// Breaks up aa String into characters for transmission
void LaserRxTx::sendMessage(String message) {
  for (int i=0; i<message.length(); i++) {                         // loops once for each character in the String
    sendCharacter(message[i]);                                     // calls sendCharacter on one character at a time
  }
}

// Shoots the IR signal out
void LaserRxTx::fireLaser(char* message) {
  Serial.print("Firing ");
  Serial.println(message);
  sendPulse(8000, 4000);                                           // initial carrier pulse to flag receiver
  sendMessage(message);                                            // data to be transmitted /* CHANGE */
  sendPulse(1000, 0);                                              // final carrier pulse to close
}


char* LaserRxTx::laserRecv() {
  return this->irRecv(4);
}


// reads the incoming transmission and stores the data received as a String
// *note that all data is sent and received left to right (bytes AND strings)
char* LaserRxTx::irRecv(int msg_len) { 
  char message[msg_len+1];                                          // the initially empty message to receive
  unsigned long time_started, time_high;

  for (int i=0; i<2; i++) {                                        // filters out the junk at the start of transmission
    time_started = micros();
    while (digitalRead(_ir_rx)) { 
      if(micros()-time_started>ERROR_TIME) {                       // in case stuck in loop
        Serial.println("IR pre-data timed out on high signal");
        return "";                                                 // return "" (empty)
      }
    }
    
    time_started = micros();
    while (not digitalRead(_ir_rx)) { 
      if(micros()-time_started>ERROR_TIME) {                       // in case stuck in loop
        Serial.println("IR pre-data timed out on low signal");
        return "";                                                 // escape with empty 
      }
    }
  }
  
  for (int cycles = 0; cycles < msg_len; cycles++) {               // loop through each byte
                       
    char data_byte = 0;
    
    for (int i=0; i<8; i++) {                                      // loop to receive each bit
      
      time_started = micros();                                     // record the time that the signal first went high
      while (digitalRead(_ir_rx)) {                                // wait on the high signal until it goes low
        if(micros()-time_started>ERROR_TIME) {                     // in case stuck in loop
          Serial.println("IR bit read timed out on high signal");
          return "";                                               // escape with empty 
        }
      }
      
      data_byte = data_byte << 1;                                  // shift the bits in the data_byte byte to prepare for the next loop through
      unsigned long time_difference = micros()-time_started;       // check how long it has been since the signal first went high until now 
      if (time_difference > 2*PULSE_TIME)                          // if the time diff is more than two pulse times (expects three), it is a 1, if less than two pulse times (expects one), it is a 0
        data_byte++;                                               // no need to add zero, so it only adds 1 for a 1
      
      time_started = micros();                                     // record the time that the signal first went high
      while (not digitalRead(_ir_rx)) {                            // wait on the low signal until it goes high
        if(micros()-time_started>ERROR_TIME) {                     // in case stuck in loop
          Serial.println("IR bit read timed out on low signal");
          return "";                                               // escape with empty 
        }
      }
    }
    
    message[cycles] = data_byte;                                   // add the new character of data to the message
  }
  message[msg_len] = '\0';                                         // special terminating character for all char arrays
  //Serial.print("Message received: ");
  //Serial.println(message);                                         // for debugging: print the received message
  return message;                                                  // return the received message text
}


/////////////////////////////////////////////////////////////////////////////////


#include <EEPROM.h>                                                // used to set/read variables in special EEPROM memory

// store the message fired out by the IR beam in-game
static void LaserMsg::setMyName(char* name) {
  EEPROM.put(0, 42);
  EEPROM.put(1, name[0]);
  EEPROM.put(2, name[1]);
}

// store the message fired out by the IR beam in-game
static void LaserMsg::setMyParameters(char attack, char team) {
  EEPROM.put(3, attack);                                           
  EEPROM.put(4, team);                                             
}


// Internal check that EEPROM was set
static bool LaserMsg::storedCheck() {
  byte check_byte;           
  EEPROM.get(0, check_byte);
  if (check_byte == 42)  return true;                              // the check byte 42 is an arbitary way to see if we set the memory or not
  else  return false;
}


// Finds your Arduino's last-set full message in EEPROM
static char* LaserMsg::getMyShotMessage() {
  char* message = "00AS";
  if (!LaserMsg::storedCheck())  return message;
  
  message[0] = EEPROM.read(1); 
  message[1] = EEPROM.read(2); 
  message[2] = EEPROM.read(3); 
  message[3] = EEPROM.read(4); 
  return message;
}


// Finds your Arduino's last-set username in EEPROM
static char* LaserMsg::getMyName() {
  if (!LaserMsg::storedCheck())  return "00";

  char* username = "00";
  username[0] = EEPROM.read(1); 
  username[1] = EEPROM.read(2); 
  return username;
}


// Finds your Arduino's last-set attack power in EEPROM
static char LaserMsg::getMyAttack() {
  if (!LaserMsg::storedCheck())  return 'A';

  char attack;
  EEPROM.get(3, attack); 
  return attack;
}


// Finds your Arduino's last-set team in EEPROM
static char LaserMsg::getMyTeam() {
  if (!LaserMsg::storedCheck()) 
    return 'S';

  char team;
  EEPROM.get(4, team); 
  return team;
}


// Compares your last-set team to the incoming value to check for friendly fire Finds your Arduino's last-set team in EEPROM
static bool LaserMsg::checkFriendlyFire(char other_team) {
  if (!LaserMsg::storedCheck()) 
    return false;

  char team;
  EEPROM.get(4, team); 
  if (team == 'S')  return false;
  if (team == other_team)  return true;
  return false;
}


// Parses out the name from a message
static char* LaserMsg::getName(char* message) {
  char* username = "00";
  username[0] = message[0];
  username[1] = message[1];
  return username;
}


// Parses out the attack power from a message
static char LaserMsg::getAttack(char* message) {
  return message[2];
}


// Parses out the team from a message
static char LaserMsg::getTeam(char* message) {
  return message[3];
}

// Checks if the message is garbled in any way 
static bool LaserMsg::checkSafe(char* message) {
  for (int i=0; i<4; i++) {
	if ((message[i] < '0') || (message[i] > 'z'))  return false;
  }
  return true;
}
