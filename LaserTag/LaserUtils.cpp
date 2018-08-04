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
  for(int i=0; i<_player_count; i++) {  // search through existing players:
    if (player_name == players[i].username)
      return players[i];
  }

  if (_player_count >= _max_players) {
    Serial.println("Max player count reached, returning first added user");
    return players[0];
  }
  
  players[_player_count].username = player_name;  // add the new player
  _player_count++;  // increase your player count
  return players[_player_count-1];  // return a reference to the player you just created
}

/////////////////////////////////////////////////////////////////////////////////

#define MSG_LEN 4  // number of bytes in the data being transmitted
#define PULSE_TIME 300  // minimum length (microseconds) of a pulse of carrier frequency / delay in the transmission
#define ERROR_TIMEOUT 500  // milliseconds before a transmission times out

LaserRxTx::LaserRxTx(byte ir_rx, byte ir_tx) {
  _ir_rx = ir_rx;
  _ir_tx = ir_tx;
  pinMode(ir_tx, OUTPUT);  // transmission pin as an output
  pinMode(ir_rx, INPUT);  // receiver pin as an input
  
}

// sends a pulse of 38khz square waves for a while (delay1), then nothing for a while (delay2)
void LaserRxTx::sendPulse(int delay1, int delay2) {
  tone(_ir_tx, 38000);  // turns on the 38khz (38000hz) square wave on the transmit pin
  delayMicroseconds(delay1);  // waits for a moment
  noTone(_ir_tx);  // turns off the square wave
  delayMicroseconds(delay2);  // waits for a moment sending nothing
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
  for(int i = 7; i >= 0; i--) {  // loops from index 7,6,5,...1,0
    if(bitRead(character, i))  // checks if this bit (i) is a 1 or 0
      sendOne();
    else
      sendZero();
  }
}

// Breaks up aa String into characters for transmission
void LaserRxTx::sendMessage(String message) {
  for (int i=0; i<message.length(); i++) {  // loops once for each character in the String
    sendCharacter(message[i]);  // calls sendCharacter on one character at a time
  }
}

// Shoots the IR signal out
void LaserRxTx::fireLaser(char* message) {
  sendPulse(8000, 4000);  // initial carrier pulse to flag receiver
  sendMessage(message);  // data to be transmitted /* CHANGE */
  sendPulse(1000, 0);  // final carrier pulse to close
}

// reads the incoming transmission and stores the data received as a String
// *note that all data is sent and received left to right (bytes AND strings)
String LaserRxTx::irRecv() { 
  String message = "";  // the initially empty message to receive
  int timeout = millis();
  int time_high = 0;
  for (int i=0; i<2; i++) {  // filters out the junk at the start of transmission
    while (digitalRead(_ir_rx)) { 
      if(millis()-timeout>ERROR_TIMEOUT) { // in case stuck in loop
        Serial.println("Timed out on high signal");
        return "";  // return "" (empty)
      }
    }
    while (not digitalRead(_ir_rx)) { 
      if(millis()-timeout>ERROR_TIMEOUT) { // in case stuck in loop
        Serial.println("Timed out on high signal");
        return "";  // return "" (empty)
      }
    }
    time_high = micros();
  }
  for (int cycles = 0; cycles < MSG_LEN; cycles++) {  // loop through each byte
    char data_byte = 0;
    for (int i=0; i<8; i++) {  // loop to receive each bit
      while (digitalRead(_ir_rx)) {  // wait on the high signal until it goes low
        if(millis()-timeout>ERROR_TIMEOUT) break;  // in case stuck in loop
      }
      data_byte = data_byte << 1;  // shift the bits in the data_byte byte to prepare for the next loop through
      int time_difference = micros()-time_high;  // check how long it has been since the signal first went high until now 
      if (time_difference > 2*PULSE_TIME)  // if the time diff is more than two pulse times (expects three), it is a 1, if less than two pulse times (expects one), it is a 0
        data_byte++;  // no need to add zero, so it only adds 1 for a 1
      while (not digitalRead(_ir_rx)) {  // wait on the low signal until it goes high
        if(millis()-timeout>ERROR_TIMEOUT) break;  // in case stuck in loop
      }
      time_high = micros();  // record the time that the signal first went high
    }
    if ((data_byte < 32)||(data_byte > 126)) {
      Serial.println("Exiting on garbled character");
      return "";  // if there are any non-standard characters, assume a garbled message and return "" (empty)
    }
    message = message + data_byte;  // add the new character of data to the message
  }
  Serial.println(message);  // for debugging: print the received message
  return message; // return the received message text
}



