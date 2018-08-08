#define SPEAKER 11                                                     // pin the piezo speaker is connected to
#include <LiquidCrystal_I2C.h>                                         // the file/library for the display screen
LiquidCrystal_I2C lcd(0x27, 16, 2);                                    // Set the LCD address to 0x27 for a 16 chars and 2 line display


char* my_secret_message = "TYPE A MESSAGE HERE";                       // Note that the message cuts off at 16 characters long, including spaces


void shot(char* secret_msg) {                                          //Function called when you get shot (it gets sent the shooter's codename)
  lcd.backlight();
  lcd.print(secret_msg);                                               // prints a message to the LCD screen
    
  tone(SPEAKER, 392);                                                  // sound played when you get the message 
  delay(200);
  tone(SPEAKER, 261);
  delay(200);
  noTone(SPEAKER); 

  delay(4000);                                                         // disable shooting for however long you are "dead" (subtract tone time above)
  
  lcd.clear();                                                         // clear the screen and back to action!
  lcd.noBacklight();  
}


void loop() {                                                          //Function run in a loop forever: checks if you're shooting and/or shot
  mainLoop(my_secret_message);
}
