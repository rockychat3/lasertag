#include <LaserUtils.h>                                                // the file that implements most of the behind-the-scenes code 
#include <LiquidCrystal_I2C.h>                                         // the file/library for the display screen
#define SPEAKER 11                                                     // pin the piezo speaker is connected to
LiquidCrystal_I2C lcd(0x27, 16, 2);                                    // Set the LCD address to 0x27 for a 16 chars and 2 line display
GameManager game_manager = GameManager();                              // an object that keeps track of the players in the game


void setup() {                                                         //Function run once when Arduino is powered on
  //LaserMsg::setMyName("P1"); // COMMENT OUT after first setup with YOUR initials
  gameSetup(game_manager, lcd);
}

void shot(char* username, char attack, char team) {                     //Function called when you get shot (it gets sent the shooter's codename)
  lcd.clear();
  lcd.print("Last hit by:  ");
  lcd.print(username);                                                 
  lcd.setCursor(0,1);                                                  // moves to the 2nd line of the display
  game_manager.recordHit(username, 1);                   // record the hit to track hp and player stats
    
  // dead to-dos
  lcd.print("You is very dead");
  tone(SPEAKER, 392);                                                // alive sound played when you're out of hp
  delay(1000);
  tone(SPEAKER, 261);
  delay(1000);
  noTone(SPEAKER);
  delay(2000);
 
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

