#include <Arduino.h> 

class Player {  
  public:
    String username;
    int shot_count;
    int damage_inflicted;
    Player();
};
class PlayerManager {
  public:
    PlayerManager(int max_players);
    Player lookupPlayer(String player_name);
  private:
    Player *players;
    int _player_count;
    int _max_players;
};

class LaserRxTx {
  public:
    LaserRxTx(byte ir_rx, byte ir_tx);
    void fireLaser(char* message);
    char* laserRecv();
    char* irRecv(int msg_len);
  private:
    byte _ir_rx;
    byte _ir_tx;
    void sendPulse(int delay1, int delay2);
    void sendZero();
    void LaserRxTx::sendOne();
    void sendCharacter(char character);
    void sendMessage(String message);
};

class LaserMsg {
  public:
    static void setMyShotMessage(char* name, char attack=5, char team=0);  // note: attack is a NUMBER 0-255
    static char* getMyShotMessage();
    static char* getMyName();
    static char getMyAttack();
    static char getMyTeam();
    static char* getName(char* message);
    static char getAttack(char* message);
    static char getTeam(char* message); 
  private:
    static bool storedCheck();
};