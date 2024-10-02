#define btnOn 11
#define buzz 10

#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16, 2);

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};

Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

int vType[4];
int servoState = 1;

void setup() {
  Serial.begin(9600);
  pinMode(btnOn, INPUT);
  pinMode(buzz, OUTPUT);
  lcd.init();
  lcd.setCursor(0,0);
  lcd.backlight();
}

void loop(){
  if(digitalRead(btnOn)){
    if(servoState) {
      Serial.write('1');
      servoState ^= 1;
    }
    else {
      Serial.write('0');
      servoState ^= 1;
    }
    while(digitalRead(btnOn));
  }

  while(Serial.available()){
    if(Serial.read() == '8'){
      digitalWrite(buzz, HIGH);
      delay(1000);
      digitalWrite(buzz, LOW);
    }
  }

  char customKey = customKeypad.getKey();

  if(customKey) {
    keyCommand(customKey);
  }
  delay(10);
}

int state = 0, price = 0;
char vehicleType = '$';

void resetAll() {
  state = 0;
  vehicleType = '$';
  price = 0;
  lcd.clear();
}

void keyCommand(char c) {
  lcd.print(c);
  Serial.write(c);
  switch(state) {
    case 0 : {
      if(c == 'A' || c == 'B' || c == 'C') {
        vehicleType = c;
        state = 1;
      }
      else if(c == '#') {
        resetAll();
        return;
      }
      else if(c == '*') {
        state = 3;
      }
      break;
    }
    case 1 : {
      if(isDigit(c)) {
        price = 10 * price + c - '0';
        state = 2;
      }
      else if(c == '#') {
        resetAll();
        return;
      }
      break;
    }
    case 2 : {
      if(isDigit(c)) {
        price = 10 * price + c - '0';
      }
      else if(c == 'D') {
        vType[vehicleType - 'A' + 1] = price;
        resetAll();
      }
      else if(c == '#') {
        resetAll();
        return;
      }
      break;
    }
    case 3: {
      if(c == 'A' || c == 'B' || c == 'C') {
        lcd.clear();
        lcd.print(c);
        lcd.setCursor(0, 1);
        lcd.print(vType[c - 'A' + 1]);
      }
      else if(c == '#') {
        resetAll();
        return;
      }
      break;
    }
    default :
      break;
  }
}





//#define buzz 8
//
//void setup() {
//  Serial.begin(9600);
//  pinMode(buzz, OUTPUT);
//}
//
//void loop() {
//  while(Serial.available()){
//    if(Serial.read() == '1'){
//      digitalWrite(buzz, HIGH);
//    }
//    else{
//      digitalWrite(buzz, LOW);
//    }
//  }
//  delay(10);
//}
