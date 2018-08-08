#ifndef _LaserUtils_h
#define _LaserUtils_h

#include <Arduino.h> 

class Player {  
  public:
    String username;
    int shot_count;
    int damage_inflicted;
    Player();
};

class Attack {  
  public:
    char* name;
    char shortcut;
    int damage;
    int shot_delay;
    Attack();
};

class Team {  
  public:
    char* name;
    Team();
};

class GameManager {
  public:
    GameManager();
	
    void addAttack(char* name_, int damage_, int shot_delay_);
	void addTeam(char* team);
	
	Player lookupPlayer(String player_name);
	char* listAttackName(int index);
	char* listTeamName(int index);
	
	int lookupShotDelay(char attack_shortcut);
	int lookupDamage(char attack_shortcut);
	int recordHit(char* attacker, int damage);
	void revive(int hp_);
	int lives_used;
	int hp;
	
  private:
    Player players[10];  // hardcode max of 10 players
	Attack attacks[10];  // hardcode max of 10 attacks
	Team teams[5];     // hardcode max 16 chars for team name
    int _player_count;
    int _attack_count;
	int _team_count;
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
    static void setMyName(char* name); 
    static void setMyParameters(char attack, char team); 
    static char* getMyShotMessage();
    static char* getMyName();
    static char getMyAttack();
    static char getMyTeam();
	static bool checkFriendlyFire(char other_team);
    static char* getName(char* message);
    static char getAttack(char* message);
    static char getTeam(char* message); 
	static bool checkSafe(char* message);
  private:
    static bool storedCheck();
};

#endif