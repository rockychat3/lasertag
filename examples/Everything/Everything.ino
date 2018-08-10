#define HP_VALUE 50
#define AMMO_VALUE 100
#include <LaserUtils.h>                                                // the file that implements most of the behind-the-scenes code 
#include <LiquidCrystal_I2C.h>                                         // the file/library for the display screen
#define SPEAKER 11                                                     // pin the piezo speaker is connected to
LiquidCrystal_I2C lcd(0x27, 16, 2);                                    // Set the LCD address to 0x27 for a 16 chars and 2 line display
GameManager game_manager = GameManager();                              // an object that keeps track of the players in the game

int cycle = 0;
int last_fired_changer = 0;
int ammo = AMMO_VALUE;

void setup() {                                                         //Function run once when Arduino is powered on
  game_manager.addAttack("Firestorm", 10, 4000);
  game_manager.addAttack("Magic Missile", 5, 1500);
  game_manager.addAttack("Cure Wounds", -5, 2500);
  game_manager.addAttack("Holy Flame", 3, 1000);
  game_manager.addAttack("Warhammer", 15, 7500);
  game_manager.addAttack("Sword Slash", 2, 500);
  game_manager.addTeam("Rohan");
  game_manager.addTeam("Gondor");
  game_manager.addTeam("Ammo");
  game_manager.revive(HP_VALUE);
  //LaserMsg::setMyName("AA");                                          // COMMENT OUT after first setup with YOUR initials
  gameSetup(game_manager, lcd);
}

void shot(char* username, char attack, char team) {                     //Function called when you get shot (it gets sent the shooter's codename)
  if (team=='A') {
    //reload ammo
    ammo = AMMO_VALUE;
    lcd.clear();
    lcd.print("Reloaded!");
    return;
  } else {
  
    if (LaserMsg::checkFriendlyFire(team)) {                              // check if you got shot by a teammate
      lcd.setCursor(0,0);
      lcd.print("Friendly Fire   ");
  
      int damage = game_manager.lookupDamage(attack);
  
      if(damage < 0){
        lcd.clear();
        lcd.print("Healing");
        int hp = game_manager.recordHit(username, damage);
        lcd.print(hp);
        lcd.setCursor(0,1);
        lcd.print(" HP remaining");
        delay(1000);
        lcd.clear();
      }
      delay(500);
    } else {                                                               // if it was NOT a teammate... 
      lcd.clear();
      lcd.print("Last hit by:  ");
      lcd.print(username);                                                 
      lcd.setCursor(0,1);                                                  // moves to the 2nd line of the display
      int damage = game_manager.lookupDamage(attack);
      int hp = game_manager.recordHit(username, damage);                   // record the hit to track hp and player stats
      tone(SPEAKER, 440);                                                // alive sound played when you're out of hp
      delay(75);
      tone(SPEAKER, 220);
      delay(75);
      noTone(SPEAKER);
  
      
      if (hp > 0) {
        lcd.print(hp);
        lcd.print(" HP remaining");
        delay(500);
      } else {                                                             // this means you died
        // dead to-dos
        lcd.print("You Died");
        tone(SPEAKER, 392);                                                // alive sound played when you're out of hp
        delay(1000);
        tone(SPEAKER, 261);
        delay(1000);
        noTone(SPEAKER);
        delay(8000);
        
        // back alive to-dos
        game_manager.revive(HP_VALUE);
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
}





























#define IR_RX 2                                                        // pin the IR receiver (left data pin) is connected to
#define IR_TX 13                                                       // pin the IR transmitter is connected to
#define BLASTER_TRIGGER 12                                             // pin the trigger is connected to

int im_hit = false;                                                    // a true/false variable that records if you're being hit
unsigned long last_fired = 0;                                          // a number that records when the last shot was fired
bool shooting_now = false;                                             // a true/false variable that records if you're shooting
LaserRxTx laser = LaserRxTx(IR_RX, IR_TX);                             // an object that lets you fire and receive IR signals
int fire_rate = 2000;

void gameSetup(GameManager game_manager, LiquidCrystal_I2C lcd) {                                                     
  Serial.begin(9600);                                                  // so we can receive messages / watch for errors
  lcd.begin();                                                         // initialize the LCD
  pinMode(BLASTER_TRIGGER, INPUT_PULLUP);                              // trigger as an input that defaults to high (5V)
  pinMode(3, INPUT_PULLUP);                              // trigger as an input that defaults to high (5V)
  pinMode(SPEAKER, OUTPUT);                                            // setup speaker pin as an output 
  pinMode(IR_TX, OUTPUT);                                              // setup IR sending pin as an output 
  pinMode(IR_RX, INPUT);                                               // setup IR receiving pin as an input 
  
  //lcd.backlight();
  lcd.print("Welcome, ");                                              // looks up your username in EEPROM memory and greets you
  lcd.print(LaserMsg::getMyName());
  unsigned long timeval = millis();
  while (millis()-timeval < 2000) {
    if (digitalRead(BLASTER_TRIGGER) == LOW) {
      lcd.clear();
      gameMenu(game_manager, lcd);
    }
    delay(1);
  }
  lcd.clear();
  //lcd.noBacklight();

  fire_rate = game_manager.lookupShotDelay(LaserMsg::getMyAttack());
  Serial.println("Fire rate: ");
  Serial.println(fire_rate);
  attachInterrupt(digitalPinToInterrupt(IR_RX),irInterrupt,FALLING);   // the function irInterrupt is called when an IR signal is being received
  
}


void gameMenu(GameManager game_manager, LiquidCrystal_I2C lcd) {
  lcd.clear();
  while (digitalRead(BLASTER_TRIGGER) == LOW) { delay(1); }
  
  char team = NULL;
  int cycle = 0;
  while (!team) {
    team = itemSelect(game_manager, lcd, 'T', cycle); 
    cycle++;
  }

  lcd.clear();
  while (digitalRead(BLASTER_TRIGGER) == LOW) {}
  char attack = NULL;
  cycle = 0;
  while (!attack) {
    attack = itemSelect(game_manager, lcd, 'A', cycle); 
    cycle++;
  }

  LaserMsg::setMyParameters(attack, team);
}


char itemSelect(GameManager game_manager, LiquidCrystal_I2C lcd, char item_type, int index) {
  lcd.clear();
  char* item = "";
  if (item_type=='T') {
    lcd.print("Select team:");
    item = game_manager.listTeamName(index);
  } else {
    lcd.print("Select attack:");
    item = game_manager.listAttackName(index);
  }
  lcd.setCursor(0,1);
  lcd.print(item);
  while (digitalRead(BLASTER_TRIGGER) == HIGH) { delay(1); }                      // waits for user to press trigger
  unsigned long timeval = millis();
  while (digitalRead(BLASTER_TRIGGER) == LOW) {
    if (millis()-timeval > 400) {
      return item[0];                        // once pressed, if longer than 400ms, selects the item (returns first character)
    }
    delay(1);
  }
  return NULL;
}


void irInterrupt() {                                                   //Function called when the IR receiver sees a new message
  if (not shooting_now)                                                // ignore this interrupt if you're in the middle of sending (avoid self-interference)
    im_hit = true;                                                     // records that we have been hit, but doesn't take any action immediately, like leaving a note on the fridge
}

// The main loop of code always running
void loop() {                                                          //Function run in a loop forever: checks if you're shooting and/or shot

  if (im_hit) {                                                        // when it checks here, if you were hit... 
    char* code = laser.laserRecv();                                    // starts the irRecv() function and return the sender's code to a function to do the shot logic
    if (code != "") {                                                  // if no message received, cancel the shot, otherwise complete
      if (LaserMsg::checkSafe(code)) {
        shot(LaserMsg::getName(code), LaserMsg::getAttack(code), LaserMsg::getTeam(code));
      } 
    }
    im_hit = false;                                                    // reset the hit flag to false since you are no longer hit
  }

  if ((digitalRead(3) == LOW) && (millis()-last_fired_changer > 2000)) {
    last_fired_changer = millis();
    lcd.clear();
    char* item = game_manager.listAttackName(cycle);
    LaserMsg::setMyParameters(item[0], LaserMsg::getMyTeam());
    fire_rate = game_manager.lookupShotDelay(item[0]);
    cycle++;
    lcd.print(item);
  }
                                                                       // next, check if you're trying to shoot
  if ((digitalRead(BLASTER_TRIGGER) == LOW) &&                         // when it checks here, if the trigger is down...
      (millis()-last_fired > fire_rate)) {                             // ...AND if it has been FIRE_RATE milliseconds since the last shot...
    if (ammo>0) {
      last_fired = millis();                                             // store the current time as the new "last_fired" time
      shooting_now = true;                                               // tell the program you're shooting so it doesn't cause issues with your own receiver
      laser.fireLaser(LaserMsg::getMyShotMessage());                     // fire off a message (the username)
      shooting_now = false;                                              // tell the program you're done shooting
      ammo--;
    } else {
      lcd.clear();
      lcd.print("Out of ammo!");
      tone(SPEAKER, 880);                                                // alive sound played when you're out of hp
      delay(75);
      tone(SPEAKER, 440);
      delay(75);
      noTone(SPEAKER);
    }
  }
}