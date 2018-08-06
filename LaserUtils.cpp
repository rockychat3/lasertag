#include "LaserUtils.h"


Player::Player() {
  username = "";
  shot_count = 0;
  damage_inflicted = 0;
}


PlayerManager::PlayerManager(int max_players) {
  players = new Player[max_players];
  _player_count = 0;
  _max_players = max_players;
}


Player PlayerManager::lookupPlayer(String player_name) {  
  for(int i=0; i<_player_count; i++) {                             // search through existing players:
    if (player_name == players[i].username)
      return players[i];
  }

  if (_player_count >= _max_players) {
    Serial.println("Max player count reached, returning first added user");
    return players[0];
  }
  
  players[_player_count].username = player_name;                   // add the new player
  _player_count++;                                                 // increase your player count
  return players[_player_count-1];                                 // return a reference to the player you just created
}


/////////////////////////////////////////////////////////////////////////////////


#define MSG_LEN 4                                                  // number of bytes in the data being transmitted
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
  //tone(_ir_tx, 38000);                                           // turns on the 38khz (38000hz) square wave on the transmit pin
  //delayMicroseconds(delay1/2);                                   // waits for a moment, half as long as it should (AND NOBODY KNOWS WHY...quirky)
  //noTone(_ir_tx);                                                // turns off the square wave
  
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
    if(bitRead(character, i))                                      // checks if this bit (i) is a 1 or 0
      sendOne();
    else
      sendZero();
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
  Serial.println("Firing");
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
  char message[msg_len];                                          // the initially empty message to receive
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
    
    message[cycles] = data_byte;                                 // add the new character of data to the message
  }
  
  Serial.print("Message received: ");
  Serial.println(message);                                         // for debugging: print the received message
  return message;                                                  // return the received message text
}


/////////////////////////////////////////////////////////////////////////////////


#include <EEPROM.h>                                                // used to set/read variables in special EEPROM memory

// store the message fired out by the IR beam in-game
static void LaserMsg::setMyShotMessage(char* name, char attack=5, char team=0) {
  if ((attack < 1) || (attack > 100))
	attack = 5;
  EEPROM.put(0, 42);
  EEPROM.put(1, name[0]);
  EEPROM.put(2, name[1]);
  EEPROM.put(3, attack);                                           // note that this is stored as a NUMBER of type char (0 to 255), defaults to 5
  EEPROM.put(4, team);                                             // team defaults to 0
}


// Internal check that EEPROM was set
static bool LaserMsg::storedCheck() {
  byte check_byte;           
  EEPROM.get(0, check_byte);
  if (check_byte == 42)                                            // the check byte 42 is an arbitary way to see if we set the memory or not
	return true;
  else
	return false;
}


// Finds your Arduino's last-set full message in EEPROM
static char* LaserMsg::getMyShotMessage() {
  if (!LaserMsg::storedCheck()) {
    char default_value[4] = {'0','0',5,0};
    return default_value;
  }

  char message[4];
  for (int i=1; i<=4; i++) {  
    char name_character;           
    EEPROM.get(i, name_character); 
    message[i] = name_character;  
  }
  return message;
}


// Finds your Arduino's last-set username in EEPROM
static char* LaserMsg::getMyName() {
  if (!LaserMsg::storedCheck()) 
    return "00";

  char username[2];
  EEPROM.get(1, username[0]); 
  EEPROM.get(2, username[1]); 
  return username;
}


// Finds your Arduino's last-set attack power in EEPROM
static char LaserMsg::getMyAttack() {
  if (!LaserMsg::storedCheck()) 
    return 5;

  char attack;
  EEPROM.get(3, attack); 
  return attack;
}


// Finds your Arduino's last-set team in EEPROM
static char LaserMsg::getMyTeam() {
  if (!LaserMsg::storedCheck()) 
    return 0;

  char team;
  EEPROM.get(4, team); 
  return team;
}


// Parses out the name from a message
static char* LaserMsg::getName(char* message) {
  char username[2];
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
