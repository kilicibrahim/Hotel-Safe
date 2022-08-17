#include <Key.h>
#include <EEPROM.h>
#include <Keypad.h>
#include <LiquidCrystal.h>
//EEPROM.length()=4096
//4,096 max
//EEPROM.write(adress, value);
//adresses
int adressSucsesfullAttemps = 0;
int adressUnsucsesfullAttemps = 1;
int adressAPassword = 2;
int adressBPassword = 3;
int adressBaitPassword = 4;
int adressMasterPassword = 5;


//interrupt;
// timer1 max = 65535
// 1/16MHZ = 62.5ms
//(presc) 8/16MHZ = 500ms
// 15 /(1/16Mhz) = 240m
//(presc)1024 -> 234,375
// 3 seconds = 46,875
// I can use this one to reset SFA

LiquidCrystal lcd(14, 15, 16, 17, 18, 19);

const byte ROW = 4;
const byte COL = 3;


String paswMaster = "1234";
String paswA = "2222";
String paswB = "3333";
String paswBait = "9999";
void state_1 ();
void control_pasw(char pasw[5]);
char keys[ROW][COL] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROW] = {4, 3, 2, 1};
byte colPins[COL] = {7, 6, 5};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROW, COL);
int i = 1;
char entPasw;

int SFA = 0;

bool x = false;
bool y = false;
bool z = false;

int sucsesfullAttemps = 0;
int unsucsesfullAttemps = 0;

int EN1 = 13;
int EN2 = 12;

int Led = 20;

int Status_MotorA = 0;
int Status_MotorB = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.begin(16, 2);
  pinMode(Led, OUTPUT);
  pinMode(EN1, OUTPUT);
  pinMode(EN2, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(8, OUTPUT);
  //Reset Timer1 Control Reg A
  TCCR1A = 0;
  //Set CTC mode
  TCCR1B &= ~(1 << WGM13);
  TCCR1B |= (1 << WGM12);
  //Set prescaler of 1024
  //CSn2 CSn1 CSn0
  //1     0     1 clkI/O/1024 (From prescaler)
  TCCR1B |= (1 << CS12);
  TCCR1B &= ~(1 << CS11);
  TCCR1B |= (1 << CS10);
  //Reset Timer1 and set compare value
  TCNT1 = 0;
  OCR1A = 46875;
  //Enable Timer1 compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  //Enable global interrupts
  sei();

  lcd.setCursor(0, 0);
  lcd.print("   Welcome to");
  // entPasw[4] = '\0';
  lcd.setCursor(0, 1);
  lcd.print("   Hotel Safe");
  delay(2500);
  state1();
}
// my calculation was 3 secons every time I enter the interrupt but although my calculations were correct it worked differently
// so with the help of TA Cengiz Emre Dedeagac I realized I go into intterrupt every 0.45 sec so I chanced the original plan to
// s == 34 it will give me 15.3 seconds (so its roughly 15 secons)
int s = 0;
ISR(TIMER1_COMPA_vect) {
  if (s == 34) {
    SFA = 0;
    s = 0;
  } else 
    s++;
}
//enteredPasw(entPasw);

//char aPasw[] = {'2', '2', '2', '2', '\0'};
//char bPasw[] = {'3', '3', '3', '3', '\0'};
//char masterPasw[] = {'1', '2', '3', '4', '\0'};
//char baitPasw[] = {'9', '9', '9', '9', '\0'};

// Starting = state 0
// Main Menu = state 1
// Open/Lock Section A = state 1.1(enteringA or enteringMaster)
// Open/Lock Section B = state 1.2(enteringB or enteringMaster)
// Change Password = state 1.3(Change Password)
// Report Previous Attempts = state 1.4
// Control Password = state 2 (controlA, controlB, controlMaster)

// Starting = state 0
void loop() {


  //   entPasw[i] = (key % 10) + '0' ;


}
// Main Menu = state 1

void state1 () {
  digitalWrite(EN1, HIGH);
  digitalWrite(EN2, HIGH);
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Main Menu");
  lcd.setCursor(1, 1);
  lcd.print("1-A 2-B 3-M 4-R");
  while (1) {
    char key = keypad.getKey();
    if (key) {

      if (key == '1') {
        enteringA();
      }
      if (key == '2') {
        enteringB();
      }
      if (key == '3') {
        enteringMaster();//change Password
      }
      if (key == '4') {
        reportPrevious();
      }      
    }
  }
}

void enteringA() {
  i = 1;
  String pressedKey;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter Password A");
  lcd.setCursor(0, 1);
  while (i < 5) {
    char key = keypad.getKey();
    if (key) {
      lcd.print("*");
      pressedKey = pressedKey + key;
      i++;
    }
  }
  controlA(pressedKey);
}

void enteringB() {
  i = 1;

  String pressedKey;
  lcd.clear();
  lcd.print("Enter Password B");
  lcd.setCursor(0, 1);
  while (i < 5) {
    char key = keypad.getKey();
    if (key) {

      lcd.print("*");
      pressedKey = pressedKey + key;
      i++;
    }
  }
  controlB(pressedKey);
}

void enteringMaster() {
  i = 1;
  String pressedKey;
  lcd.clear();
  lcd.print("Enter Master Psw");
  lcd.setCursor(0, 1);
  while (i < 5) {
    char key = keypad.getKey();
    if (key) {

      lcd.print("*");
      pressedKey = pressedKey + key;
      i++;
    }
  }
  controlMaster(pressedKey);
}

void controlA(String givenPasw) {
  bool x = givenPasw.equals(paswA);
  bool y = givenPasw.equals(paswMaster);
  bool z = givenPasw.equals(paswBait);
  if (x) {
    if (Status_MotorA == 0)
      openMotorA();
    else if (Status_MotorA == 1)
      closeMotorA();
    delay(1000);
    SFA = 0;
    sucsesfullAttemps++;
    EEPROM.write(adressSucsesfullAttemps, sucsesfullAttemps);
    state1();
  }
  else if (y) {
    if (Status_MotorA == 0)
      openMotorA();
    else if (Status_MotorA == 1)
      closeMotorA();
    delay(1000);
    SFA = 0;
    sucsesfullAttemps++;
    EEPROM.write(adressSucsesfullAttemps, sucsesfullAttemps);
    state1();
  }
  else if (z) {
    if (Status_MotorA == 0) {
      openMotorA();
      digitalWrite(Led, HIGH);
      delay(250);
      digitalWrite(Led, LOW);
      delay(250);
      digitalWrite(Led, HIGH);
      delay(250);
      digitalWrite(Led, LOW);
      delay(250);
    }
    else if (Status_MotorA == 1) {
      closeMotorA();
      digitalWrite(Led, HIGH);
      delay(250);
      digitalWrite(Led, LOW);
      delay(250);
      digitalWrite(Led, HIGH);
      delay(250);
      digitalWrite(Led, LOW);
      delay(250);    
    }
    SFA = 0;
    sucsesfullAttemps++;
    EEPROM.write(adressSucsesfullAttemps, sucsesfullAttemps);
    state1();
  }
  else if (SFA < 2) {
    unsucsesfullAttemps++;
    EEPROM.write(adressUnsucsesfullAttemps, unsucsesfullAttemps);
    SFA++;
    lcd.clear();
    lcd.print("Wrong Password");
    lcd.setCursor(0, 1);
    lcd.print("Try again");
    delay(1500);
    enteringA();
  }
  else {
    unsucsesfullAttemps++;
    EEPROM.write(adressUnsucsesfullAttemps, unsucsesfullAttemps);
    masterLock();
  }
}

void controlB(String givenPasw) {

  bool x = givenPasw.equals(paswB);
  bool y = givenPasw.equals(paswMaster);
  bool z = givenPasw.equals(paswBait);

  if (x) {
    if (Status_MotorB == 0)
      openMotorB();
    else if (Status_MotorB == 1)
      closeMotorB();
    delay(1000);
    SFA = 0;
    sucsesfullAttemps++;
    EEPROM.write(adressSucsesfullAttemps, sucsesfullAttemps);
    state1();
  }
  else if (y) {
    if (Status_MotorB == 0)
      openMotorB();
    else if (Status_MotorB == 1)
      closeMotorB();
    delay(1000);
    SFA = 0;
    sucsesfullAttemps++;
    EEPROM.write(adressSucsesfullAttemps, sucsesfullAttemps);
    state1();
  }
  else if (z) {
    if (Status_MotorB == 0) {
      openMotorB();
      digitalWrite(Led, HIGH);
      delay(250);
      digitalWrite(Led, LOW);
      delay(250);
      digitalWrite(Led, HIGH);
      delay(250);
      digitalWrite(Led, LOW);
      delay(250);
     

    }

    else if (Status_MotorB == 1) {
      closeMotorB();
      digitalWrite(Led, HIGH);
      delay(250);
      digitalWrite(Led, LOW);
      delay(250);
      digitalWrite(Led, HIGH);
      delay(250);
      digitalWrite(Led, LOW);
      delay(250);
      

    }


    SFA = 0;
    sucsesfullAttemps++;
    EEPROM.write(adressSucsesfullAttemps, sucsesfullAttemps);
    state1();
  }
  else if (SFA < 2) {
    SFA++;
    lcd.clear();
    lcd.print("Wrong Password");
    lcd.setCursor(0, 1);
    lcd.print("Try again");
    delay(1500);
    enteringB();

  }
  else {
    unsucsesfullAttemps++;
    EEPROM.write(adressUnsucsesfullAttemps, unsucsesfullAttemps);
    masterLock();
  }
}

void controlMaster(String givenPasw) {
  bool y = givenPasw.equals(paswMaster);
  bool z = givenPasw.equals(paswBait);
  if (y) {
    sucsesfullAttemps++;
    EEPROM.write(adressSucsesfullAttemps, sucsesfullAttemps);
    lcd.clear();
    lcd.print("CP 1M 2A 3B 4T");
    SFA = 0;
    while (1) {
      char key = keypad.getKey();
      if (key) {
        lcd.setCursor(0, 0);
        lcd.print("    Main Menu");

        if (key == '1') {
          changeMaster();
        }
        if (key == '2') {
          changeA();
        }
        if (key == '3') {
          changeB();
        }
        if (key == '4') {
          changeBait();
        }
        delay(100);
      }
    }
  }
  else if (SFA < 2) {
    unsucsesfullAttemps++;
    EEPROM.write(adressUnsucsesfullAttemps, unsucsesfullAttemps);
    SFA++;
    lcd.clear();
    lcd.print("Wrong Password");
    lcd.setCursor(0, 1);
    lcd.print("Try again");
    delay(1500);
    enteringMaster();
  }
  else {
    unsucsesfullAttemps++;
    EEPROM.write(adressUnsucsesfullAttemps, unsucsesfullAttemps);
    masterLock();
  }
}

void masterLock() {
  i = 1;

  String pressedKey;
  lcd.clear();
  lcd.print("System Locked MP");
  lcd.setCursor(0, 1);
  while (i < 5) {
    char key = keypad.getKey();
    if (key) {

      lcd.print("*");
      pressedKey = pressedKey + key;
      i++;
    }
  }
  bool y = pressedKey.equals(paswMaster);
  if (y) {
    sucsesfullAttemps++;
    EEPROM.write(adressSucsesfullAttemps, sucsesfullAttemps);
    state1();
  }
  else {
    unsucsesfullAttemps++;
    EEPROM.write(adressUnsucsesfullAttemps, unsucsesfullAttemps);
    timeLock();
  }
}
void timeLock() {
  for (int j = 20; j >= 1; j--) {
    lcd.clear();
    lcd.print("    No access");
    lcd.setCursor(0, 1);
    lcd.print("       " + String(j));
    delay(1000);
  }
  masterLock();
}

void changeMaster() {
  i = 1;
  String pressedKey;
  lcd.clear();
  lcd.print("Enter New Master");
  lcd.setCursor(0, 1);
  while (i < 5) {
    char key = keypad.getKey();
    if (key) {

      lcd.print("*");
      pressedKey = pressedKey + key;
      i++;
    }
  }
  paswMaster = pressedKey;
  // EEPROM.write(adressMasterPasswordPassword, paswMaster);
  lcd.clear();
  lcd.print("Pasw is changed");
  delay(1000);
  state1();

}

void changeA() {
  i = 1;
  String pressedKey;
  lcd.clear();
  lcd.print("Enter New A");
  lcd.setCursor(0, 1);
  while (i < 5) {
    char key = keypad.getKey();
    if (key) {
      lcd.print("*");
      pressedKey = pressedKey + key;
      i++;
    }
  }
  paswA = pressedKey;
  // EEPROM.write(adressAPassword, paswA);
  lcd.clear();
  lcd.print("Pasw is changed");
  delay(1000);
  state1();
}

void changeB() {
  i = 1;
  String pressedKey;
  lcd.clear();
  lcd.print("Enter New B");
  lcd.setCursor(0, 1);
  while (i < 5) {
    char key = keypad.getKey();
    if (key) {

      lcd.print("*");
      pressedKey = pressedKey + key;
      i++;
    }
  }
  paswB = pressedKey;
  // EEPROM.write(adressBPassword, paswB);
  lcd.clear();
  lcd.print("Pasw is changed");
  delay(1000);
  state1();

}

void changeBait() {
  i = 1;
  String pressedKey;
  lcd.clear();
  lcd.print("Enter New T");
  lcd.setCursor(0, 1);
  while (i < 5) {
    char key = keypad.getKey();
    if (key) {

      lcd.print("*");
      pressedKey = pressedKey + key;
      i++;
    }
  }
  paswBait = pressedKey;

  //  EEPROM.write(adressBaitPassword, paswBait);
  lcd.clear();
  lcd.print("Pasw is changed");
  delay(1000);
  state1();

}


void openMotorA() {
  lcd.clear();
  lcd.print("Sect A opening");
  digitalWrite(11, LOW);
  digitalWrite(10, HIGH);
  delay(973);
  digitalWrite(11, LOW);
  digitalWrite(10, LOW);
  lcd.clear();
  lcd.print("Sect A opened");
  delay(500);
  Status_MotorA = 1;
}

void openMotorB() {
  lcd.clear();
  lcd.print("Sect B opening");
  digitalWrite(9, LOW);
  digitalWrite(8, HIGH);
  delay(973);
  digitalWrite(9, LOW);
  digitalWrite(8, LOW);
  lcd.clear();
  lcd.print("Sect B opened");
  delay(500);
  Status_MotorB = 1;
}

void closeMotorA() {
  lcd.clear();
  lcd.print("Sect A locking");
  digitalWrite(10, LOW);
  digitalWrite(11, HIGH);
  delay(973);
  digitalWrite(11, LOW);
  digitalWrite(10, LOW);
  lcd.clear();
  lcd.print("Sect A locked");
  delay(500);
  Status_MotorA = 0;
}
void closeMotorB() {
  lcd.clear();
  lcd.print("Sect B locking");
  digitalWrite(8, LOW);
  digitalWrite(9, HIGH);
  delay(973);
  digitalWrite(9, LOW);
  digitalWrite(8, LOW);
  lcd.clear();
  lcd.print("Sect B locked");
  delay(500);
  Status_MotorB = 0;
}

void reportPrevious() {
  lcd.clear();
  //lcd.print(String(sucsesfullAttemps) + " Suc " + String(unsucsesfullAttemps) + " Unsuc");
  // I am getting the attemp counts from EEPROM
  lcd.print(String(EEPROM.read(adressSucsesfullAttemps)) + " Suc " +  String(EEPROM.read(adressUnsucsesfullAttemps)) + " Unsuc");
  delay(3000);
  state1();
}
