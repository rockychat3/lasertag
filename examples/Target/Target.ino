#include <LaserUtils.h>                                                // the file that implements most of the behind-the-scenes code 
                                                                      //Pin assignments:
#define IR_RX 2                                                        // pin the IR receiver (left data pin) is connected to
#define SPEAKER 11                                                     // pin the piezo speaker is connected to

#define MSG_LENGTH 16                                                  // length of transmitted and received messages (must be same on ALL blasters)
                                                                       //Variables used throughout the code:
int im_hit = false;                                                    // a true/false variable that records if you're being hit
LaserRxTx laser = LaserRxTx(IR_RX, 13);                             // an object that lets you fire and receive IR signals

void setup() {                                                         //Function run once when Arduino is powered on
  Serial.begin(9600);                                                  // so we can receive messages / watch for errors
  pinMode(SPEAKER, OUTPUT);                                            // setup speaker pin as an output 
  pinMode(13, OUTPUT);
  pinMode(IR_RX, INPUT);                                               // setup IR receiving pin as an input 
  attachInterrupt(digitalPinToInterrupt(IR_RX),irInterrupt,FALLING);   // the function irInterrupt is called when an IR signal is being received
}


void irInterrupt() {                                                   //Function called when the IR receiver sees a new message
  im_hit = true;                                                       // records that we have been hit, but doesn't take any action immediately, like leaving a note on the fridge
}


void shot(char* secret_msg) {                                          //Function called when you get shot (it gets sent the shooter's codename)
  if (secret_msg == "")                                                // if no message received, cancel the shot
    return;                                                             
  digitalWrite(13, HIGH);

  MarioMain:  

  play_rtttl("d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b", SPEAKER);
  //,16p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16c7,16p,16c7,16c7,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16d#6,8p,16d6,8p,16c6"
  digitalWrite(13, LOW);  
}


void loop() {                                                          //Function run in a loop forever: checks if you're shooting and/or shot
  if (im_hit) {                                                        // when it checks here, if you were hit... 
    shot(laser.irRecv(MSG_LENGTH));                                    // starts the irRecv() function and return the sender's code to a function to do the shot logic
    im_hit = false;                                                    // reset the hit flag to false since you are no longer hit
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