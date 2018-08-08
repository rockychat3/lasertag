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
  pinMode(SPEAKER, OUTPUT);                                            // setup speaker pin as an output 
  pinMode(IR_TX, OUTPUT);                                              // setup IR sending pin as an output 
  pinMode(IR_RX, INPUT);                                               // setup IR receiving pin as an input 
  
  lcd.backlight();
  lcd.print("Welcome, ");                                              // looks up your username in EEPROM memory and greets you
  lcd.print(LaserMsg::getMyName());
  delay(2500);
  lcd.clear();
  lcd.noBacklight();

  game_manager.addAttack("Rapid", 1, 250);
  LaserMsg::setMyParameters('R', 'S');

  fire_rate = game_manager.lookupShotDelay(LaserMsg::getMyAttack());
  Serial.println("Fire rate: ");
  Serial.println(fire_rate);
  attachInterrupt(digitalPinToInterrupt(IR_RX),irInterrupt,FALLING);   // the function irInterrupt is called when an IR signal is being received
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
                                                                       // next, check if you're trying to shoot
  if ((digitalRead(BLASTER_TRIGGER) == LOW) &&                         // when it checks here, if the trigger is down...
      (millis()-last_fired > fire_rate)) {                             // ...AND if it has been FIRE_RATE milliseconds since the last shot...
    last_fired = millis();                                             // store the current time as the new "last_fired" time
    shooting_now = true;                                               // tell the program you're shooting so it doesn't cause issues with your own receiver
    laser.fireLaser(LaserMsg::getMyShotMessage());                     // fire off a message (the username)
    shooting_now = false;                                              // tell the program you're done shooting
  }
}



#define OCTAVE_OFFSET 0

// These values can also be found as constants in the Tone library (Tone.h)
int notes[] = { 0, 262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494,
  523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988,
  1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976,
  2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951 };

#define isdigit(n) (n >= '0' && n <= '9')

void play_rtttl(char *p, int tonePin) {
  // Absolutely no error checking in here
  Serial.print("Playing: ");
  Serial.println(p);
  byte default_dur = 4;
  byte default_oct = 6;
  int bpm = 63;
  int num;
  long wholenote;
  long duration;
  byte note;
  byte scale;

  // format: d=N,o=N,b=NNN:

  // get default duration
  if(*p == 'd') {
    p++; p++;              // skip "d="
    num = 0;
    while(isdigit(*p)) {
      num = (num * 10) + (*p++ - '0');
    }
    if(num > 0) 
      default_dur = num;
    p++;                   // skip comma
  }
  //Serial.print("ddur: "); Serial.println(default_dur, 10);

  // get default octave
  if(*p == 'o') {
    p++; p++;              // skip "o="
    num = *p++ - '0';
    if(num >= 3 && num <=7) 
      default_oct = num;
    p++;                   // skip comma
  }
  //Serial.print("doct: "); Serial.println(default_oct, 10);

  // get BPM
  if(*p == 'b') {
    p++; p++;              // skip "b="
    num = 0;
    while(isdigit(*p)) {
      num = (num * 10) + (*p++ - '0');
    }
    bpm = num;
    p++;                   // skip colon
  }
  //Serial.print("bpm: "); Serial.println(bpm, 10);
  // BPM usually expresses the number of quarter notes per minute
  wholenote = (60 * 1000L / bpm) * 4;  // this is the time for whole note (in milliseconds)

  // now begin note loop
  while(*p) {
    num = 0;
    while(isdigit(*p)) {  // first, get note duration, if available
      num = (num * 10) + (*p++ - '0');
    }
    if(num) 
      duration = wholenote / num;
    else 
      duration = wholenote / default_dur;  // we will need to check if we are a dotted note after

    switch(*p) { // now get the note
      case 'c':
        note = 1;
        break;
      case 'd':
        note = 3;
        break;
      case 'e':
        note = 5;
        break;
      case 'f':
        note = 6;
        break;
      case 'g':
        note = 8;
        break;
      case 'a':
        note = 10;
        break;
      case 'b':
        note = 12;
        break;
      case 'p':
      default:
        note = 0;
    }
    p++;

    if(*p == '#') {  // '#' sharp notes are higher pitched notes
      note++;
      p++;
    }

    if(*p == '.') {  // '.' dotted notes are 1.5 times the duration
      duration += duration/2;
      p++;
    }
    
    if(isdigit(*p)) {  // now, get scale
      scale = *p - '0';
      p++;
    } else {
      scale = default_oct;
    }
    scale += OCTAVE_OFFSET;

    if(*p == ',')
      p++;       // skip comma for next note (or we may be at the end)

    if(note) {  // now play the note
      //Serial.print("Playing: ");
      //Serial.print(scale, 10); Serial.print(' ');
      //Serial.print(note, 10); Serial.print(" (");
      //Serial.print(notes[(scale - 4) * 12 + note], 10);
      //Serial.print(") ");
      //Serial.println(duration, 10);
      tone(tonePin, notes[(scale - 4) * 12 + note]);
      delay(duration);
      noTone(tonePin);
    } else {  // or a silent pause
      //Serial.print("Pausing: ");
      //Serial.println(duration, 10);
      delay(duration);
    }
  }
}
