#include <Wire.h>
#include <SD.h>
#include "Adafruit_SGP30.h"

#define cardSelect 4
Adafruit_SGP30 sgp;
File logfile;
File errLog;
long secondTimer;



// blink out an error code
void error(uint8_t errno) {
  while(1) {
    uint8_t i;
    if ( errno != 1 ) {
      Serial.println("sensor not found");
    }
    for (i=0; i<errno; i++) {
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
    }
    for (i=errno; i<10; i++) {
      delay(200);
    }
  }
}


void setup() {
  Serial.begin(9600);
  Serial.println("SGP30 test");
  secondTimer = 0; // initialize the start time of the device in seconds

  if (! sgp.begin()){
    Serial.println("Sensor not found :(");
    //error(1);
  }
  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);

  // If you have a baseline measurement from before you can assign it to start, to 'self-calibrate'
  //sgp.setIAQBaseline(0x8E68, 0x8F41);  // Will vary for each sensor!


  Serial.println("Looking for SD Card");
  // see if the card is present and can be initialized:
  if (!SD.begin(cardSelect)) {
    Serial.println("Card init. failed!");
    error(2);
  }
  char filename[15];
  bool fileExists;
  strcpy(filename, "gas.csv");
  fileExists = SD.exists(filename);
  logfile = SD.open(filename, FILE_WRITE);
  errLog = SD.open("gas-err.log", FILE_WRITE);
  if ( !logfile ) {
    Serial.println("Could not create files on sd card");
    error(3);
  }

  // if new file print the csv header
  if ( !fileExists ) {
   logfile.println("seconds,tvoc-ppb,eco2-ppm,base-tvoc,base-eco2");  
  }
}

void logError(char msg[30]) {
  Serial.println(msg);
  errLog.print(secondTimer, DEC); errLog.print(":"); errLog.println(msg);
  errLog.flush();
}

void logData(uint16_t tvoc, uint16_t eco2, uint16_t baseTVOC, uint16_t baseECO2) {
  logfile.print(secondTimer); 
    logfile.print(",");
   logfile.print(tvoc);
    logfile.print(",");
   logfile.print(eco2);
    logfile.print(",");
   logfile.print(baseTVOC);
    logfile.print(",");
   logfile.println(baseECO2);
   logfile.flush();
}

int counter = 0;
void loop() {
  if (! sgp.IAQmeasure()) {
    //Serial.println("Measurement failed");
    logError("Measurement failed");

  if (! sgp.begin()){
    Serial.println("Sensor not found :(");
    //error(1);
  }
    
    //return;
  }

  uint16_t TVOC_base, eCO2_base, tvoc, eco2;
  TVOC_base = 0;
  eCO2_base = 0;
  tvoc = sgp.TVOC;
  eco2 = sgp.eCO2;


  counter++;
  if (counter == 30) {
    counter = 0;

    if (! sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
      logError("Failed to get baseline readings");
    }
    Serial.print("****Baseline values: eCO2: 0x"); Serial.print(eCO2_base, HEX);
    Serial.print(" & TVOC: 0x"); Serial.println(TVOC_base, HEX);
  }
  
  Serial.print("TVOC "); Serial.print(tvoc); Serial.print(" ppb\t");
  Serial.print("eCO2 "); Serial.print(eco2); Serial.println(" ppm");
  logData(tvoc, eco2, TVOC_base, eCO2_base);
  delay(1000);
  secondTimer += 1; // add 1 second to timer
}
