#include <DS3231.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Servo.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define RST_PIN 9
#define SS_PIN 10
#define BUZZER_PIN 8
#define TOT_TP 3
#define IR1_PIN 2
#define IR2_PIN 3
#define ULTRASONIC_TRIG_PIN 4
#define ULTRASONIC_ECHO_PIN 5

DS3231 rtc(SDA, SCL);
Time t;

Time t1, t2;
unsigned long ir1, ir2, ul1, ul2;
int mesSt = 0;
const float distanceBetweenIR = 18;
long duration;
int distance;

void resetMes() {
  mesSt = 0;
}

MFRC522 mfrc522(SS_PIN, RST_PIN);

String tagID = "";

String card[10];
int flag[10];
Time stTime[10];
int vType[4];

int cost = 1;

int inOrOut = 0;
int ff = 0;
int fndInd;

Servo myservo;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  SPI.begin();
  mfrc522.PCD_Init();
  rtc.begin();
  myservo.attach(6);
  myservo.write(0);
  delay(4);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Vehicle Expresso");
  pinMode(IR1_PIN, INPUT);
  pinMode(IR2_PIN, INPUT);
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
}

void loop() {
  t = rtc.getTime();
  switch (mesSt) {
    case 0: {
//      Serial.println("State0");
      if (digitalRead(IR1_PIN) == LOW) {
        mesSt = 1;
        fndInd = 1;
        ir1 = millis();
      }
      break;
    }
    case 1: {
//      Serial.println("State1");
      digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
      delayMicroseconds(2);

      digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
      delayMicroseconds(10);

      digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

      duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);
      distance = duration * 0.0344 / 2;

//      Serial.print("Distance: ");
//      Serial.println(distance);
-
      if (distance < 30) {
        mesSt = 2;
        ul1 = millis();
//        Serial.print("ul1: ");
//        Serial.println(ul1);
      }
      break;
    }
    case 2: {
//      Serial.println("State2");
      digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
      delayMicroseconds(2);

      digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
      delayMicroseconds(10);

      digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

      duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);
      distance = duration * 0.0344 / 2;

//      Serial.print("Distance: ");
//      Serial.println(distance);

      if (distance > 30) {
        mesSt = 3;
        ul2 = millis();
//        Serial.print("ul2: ");
//        Serial.println(ul2);
      }
      break;
    }
    case 3: {
//      Serial.println("State3");
      if (digitalRead(IR2_PIN) == LOW) {
        ir2 = millis();
//        Serial.print("ul1: ");
//        Serial.println(ul1);
//        Serial.print("ul2: ");
//        Serial.println(ul2);
//        Serial.print("ir1: ");
//        Serial.println(ir1);
//        Serial.print("ir2: ");
//        Serial.println(ir2);

        float ulDiff = (ul2 - ul1);

//        Serial.print("car delay ");
//        Serial.println(ulDiff);
        float irDiff = (ir2 - ir1);
        float speed = distanceBetweenIR / irDiff;
        float len = ulDiff * speed;
        fndInd = ((int)len % TOT_TP ) + 1;
//        Serial.print("Car length: ");
//        Serial.println(len);
        mesSt = 0;
      }
      break;
    }
  }


  while (getID()) {
    lcd.clear();
    Serial.write('8');
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);

    int isAble = -1;

    for (int i = 0; i < 10; i++) {
      if (card[i] == tagID) {
        isAble = i;
        break;
      }
    }

    if (isAble == -1) {
      int isIns = -1;
      for (int i = 0; i < 10; i++) {
        if (flag[i] == 0) {
          card[i] = tagID;
          isIns = i;
          break;
        }
      }

      if (isIns != -1) {
        inOrOut = 1;
        card[isIns] = tagID;
        flag[isIns] = fndInd;
        stTime[isIns] = t;
        lcd.clear();
        lcd.print("Vehicle inserted");
      }
      else {
        lcd.clear();
        lcd.print("Garage is full");
      }
    }
    else {
      t1 = stTime[isAble];
      t2 = t;

      long seconds1 = dateTimeToSeconds(t1);
      long seconds2 = dateTimeToSeconds(t2);

      lcd.clear();

      lcd.print("TK : ");
      lcd.print((seconds2 - seconds1) * vType[flag[isAble]]);
      card[isAble] = "";
      flag[isAble] = 0;
      inOrOut = 2;
    }
  }

  while (Serial.available()) {
    keyCommand(Serial.read());
  }
}

int state = 0, price = 0;
char vehicleType = '$';

void resetAll() {
  state = 0;
  vehicleType = '$';
  price = 0;
}

void keyCommand(char c) {
  if (c == '\n') return;

  switch (state) {
    case 0: {
      if (c == 'A' || c == 'B' || c == 'C') {
        vehicleType = c;
        state = 1;
      }
      else if (c == '1') {
        myservo.write(90);
        lcd.clear();
      }
      else if (c == '0') {
        myservo.write(0);
      }
      else if (c == '#') {
        resetAll();
        return;
      }
      break;
    }
    case 1: {
      lcd.clear();
      lcd.print(c);
      if (isDigit(c)) {
        price = 10 * price + c - '0';
        state = 2;
      }
      else if (c == '#') {
        resetAll();
        return;
      }
      break;
    }
    case 2: {
      lcd.clear();
      lcd.print(c);
      if (isDigit(c)) {
        price = 10 * price + c - '0';
      }
      else if (c == 'D') {
        lcd.clear();
        lcd.print(vehicleType);
        lcd.setCursor(0, 1);
        lcd.print(price);
        vType[vehicleType - 'A' + 1] = price;
        resetAll();
      }
      else if (c == '#') {
        resetAll();
        return;
      }
      break;
    }
    default:
      break;
  }
}

boolean getID() {
  // Getting ready for Reading PICCs
  if (!mfrc522.PICC_IsNewCardPresent()) {  // If a new PICC placed to RFID reader continue
    return false;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {  // Since a PICC placed get Serial and continue
    return false;
  }
  tagID = "";
  for (uint8_t i = 0; i < 4; i++) {  // The MIFARE PICCs that we use have 4 byte UID
    // readCard[i] = mfrc522.uid.uidByte[i];
    tagID.concat(String(mfrc522.uid.uidByte[i], HEX));  // Adds the 4 bytes in a single String variable
  }
  tagID.toUpperCase();
  mfrc522.PICC_HaltA();  // Stop reading
  return true;
}

// Helper function to convert a DateTime object to seconds since a reference date
long dateTimeToSeconds(Time dt) {
  // Constants for seconds calculation
  const int SECONDS_PER_MINUTE = 60;
  const int SECONDS_PER_HOUR = 3600;
  const int SECONDS_PER_DAY = 86400;

  // Calculate seconds for the date part
  int daysSinceEpoch = calculateDaysSinceEpoch(dt.year, dt.mon, dt.date);
  long dateInSeconds = daysSinceEpoch * SECONDS_PER_DAY;

  // Calculate seconds for the time part
  long timeInSeconds = dt.hour * SECONDS_PER_HOUR + dt.min * SECONDS_PER_MINUTE + dt.sec;

  // Total seconds
  return dateInSeconds + timeInSeconds;
}

// Helper function to calculate days since a fixed reference date (e.g., Unix epoch)
int calculateDaysSinceEpoch(int year, int month, int day) {
  // Use the Unix epoch as the reference date (January 1, 1970)
  int referenceYear = 1970;
  int referenceMonth = 1;
  int referenceDay = 1;

  // Count the number of days from the reference date to the given date
  int days = 0;

  // Calculate days for years
  for (int y = referenceYear; y < year; y++) {
    days += (isLeapYear(y) ? 366 : 365);
  }

  // Calculate days for months in the current year
  for (int m = referenceMonth; m < month; m++) {
    days += daysInMonth(m, year);
  }

  // Add days in the current month
  days += (day - referenceDay);

  return days;
}

// Helper function to check if a year is a leap year
bool isLeapYear(int year) {
  if (year % 4 == 0) {
    if (year % 100 == 0) {
      if (year % 400 == 0) {
        return true;
      }
      else {
        return false;
      }
    }
    else {
      return true;
    }
  }
  else {
    return false;
  }
}

int daysInMonth(int month, int year) {
  switch (month) {
    case 1:
      return 31;  // January
    case 2:
      return (isLeapYear(year) ? 29 : 28);  // February
    case 3:
      return 31;  // March
    case 4:
      return 30;  // April
    case 5:
      return 31;  // May
    case 6:
      return 30;  // June
    case 7:
      return 31;  // July
    case 8:
      return 31;  // August
    case 9:
      return 30;  // September
    case 10:
      return 31;  // October
    case 11:
      return 30;  // November
    case 12:
      return 31;  // December
    default:
      return 0;  // Invalid month
  }
}
