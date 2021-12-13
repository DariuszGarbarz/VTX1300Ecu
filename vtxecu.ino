//include library for lcd 16x2 screen
#include <LiquidCrystal.h>

//set up ports for lcd screen
const int rs = 12, en = 13, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//set up variables for speed calculation 
float start, finished;
float elapsed, time;
float circMetric=2.051558; //rear wheel - 170/80/15
float speed;    
float pulsespeed=6; // how many pulses for one turn

//set up variables for rpm calculation
float startrpm, finishedrpm;
float elapsedrpm, timerpm;
float rpm;
float pulserpm=2; // how many crankshaft turns per pulse

int modecount=1; //initialise mode selector

float ratio=1; //variable for gear selection

//selecting pins for inputs
const int clutch=8;
const int neutral=9;
const int mode=10;
const int relay=11;

int buttonState=0; //variable for input check

void setup()
{
// setting up interrupts
  attachInterrupt(digitalPinToInterrupt(2), speedCalc, RISING); 
  attachInterrupt(digitalPinToInterrupt(3), speedRpm, RISING);

  start=millis(); 
  startrpm=millis(); 

  modecount=1;
  // setting mode for pins
  pinMode(clutch, INPUT);
  pinMode(neutral, INPUT);
  pinMode(mode, INPUT);
  pinMode(relay, INPUT);

  //initialise lcd  
  lcd.begin(16, 2);
  lcd.print("HONDA-VTX1300");
  delay(2000); 			// print welcome message and wait 2 seconds
  lcd.setCursor(0,0);
  lcd.print("                "); // clear lcd
  
  
  Serial.begin(115200);
}
 
void speedCalc() // interrupt for speed calc
{
  if((millis()-start)>1) // debounce set for 1ms
    {
    elapsed=millis()-start;
    start=millis();

    speed=(3600*circMetric)/(elapsed*pulsespeed); //speed calculation based on time between interrupts
    }
}

void speedRpm() // interrupt for rpm calc
{
  if((millis()-startrpm)>1) // debounce set for 1ms
    {
    elapsedrpm=millis()-startrpm;
    startrpm=millis();
    
    rpm=(60000*pulserpm)/elapsedrpm; //rpm calculation based on time between interrupts
    }
}
 
void loop()
{
  //check mode button and react if pressed
  buttonState = digitalRead(mode); 
  if (buttonState == HIGH) {
  modecount++;
  delay(1000);  //should i set less? 1 second is a lot
  }
  
  //check if turn lights are blinking, if yes switch off the screen
  buttonState = digitalRead(relay); 
  if (buttonState == HIGH) {
  	lcd.setCursor(0,0);
  	lcd.print("                ");
  	lcd.setCursor(0,1);
  	lcd.print("                ");
  	delay(500); // how long it should stay off
  } else {
    // switch on screen placeholder for TM 1637 7 digit screen
  }
  
  //some fancy stuff
  lcd.setCursor(10,0);
  lcd.print("GEAR:");
  
  //Gear selection part
  buttonState = digitalRead(clutch); 
  if (buttonState == LOW) { //check if clutch is pressed, if yes - skip gear selection
  	buttonState = digitalRead(neutral);
  	if (buttonState == HIGH) { //if neutral is on, print N
  		lcd.setCursor(15,0);
  		lcd.print("N");
      //select and print gears based on rpm to speed ratio, fixed values for vtx1300
  	} else {
    	ratio=(rpm+0.1)/(speed+0.1);
      	if (ratio>=65){
     	lcd.setCursor(15,0);
  		lcd.print("1");
      }
      else if (ratio>=43 && ratio<65){
     	lcd.setCursor(15,0);
  		lcd.print("2");
      }
      else if (ratio>=35 && ratio<43){
     	lcd.setCursor(15,0);
  		lcd.print("3");
      }
      else if (ratio>=31 && ratio<35){
     	lcd.setCursor(15,0);
  		lcd.print("4");
      }
      else{
     	lcd.setCursor(15,0);
  		lcd.print("5");
      }
     
    }
  }
  
//mode selector based on modecount variable  
switch (modecount) {
  
	case 1: //by default RPM
  		lcd.setCursor(0,0);
  		lcd.print(int(rpm));
  		lcd.print("rpm   ");
  		lcd.setCursor(0,1);
  		lcd.print(int(speed));
  		lcd.print("km/h      ");
    	break;
  
  	case 2: //placeholder for SPEED
     	lcd.setCursor(0,0);
  		lcd.print("dariusz         ");
    	break;
  
  	case 3: //placeholder for Voltage
     	lcd.setCursor(0,0);
  		lcd.print("                ");
    	break;  
  
  	default:
    	modecount = 1; // reset counter if modecount exceed 3
    	break;
	} 
}