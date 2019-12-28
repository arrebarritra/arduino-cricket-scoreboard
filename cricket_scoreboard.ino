#include <LiquidCrystal.h> 

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
const int potPin = 0;
const int switchPin = 13;

float potInput = 0;
float prevPotInput = 0;
bool stickyPot = false;
int lcdNumber = 0;
int prevLcdNumber = 0;

int switchInput = 0;
int prevSwitchInput = 0;
long lastPressed;
long pressCooldownTime = 200;
String modes[4] = {" ", "R", "W", "B"};
int mode = 0;

const int maxOutputs = 4;
String outputs[maxOutputs] = {"", "", "", ""};
int outputCursorRow[maxOutputs] = {0, 0, 0, 0};
int outputCursorColumn[maxOutputs] = {0, 0, 0, 0};
bool outputChanged = false;

int runs = 0;
int wickets = 0;
int balls = 0;

String ballsToOvers(){
  return (String) (balls / 6) + "." + (String) (balls % 6);
}

void lcdPrint(){
  if(outputChanged == true){
    outputChanged = false;
    lcd.clear();
    for(int i = 0; i < maxOutputs; i++){
      lcd.setCursor(outputCursorRow[i], outputCursorColumn[i]);
      lcd.print(outputs[i]);
    }
  }
}

void lcdQueuePrint(String output, int row, int column, int index){
  outputChanged = true;
  
  outputs[index] = output;
  outputCursorRow[index] = row;
  outputCursorColumn[index] = column;
}

int lerp1023(float min, float max, float t){
  t = t / 1023;
  return (int) lround(min + t * (max - min));
}

void changeMode(){
  if(modes[mode] == "R" && runs + lcdNumber >= 0){
    runs += lcdNumber;
  } else if(modes[mode] == "W" && wickets + lcdNumber >= 0){
    wickets += lcdNumber;
  } else if(modes[mode] == "B" && balls + lcdNumber >= 0){
    balls += lcdNumber;    
  }
  lcdQueuePrint((String) runs + "/" + (String) wickets, 0, 0, 0);
  lcdQueuePrint(ballsToOvers(), 0, 1, 1);
  
  mode = (mode == sizeof(modes) / sizeof(modes[0]) - 1) ? 0 : mode + 1;

  switch(mode){
    case 0:
      lcdQueuePrint("  ", 14, 0, 2);
      break;
    case 1:
    case 2:
      lcdNumber = prevLcdNumber = 0;
      lcdQueuePrint(" 0", 14, 0, 2);
      stickyPot = true;
      break;
    case 3:
      lcdNumber = prevLcdNumber = 1;
      lcdQueuePrint("+1", 14, 0, 2);
      stickyPot = true;
      break;
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(potPin,INPUT);
  pinMode(switchPin,INPUT);

  lcd.begin(16,2);
  lcdQueuePrint((String) runs + "/" + (String) wickets, 0, 0, 0);
  lcdQueuePrint(modes[mode], 15, 1, 2);
}

void loop() {  
  prevPotInput = potInput;
  potInput = analogRead(potPin);

  prevLcdNumber = lcdNumber;

  if(abs(potInput - prevPotInput) > 2 || !stickyPot){
    stickyPot = false;
    String sign;
    if(modes[mode] == "R"){
      int min;
      min = (runs < 6) ? -runs : -6;
      lcdNumber = lerp1023(min, 6, potInput);
      if(lcdNumber != prevLcdNumber){
        if(lcdNumber > 0){
          sign = "+";
        } else if(lcdNumber == 0){
          sign = " ";
        }
        lcdQueuePrint(sign + lcdNumber, 14, 0, 2);
      }
    } else if(modes[mode] == "W"){
      int min;
      min = (wickets == 0) ? 0 : -1;
      lcdNumber = lerp1023(min, 1, potInput);
      if(lcdNumber != prevLcdNumber){
        if(lcdNumber > 0){
          sign = "+";
        } else if(lcdNumber == 0){
          sign = " ";
        }
        lcdQueuePrint(sign + lcdNumber, 14, 0, 2);
      }
    } else if(modes[mode] == "B"){
      int min;
      min = (balls < 6) ? -balls : -6;
      lcdNumber = lerp1023(min, 6, potInput);
      if(lcdNumber != prevLcdNumber){
        if(lcdNumber > 0){
          sign = "+";
        } else if(lcdNumber == 0){
          sign = " ";
        }
        lcdQueuePrint(sign + lcdNumber, 14, 0, 2);
      }
    }
  }
  

  prevSwitchInput = switchInput;
  switchInput = digitalRead(switchPin);
  long pressTime = millis() - lastPressed;
  
  if(switchInput == HIGH & prevSwitchInput == LOW & pressTime > pressCooldownTime){
    changeMode();
    lcdQueuePrint(modes[mode], 15, 1, 3);
    lastPressed = millis();
  }

  lcdPrint();
}
