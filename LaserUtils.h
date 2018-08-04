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
    String irRecv();
  private:
    byte _ir_rx;
    byte _ir_tx;
    void sendPulse(int delay1, int delay2);
    void sendZero();
    void LaserRxTx::sendOne();
    void sendCharacter(char character);
    void sendMessage(String message);
};

