//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
// Thermometre
//-------------------------------------------------------------------------------
//
// This is a thermometer based in Arduino UNO R3 with a DHT22 sensor and a 16x2
// LCD screeen, also store the temperature and the humidity into the SD card.
//
//-------------------------------------------------------------------------------
//
// Author: Jorge Garcia Gonzalez
//
// GPL v3.0
//
//-------------------------------------------------------------------------------
#include <Time.h>
#include <TimeLib.h>
#include <SD.h>
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define I2C_ADDR    0x27  // I2C LCD screen direction.

#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321.
const int DHTPin = 2;  // DHT22 pin data input.
DHT dht(DHTPin, DHTTYPE);

//                    addr,     en,rw,rs,d4,d5,d6,d7
LiquidCrystal_I2C lcd(I2C_ADDR, 2, 1, 0, 4, 5, 6, 7);

// file descriptor.
File FileDes;

// SD constant.
const int SSPIN = 4;
// switch pin.
const int SWCHPIN = 3;

// counter to write the temperature into SD each minute.
int counter = 0;

void setup() {
  // configure serial port for debug pourposes.
  Serial.begin(9600);
  
  // LCD initialization data.
  lcd.begin (16, 2);  
  lcd.setBacklightPin(3, POSITIVE);
  lcd.setBacklight(HIGH);

  // Temperature sensor initialization.
  dht.begin();

  // pin where the reset pin is connected
  pinMode(SWCHPIN,INPUT);
  // SD pin configuration.
  pinMode(SSPIN, OUTPUT);

  // check if the SD is correct.
  if (!SD.begin(SSPIN)) {
    // alert by the LCD in case of SD error.
    lcd.home ();           // write in the first line.             
    lcd.print("SD ERROR");
    delay(2000);          // wait two seconds before clear the LCD.
    lcd.clear();
    return;
  }

  // open and create file to store the temperature and the humidity.
  FileDes = SD.open("data.txt", FILE_WRITE);

  // raise an error in case of there are a problem when open the file.
  if (!FileDes){
    lcd.home ();                 // write in the first line.         
    lcd.print("UNABLE TO OPEN");
    lcd.setCursor ( 0, 1 );      // go to the 2nd line.
    lcd.print("THE FILE");
    delay(2000);                 // wait two seconds before clear the LCD.
    lcd.clear();
  }

  // delete the logging file before to start to storage the temperature.
  if (SD.exists("data.txt")) {
    SD.remove("data.txt");
    lcd.home ();             // write in the first line.
    lcd.print("data.txt");
    lcd.setCursor ( 0, 1 );  // go to the 2nd line.
    lcd.print("deleted");
    delay(2000);            // wait two seconds before clear the LCD.
    lcd.clear();
  };

  // set origin of time.
  setTime(10,00,00,21,9,2017);
}

// function to reset the Arduino.
//declare reset function @ address 0.
void(* resetFunc) (void) = 0; 

void loop() {

  // string to compose the line to log in the file.
  String dataString = "";

  // take the time.
  time_t clk = now();
  
  // Reading temperature or humidity takes about 250 milliseconds!
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // check sensor readings.
  if (isnan(h) || isnan(t)) {
    lcd.home ();               // write in the first line.
    lcd.print("SENSOR ERROR");
    delay(2000);               // wait two seconds before clear the LCD.
    lcd.clear(); 
    return;
  }

  // output temperature on the LCD
  lcd.home ();             // write in the first line.
  lcd.print("Temp: ");
  lcd.setCursor ( 6, 0);
  lcd.print(t);
  lcd.setCursor ( 12, 0);
  lcd.print("C");
  lcd.setCursor ( 0, 1 );  // go to the 2nd line.
  lcd.print("Humidity: ");
  lcd.setCursor ( 9, 1 );
  lcd.print(h);
  lcd.setCursor ( 15, 1 );
  lcd.print("%");

  // write in the file each 60 seconds.
  // put 30 seconds i nthe counter due to the delay of two sec.
  // at the end of the loop.
  // this step it is not alwys the same, but it is not
  // necessary much more accuracy.
  if (counter >= 30){
    dataString = hour(clk);
    dataString +=":";
    dataString +=minute(clk);
    dataString +=":";
    dataString +=second(clk);
    dataString += ";";
    dataString += t;
    dataString += ";";
    dataString += h;
    FileDes.println(dataString); // store the line in the buffer file
    FileDes.flush();             // write the line in the file
    counter = 0;
  }else{
    counter += 1;
  }

  // due to I want to close the file before a reset, 
  // I put a new reset switch with some sentences associated.
  if (digitalRead(SWCHPIN)){
     FileDes.close();           // close data.txt file
     lcd.clear();
     lcd.home ();              // write in the first line.
     lcd.print("Reset... ");
     delay(5000);
     resetFunc();  
  }
  
  delay(2000);
  
}

