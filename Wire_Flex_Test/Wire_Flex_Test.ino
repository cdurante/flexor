#include <Cmd.h>
#include <Wire.h>
#include <Encoder.h>
#include <LiquidCrystal_I2C.h>
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <Bridge.h>
#include <BlynkSimpleYun.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "40fa285d34cf4c0c89f153b389a98b31";

#define stp 5
#define dir 4
#define MS1 8
#define MS2 6
#define EN  7

#define SENSE 10

long BEND_DEGREES = 300;
long blynk_degrees = 180;
float resistance;
char* user_input;
char choice;
int param;
int state;
int blynk_cycles = 0;
int blynk_button = 0;
int cursor_col = 0;
int cursor_row = 0;

// initialize the library with the numbers of the interface pins
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
WidgetLCD blcd(V0);

BLYNK_WRITE(V3)
{
  blynk_cycles = param.asInt(); // assigning incoming value from pin V3 to a variable
  Serial.print("V3 Slider value is: ");
  Serial.println(blynk_cycles);
}
BLYNK_WRITE(V2)
{
  blynk_degrees = (long)param.asInt(); // assigning incoming value from pin V2 to a variable
  Serial.print("V3 Slider value is: ");
  Serial.println(blynk_degrees);
}
BLYNK_WRITE(V1)
{
  blynk_button = param.asInt(); // assigning incoming value from pin V1 to a variable
  Serial.print("V1 Button value is: ");
  Serial.println(blynk_button);
}

void steps(int arg_cnt, char **args) {
  uint16_t stps = atoi(args[1]);
  Serial.print("Rotating");
  rotateSteps(stps, 1);
}

void bendCycleTest(int arg_cnt, char **args) {
  int cycles = atoi(args[1]);
  Serial.println(arg_cnt);
  Serial.println(**args);
  long cysteps = (BEND_DEGREES * 200) / 360;
  Serial.println("Beginning Flex Cycle Test");
  
  for (int cycle = 1; cycle < cycles; cycle++) {
    bendWire(cysteps);
    unbendWire(cysteps);
    Serial.print(cycle+1);
    Serial.print(", ");
    Serial.println(measureResistance());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Cycle:");
    lcd.setCursor(7, 0);
    lcd.print(cycle+1);
    lcd.setCursor(0, 1);
    lcd.print("Continuity?:");
    lcd.setCursor(12, 1);
    blcd.clear();
    blcd.print(0, 0, "Cycle:");
    blcd.print(7, 0, cycle);
    blcd.print(0, 1, "Continuity?:");
    if (measureResistance() == 0)
    {
      lcd.print("FALSE");
      blcd.print(12, 1, "FALSE");
      return;
    }
    lcd.print("TRUE");
    blcd.print(12, 1, "TRUE");
    Blynk.run();
    cycles = blynk_cycles;
  }
}

void blynkBendCycleTest(int arg_cnt, int cycles) {
  Serial.println("Beginning Flex Cycle Test");
  //  Serial.print("Initial wire resistance: ");
  //  Serial.println(String(measureResistance()));
  for (int cycle = 1; cycle <= cycles; cycle++) {
      long cysteps = (blynk_degrees * 200) / 360;
    while (blynk_button == 0) {
      Blynk.run();
    }
    bendWire(cysteps);
    unbendWire(cysteps);
    Serial.print(cycle+1);
    Serial.print(", ");
    Serial.println(measureResistance());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Cycle:");
    lcd.setCursor(7, 0);
    lcd.print(cycle+1);
    lcd.setCursor(0, 1);
    lcd.print("Continuity?:");
    lcd.setCursor(12, 1);

    blcd.clear();
    blcd.print(0, 0, "Cycle:");
    blcd.print(7, 0, cycle);
    blcd.print(0, 1, "Continuity?:");

    if (measureResistance() == 0)
    {
      lcd.print("FALSE");
      blcd.print(12, 1, "FALSE");
      break;
    }
    lcd.print("TRUE");
    blcd.print(12, 1, "TRUE");
    Blynk.run();
  }
  if (measureResistance() == 0) {
    blynk_button = 0;
    return;
  }
  blynk_button = 0;
}

void setup() {
  Serial.begin(57600); //Open Serial connection for debugging
  Blynk.begin(auth);
  cmdInit(57600);
  cmdAdd("step", steps);
  cmdAdd("cycle", bendCycleTest);

  pinMode(SENSE, INPUT);

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

  blcd.clear();
  blcd.print(0, 0, "Wire Flex Tester");

}

void loop() {
  Blynk.run();
  cmdPoll();
  if (blynk_button == 1) {
    blynkBendCycleTest(1, blynk_cycles);
  }
}

void rotateSteps(int steps, int direct) {
  digitalWrite(EN, LOW);
  digitalWrite(dir, direct);
  for (int y = 1; y < steps; y++)
  {
    digitalWrite(stp, HIGH); //Trigger one step
    delay(1);
    digitalWrite(stp, LOW); //Pull step pin low so it can be triggered again
    delay(5);
  }
}

void bendWire(int cysteps) {
  rotateSteps(cysteps, 0);
}

void unbendWire(int cysteps) {
  rotateSteps(cysteps, 1);
}

int measureResistance() {
  delay(100);
  //  Enable relay to send 5V
  //int adc = analogRead(RES);
  int sen = digitalRead(SENSE);
  //Serial.println(adc);
  //resistance = float(0.5*(1.0/((5.0/((adc*5.0/1024.0)/5.7))-1.0)));
  //digitalWrite(RELAY, HIGH);
  return sen;
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

