#include <LiquidCrystal.h>
const int BLUE_LED = 51;
const int RED_LED = 53;  
const int YELLOW_LED = 52;
const int GREEN_LED = 50;

const int BUZZ = 22;

const int YELLOW_BUTTON = 47;
const int GREEN_BUTTON = 45;  
const int RED_BUTTON = 44;
const int BLUE_BUTTON = 46;

//notes used in og simon game
int tones[4] = {440, 494, 523, 587}; // Red, Green, Blue, Yellow

int buttonPins[4] = { RED_BUTTON, GREEN_BUTTON, BLUE_BUTTON, YELLOW_BUTTON };
bool buttonState[4] = { HIGH, HIGH, HIGH, HIGH };      
bool lastReading[4] = { HIGH, HIGH, HIGH, HIGH };       
unsigned long lastDebounceTime[4] = { 0, 0, 0, 0 };

bool INTRO_PLAYED = false; 
int INTROSTEP = 0;

bool IN_GAME = false;
bool hard = false;

int NEXT_TIME = 0;
int NEXT_ACTION = true;

int starting_length = 1;
int game_delay = 500;

bool buttonLocked = false;
int lastButton = -1;

int sequence[50];
int seqLength = 1;
int userStep = 0;
bool showingSequence = false;
unsigned long seqTimer = 0;
int seqIndex = 0;
bool waitingForInput = false;
int flashDuration = 400; // show time
int gapDuration = 200; // pause
bool ledOn = false;
unsigned long ledTimer = 0;

int intro[20][6] = {
  {1,0,0,0,440,250}, // Red
  {0,1,0,0,494,250}, // Green
  {0,0,1,0,523,250}, // Blue
  {0,0,0,1,587,300}, // Yellow
  {0,0,0,0,0,150},     
  {0,1,0,0,494,200}, // Green
  {1,0,0,0,440,200}, // Red
  {0,1,0,0,494,250}, // Green
  {0,0,1,0,523,250}, // Blue
  {0,0,0,1,659,400}, // Yellow
  {0,0,0,0,0,200},     
  {0,0,1,0,523,200}, // Blue
  {0,1,0,0,494,200}, // Green
  {1,0,0,0,440,300}, // Red
  {0,0,0,0,0,150},     
  {0,1,0,1,587,250}, // Green + Yellow 
  {0,0,1,0,659,300}, // Blue
  {1,0,0,0,440,200}, // Red
  {0,0,0,1,784,400}, // Yellow
  {0,0,0,0,0,400}      
};

const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {

  Serial.begin(115200);
  Serial.println(F("Console Ready"));
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

  
  lcd.begin(16, 2);
  lcd.print("Simon Game");
  
  
}

void loop() {
  if(INTRO_PLAYED == false){
    if(NEXT_TIME == 0&&NEXT_ACTION == true){
      float note = intro[INTROSTEP][4];
      if(tone>0) {
        tone(BUZZ,note);
      } else {
        noTone(BUZZ);
      }
      if(intro[INTROSTEP][0]==1){
        digitalWrite(RED_LED, HIGH);
      }
      if(intro[INTROSTEP][1]==1){
        digitalWrite(GREEN_LED, HIGH);
      }
      if(intro[INTROSTEP][2]==1){
        digitalWrite(BLUE_LED, HIGH);
      }
      if(intro[INTROSTEP][3]==1){
        digitalWrite(YELLOW_LED, HIGH);
      }
      NEXT_TIME = millis()+intro[INTROSTEP][5];
      NEXT_ACTION = false;
      
      Serial.println("note: "+String(note));
      Serial.println("next time: "+String(NEXT_TIME));
    
    }
    if(NEXT_TIME < millis()){
      if(NEXT_ACTION == false){
        noTone(BUZZ);
        off();
        
        NEXT_ACTION = true;
        NEXT_TIME = 0;
        INTROSTEP+=1;
        if(INTROSTEP >= sizeof(intro)/sizeof(intro[0])){
          INTRO_PLAYED = true;
        }

      }
    }

  } else {
    updateButtons();
    if (IN_GAME == false) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Select Difficulty ");
      lcd.setCursor(0, 1);
      lcd.print("B=Easy R=Hard");

      if (buttonState[2] == LOW) { // Red = Easy
        hard = false;
        startGame();
      }
      if (buttonState[0] == LOW) { // Blue = Hard
        hard = true;
        startGame();
      }
    }
    else if (IN_GAME == true) {
      simonLoop();
    }
  }


}

void off(){
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
}

void updateButtons() {
  for (int i = 0; i < 4; i++) {
    int reading = digitalRead(buttonPins[i]);
    if (reading != lastReading[i]) {
      lastDebounceTime[i] = millis(); 
    }
    if ((millis() - lastDebounceTime[i]) > 50) {
      if (reading != buttonState[i]) {
        buttonState[i] = reading;
        Serial.println("button switched states");
      }
    }
    lastReading[i] = reading;
  }
}

void startGame(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Starting");
  lcd.setCursor(0, 1);
  if (hard == false) lcd.print("Easy!");
  else lcd.print("Hard!");
  IN_GAME = true;
  seqLength = starting_length;
  generateSequence();
  showingSequence = true;
  seqIndex = 0;
  userStep = 0;
  seqTimer = millis();
  // optionally, adjust difficulty parameters here
  if (hard == false) {
    flashDuration = 400;
    gapDuration = 200;
  } else {
    flashDuration = 250;
    gapDuration = 100;
  }
}

void simonLoop(){
  if(showingSequence){
    // Show the sequence using millis()
    if(!ledOn && millis() - seqTimer >= gapDuration){
      int color = sequence[seqIndex];
      activateLED(color);
      tone(BUZZ, tones[color]);
      ledOn = true;
      ledTimer = millis();
    }

    if(ledOn && millis() - ledTimer >= flashDuration){
      off();
      noTone(BUZZ);
      ledOn = false;
      seqIndex++;
      seqTimer = millis();
      if(seqIndex >= seqLength){
        showingSequence = false;
        waitingForInput = true;
        lcd.clear();
        lcd.print("Your turn!");
        userStep = 0;
      }
    }
  } else if (waitingForInput) {
    bool anyPressed = false;
    for (int i = 0; i < 4; i++) {
      if (buttonState[i] == LOW) { // pressed
        anyPressed = true;
        if (!buttonLocked) { // one input per press
          buttonLocked = true;
          lastButton = i;
          activateLED(i);
          tone(BUZZ, tones[i]);
          Serial.print("Button pressed: ");
          Serial.println(i);

          if (sequence[userStep] == i) {
            userStep++;
            // successful press — wait for release (handled by lock)
            if (userStep >= seqLength) {
              lcd.clear();
              lcd.print("Good job!");
              
              delay(500);
              noTone(BUZZ);
              off();
              delay(1000);
              seqLength++;
              waitingForInput = false;
              showingSequence = true;
              seqIndex = 0;
              seqTimer = millis() + 1000;
            }
          } else {
            lcd.clear();
            lcd.print("Game Over!");
            lcd.setCursor(0,1);
            lcd.print("Score:" + String(seqLength));
            tone(BUZZ, 200);
            delay(5000);
            noTone(BUZZ);
            delay(5000); // short feedback delay
            noTone(BUZZ);
            off();
            IN_GAME = false;

          }
        }
      }
    }
    // unlock when all buttons released
    if (!anyPressed && buttonLocked) {
      buttonLocked = false;
      off();
      noTone(BUZZ);
    }
  }
}
void activateLED(int color){
  off();
  switch(color){
    case 0: digitalWrite(RED_LED, HIGH); break;
    case 1: digitalWrite(GREEN_LED, HIGH); break;
    case 2: digitalWrite(BLUE_LED, HIGH); break;
    case 3: digitalWrite(YELLOW_LED, HIGH); break;
  }
}
void generateSequence(){
  randomSeed(analogRead(A0));
  for(int i=0; i<50; i++){
    sequence[i] = random(0,4);
  }
}
