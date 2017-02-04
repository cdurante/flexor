#include <Cmd.h>
#include <Wire.h>
#include <Encoder.h>
#include <LiquidCrystal_I2C.h>

#define stp 2
#define dir 3
#define MS1 4
#define MS2 5
#define EN  6
#define 
long BEND_DEGREES =180;
char* user_input;
char choice;
int param;
int state;

int cursor_col = 0;
int cursor_row = 0;
// initialize the library with the numbers of the interface pins
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

void steps(int arg_cnt, char **args) {
  uint16_t stps = atoi(args[1]);
  Serial.print("Rotating");
  rotateSteps(stps, 1);
}

void bendCycleTest(int arg_cnt, char **args) {
  int cycles = atoi(args[1]);
  long cysteps = (BEND_DEGREES*800)/360;
  Serial.println("Beginning Flex Cycle Test");
  Serial.print("Initial wire resistance: ");
  Serial.println(String(measureResistance()));
  for (int cycle = 1; cycle < cycles; cycle++) {
    bendWire(cysteps);
    unbendWire(cysteps);
//    Serial.print("Cycle ");
    Serial.print(cycle);
    Serial.print(", ");
    Serial.println(String(measureResistance()));
//    Serial.println(" Ohms");
  }
    resetEDPins();
}

void setup() {
  Serial.begin(57600); //Open Serial connection for debugging
  cmdInit(57600);
  cmdAdd("step", steps);
  cmdAdd("cycle", bendCycleTest);

  //Motor Stuff
  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(EN, OUTPUT);
  resetEDPins(); //Set step, direction, microstep and enable pins to default states
  //  Display Stuff
  lcd.begin (20, 4);
  // ------- Quick 3 blinks of backlight  -------------
  for (int i = 0; i < 3; i++) {
    lcd.backlight();
    delay(250);
    lcd.noBacklight();
    delay(250);
  }
  lcd.backlight(); // finish with backlight on
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wire Flex Tester");
  lcd.setCursor(2, 1);
}

void loop() {
  cmdPoll();
}

void rotateSteps(int steps, int direct) {
  digitalWrite(dir, direct);
  for (int y = 1; y < steps; y++)
  {
    digitalWrite(stp, HIGH); //Trigger one step
    delay(1);
    digitalWrite(stp, LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
  resetEDPins();
}

void bendWire(int cysteps) {
  rotateSteps(cysteps, 0);
}

void unbendWire(int cysteps) {
  rotateSteps(cysteps, 1);
}

float measureResistance() {
  delay(100);
  return 0.0;
}

//Reset Easy Driver pins to default states
void resetEDPins() {
  digitalWrite(stp, LOW);
  digitalWrite(dir, LOW);
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, LOW);
  digitalWrite(EN, HIGH);
}

void printMenu() {
  Serial.println("Enter motor control command");
  Serial.println("Command Examples");
  Serial.println("ex. step 100");
  Serial.println("ex. cycle 100");
  Serial.println();
}

