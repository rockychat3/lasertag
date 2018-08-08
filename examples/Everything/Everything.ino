#include <LaserUtils.h>                                                // the file that implements most of the behind-the-scenes code 
#include <LiquidCrystal_I2C.h>                                         // the file/library for the display screen
#define SPEAKER 11                                                     // pin the piezo speaker is connected to
LiquidCrystal_I2C lcd(0x27, 16, 2);                                    // Set the LCD address to 0x27 for a 16 chars and 2 line display
GameManager game_manager = GameManager();                              // an object that keeps track of the players in the game

#define DEAD_DELAY 3000                                                // milliseconds you stay dead for after losing all health


void setup() {                                                         //Function run once when Arduino is powered on
  game_manager.addAttack("Rapid", 1, 250);
  game_manager.addAttack("Cannon", 10, 5000);
  game_manager.addTeam("Red");
  game_manager.addTeam("Blue");
  game_manager.revive(25);
  //LaserMsg::setMyName("P1"); // COMMENT OUT after first setup with YOUR initials
  gameSetup(game_manager, lcd);
}

void shot(char* username, char attack, char team) {                     //Function called when you get shot (it gets sent the shooter's codename)
  if (LaserMsg::checkFriendlyFire(team)) {                              // check if you got shot by a teammate
    lcd.setCursor(0,0);
    lcd.print("Friendly Fire   ");
    delay(500);
  } else {                                                               // if it was NOT a teammate... 
    lcd.clear();
    lcd.print("Last hit by:  ");
    lcd.print(username);                                                 
    lcd.setCursor(0,1);                                                  // moves to the 2nd line of the display
    int damage = game_manager.lookupDamage(attack);
    int hp = game_manager.recordHit(username, damage);                   // record the hit to track hp and player stats
    
    if (hp > 0) {
      lcd.print(hp);
      lcd.print(" HP remaining");
      delay(500);
    } else {                                                             // this means you died
      // dead to-dos
      lcd.print("You is very dead");
      tone(SPEAKER, 392);                                                // alive sound played when you're out of hp
      delay(1000);
      tone(SPEAKER, 261);
      delay(1000);
      noTone(SPEAKER);
      delay(8000);
      
      // back alive to-dos
      game_manager.revive(25);
      lcd.clear();
      lcd.setCursor(0,1); 
      lcd.print("FULL HEALTH");
      tone(SPEAKER, 261);                                                // alive sound played when you're out of hp
      delay(100);
      tone(SPEAKER, 392);
      delay(100);
      noTone(SPEAKER);  
    }
  }
}

