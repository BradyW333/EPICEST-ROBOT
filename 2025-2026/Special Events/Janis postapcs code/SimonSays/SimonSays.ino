#include <LiquidCrystal.h>
const int BLUE_LED = 51;
const int RED_LED = 53;  
const int YELLOW_LED = 52;
const int GREEN_LED = 50;

const int BUZZ = 48;

const int YELLOW_BUTTON = 47;
const int GREEN_BUTTON = 45;  
const int RED_BUTTON = 44;
const int BLUE_BUTTON = 46;

//notes used in og simon game
const float NOTE_E3 = 164.81; //blue 0 
const float NOTE_A3 = 220; //red 1
const float NOTE_C4S = 277.18; //green 2
const float NOTE_E4 = 329.63; //yellow 3
const float NOTE_C5 = 523.25; //4

const float NOTES[5] = {
  164.81,
  220,
  277.18,
  329.63,
  523.25
};

int YBUTT = 0;
int GBUTT = 0;
int RBUTT = 0;
int BBUTT = 0;

int YLED = 0;
int GLED = 0;
int RLED = 0;
int BLED = 0;

bool INTRO_PLAYED = false; 
int INTROSTEP = 0;
int INTROLEN = 700;
int INTROPAUSE = 150;

int NEXT_TIME = 0;
int NEXT_ACTION = true;

char intro[5][5] = {
  "0b",
  "1r",
  "2g",
  "3y",
  "4rgby"
};

const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
    Serial.begin(115200);

  // put your setup code here, to run once:
  pinMode(BLUE_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  
  pinMode(BUZZ, OUTPUT);
  
  pinMode(YELLOW_BUTTON, INPUT_PULLUP);
  pinMode(GREEN_BUTTON, INPUT_PULLUP);
  pinMode(RED_BUTTON, INPUT_PULLUP);
  pinMode(BLUE_BUTTON, INPUT_PULLUP);

  digitalWrite(BLUE_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  digitalWrite(BUZZ, LOW);

  Serial.println(F("Console Ready"));
  Serial.println(String(NOTES[0]));
  lcd.begin(16, 2);
  lcd.print("FA is awesome!");
  
  
}

void loop() {
  if(INTRO_PLAYED == false){
    if(NEXT_TIME == 0&&NEXT_ACTION == true){
      float note = NOTES[String(intro[INTROSTEP][0]).toInt()];
      tone(BUZZ,note);
      if(strchr(intro[INTROSTEP],"r")!=NULL){
        digitalWrite(RED_LED, HIGH);
      }
      if(strchr(intro[INTROSTEP],"g")!=NULL){
        digitalWrite(GREEN_LED, HIGH);
      }
      if(strchr(intro[INTROSTEP],"b")!=NULL){
        digitalWrite(BLUE_LED, HIGH);
      }
      if(strchr(intro[INTROSTEP],"y")!=NULL){
        digitalWrite(YELLOW_LED, HIGH);
      }
      NEXT_TIME = millis()+INTROLEN;
      NEXT_ACTION = false;
      
      Serial.println(String(note));


    }
    if(NEXT_TIME < millis()){
      if(NEXT_ACTION == false){
        noTone(BUZZ);
        off();
        NEXT_TIME = millis()+INTROPAUSE;
        NEXT_ACTION = true;
      } else {
        
        NEXT_TIME = 0;
        INTROSTEP+=1;
        if(INTROSTEP >= sizeof(intro)/sizeof(intro[0])){
          INTRO_PLAYED = true;
        }
      }
    }

  }

  // if(YBUTT==0){
  //   Serial.println("YBUTT");
  //   digitalWrite(YELLOW_LED, HIGH);
  // } else {
  //   digitalWrite(YELLOW_LED, LOW);
  // }
  // if(GBUTT==0){
  //   Serial.println("GBUTT");
  //   digitalWrite(GREEN_LED, HIGH);
  // } else {
  //   digitalWrite(GREEN_LED, LOW);
  // }
  // if(RBUTT==0){
  //   Serial.println("RBUTT");
  //   digitalWrite(RED_LED, HIGH);
  //   tone(BUZZ, 440);
  // } else {
  //   digitalWrite(RED_LED, LOW);
  // }
  // if(BBUTT==0){
  //   Serial.println("BBUTT");
  //   digitalWrite(BLUE_LED, HIGH);
  //   tone(BUZZ, 523.25);
    
  // } else {
  //   digitalWrite(BLUE_LED, LOW);
    
  // }
  // if(RBUTT==1&BBUTT==1){
  //   noTone(BUZZ);
  // }
  // put your main code here, to run repeatedly:

}

void off(){
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
}
void checkbutts(){
  YBUTT = digitalRead(YELLOW_BUTTON);
  GBUTT = digitalRead(GREEN_BUTTON);
  RBUTT = digitalRead(RED_BUTTON);
  BBUTT = digitalRead(BLUE_BUTTON);
}
void leds(){
  if(YLED==1){
    digitalWrite(YELLOW_LED, HIGH);
  } else {
    digitalWrite(YELLOW_LED, LOW);
  }
  if(GLED==1){
    digitalWrite(GREEN_LED, HIGH);
  } else {
    digitalWrite(GREEN_LED, LOW);
  }
  if(RLED==1){
    digitalWrite(RED_LED, HIGH);
  } else {
    digitalWrite(RED_LED, LOW);
  }
  if(BLED==1){
    digitalWrite(BLUE_LED, HIGH);
  } else {
    digitalWrite(BLUE_LED, LOW);
  }
}

