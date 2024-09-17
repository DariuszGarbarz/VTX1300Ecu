#include <TM1637Display.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

//Input Pins
const int RPM = 3;
const int CLK = 6;
const int DIO = 5;
const int Clutch = 8;
const int Neutral = 9;
const int Blinker = 11;

//Constants
const int RotationsPerPulse = 2;
const int MicroSeconds = 1000000;
const int SecondsPerMinute = 60;

//Variables
float PulseFrequency, SingleCycle, Hduration, Lduration;
int EngineSpeed, Gear, VechicleSpeed, Ratio;
String GpsMessage;

SoftwareSerial GpsSerial(0, 1);
TinyGPSPlus GPS;
TM1637Display Display(CLK, DIO);

void setup() {
  pinMode(A0, INPUT);
  pinMode(Clutch, INPUT);
  pinMode(Neutral, INPUT);
  pinMode(Blinker, INPUT);

  Serial.begin(9600);
  GpsSerial.begin(9600);

  Display.setBrightness(7);
}

void loop() {
  Serial.println("Reading vechicle speed");
  readSpeed();

  Serial.println("Calculating engine speed");
  calculateRpm();

  Serial.println("Blinkers check");
  blinkerWarning(digitalRead(Blinker) == HIGH);

  if (digitalRead(Clutch) == LOW) {
    Serial.println("Clutch released, selecting gear");
    selectGear(EngineSpeed, VechicleSpeed, digitalRead(Neutral));
  } else {
    Serial.println("Clutch pressed, skipping gear selection");
  }

  Serial.println("Printing engine speed");
  printRpm(EngineSpeed);

  Serial.println("Cooldown");
  delay(500);
}

void selectGear(int rpm, int speed, int neutralPin) {
  if (neutralPin == HIGH) {
    Serial.println("Neutral gear selected");
    Gear = 0;
  } else {
    Ratio = (rpm + 0.1) / (speed + 0.1);
    Serial.print("Rpm to Speed ratio: ");
    Serial.println(Ratio);
    if (Ratio >= 65) {
      Gear = 1;
    } else if (Ratio >= 43 && Ratio < 65) {
      Gear = 2;
    } else if (Ratio >= 35 && Ratio < 43) {
      Gear = 3;
    } else if (Ratio >= 31 && Ratio < 35) {
      Gear = 4;
    } else {
      Gear = 5;
    }
  }
  Serial.print("Gear selected: ");
  Serial.println(Gear);
  Serial.println("Printing gear");
  Display.showNumberDecEx(Gear, 0x80, false, 1, 0);
}

void printRpm(int rpm) {
  Display.showNumberDec((rpm / 1000) % 10, false, 1, 1);
  Display.showNumberDec((rpm / 100) % 10, false, 1, 2);
  Display.showNumberDec((rpm / 10) % 10, false, 1, 3);
}

void calculateRpm() {
  Hduration = pulseIn(A0, 1000);
  Serial.print("High pulse duration: ");
  Serial.print(Hduration);
  Serial.println(" us");
  Lduration = pulseIn(A0, LOW);
  Serial.print("Low pulse duration: ");
  Serial.print(Lduration);
  Serial.println(" us");
  SingleCycle = Hduration / MicroSeconds + Lduration / MicroSeconds;
  Serial.print("Single cycle duration: ");
  Serial.print(SingleCycle);
  Serial.println(" s");
  PulseFrequency = 1 / SingleCycle;
  Serial.print("Pulse frequency: ");
  Serial.print(PulseFrequency);
  Serial.println(" Hz");
  EngineSpeed = int(PulseFrequency * SecondsPerMinute * RotationsPerPulse);
  Serial.print("Engine speed: ");
  Serial.print(EngineSpeed);
  Serial.println(" rpm");
}

void blinkerWarning(int blinkerPin) {
  if (blinkerPin == HIGH) {
    Serial.println("Disabling display");
    Display.setBrightness(7, false);
  } else {
    Serial.println("Enabling display");
    Display.setBrightness(7, true);
  }
}

void readSpeed() {
  if (GpsSerial.available()) {
    Serial.println("GPS data available");
    while (GpsSerial.available()) {
      if (GPS.encode(GpsSerial.read())) {
        GpsMessage = GpsSerial.readStringUntil('\r');
        Serial.print("GPS message: "); Serial.println(GpsMessage);
        VechicleSpeed = int(GPS.speed.kmph());
        Serial.print("Vechicle speed: ");
        Serial.print(VechicleSpeed);
        Serial.println(" kmph");
      }
    }
  } else {
    Serial.println("GPS data unavailable");
  }
}