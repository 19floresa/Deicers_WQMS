/* ----- SD Stuff ----- */
#include <SPI.h>
#include <SD.h>

#define CHIP_SELECT 10
#define MAX_INT_SIZE 32767 /* max amount of files before program stops */

File file;
String filename = "";
int log_counter = 0;

/* ----- RTC Stuff ----- */

#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;
DateTime tc;

/* ----- Low Power Mode ----- */

#include "LowPower.h"

#define DS3231_I2C_ADDRESS 0x68 // might be different for other devices; use the built in Examples>SD>CardInfo to check i2c connection of device
#define RTC_INTERRUPT_PIN = 2;
#define SampleIntervalMinutes 30 // Whole numbers 1-30 only, must be a divisor of 60

volatile boolean clockInterrupt = false;  
//this flag is set to true when the RTC interrupt handler is executed

byte Alarmhour;
byte Alarmminute;
byte Alarmday;

/* ----- Sensor Stuff ----- */

#include "sensors.h"

/* ----- End of Libraries and Macros ----- */

void setup() {
  // Serial.begin(9600);
  // /* wait for Serial port to open for data transfer */
  // while (!Serial) { ; }

  if (!SD.begin(CHIP_SELECT)) {
    Serial.println(F("SD Failed!"));
    while (1);
  }
  new_filename();

  if (!rtc.begin()) {
    Serial.println(F("RTC Failed!"));
    while (1);
  }
  
  pinMode(CHIP_SELECT, OUTPUT); digitalWrite(CHIP_SELECT, HIGH);
  clearClockTrigger(); //stops RTC from holding the interrupt low if system reset just occured
  rtc.turnOffAlarm(1);

  /* sensor setup */
  setup_sensors();
	
}

void loop() {
  if (clockInterrupt) {
    if (rtc.checkIfAlarm(1)) rtc.turnOffAlarm(1); // Is the RTC alarm still on? Then turn it off.
    clockInterrupt = false;                       // reset the interrupt flag
  }
  
  while(1) { // open the next available file; fat32 has a 4gb file limit
    file = SD.open(filename, FILE_WRITE);
    
    if (file) break;
    else      new_filename();
  }
  
  String log_entry;
  String data = run_sensors();
  tc = rtc.now();
  log_entry  = get_datetime() + "," + data;
  //Serial.println(log_entry);


// Dont need rest of loop; rest of loop reads the content of a file
  file.println(log_entry);
  file.close();
  //delay(2000);

  // file = SD.open(filename);
  // //Serial.println(file.size());
  // if (file) {
  //   while (file.available()) {
  //     Serial.write(file.read());
  //   }
    
  //   file.close();
  // } 
  
//   SD.remove(filename);

// Serial.println("done");
// while(1) { ; }

Alarmhour = tc.hour();
Alarmminute = tc.minute() + SampleIntervalMinutes;
Alarmday = tc.day();

  // check for roll-overs
  if (Alarmminute > 59) { //error catching the 60 rollover!
    Alarmminute -= 60;
    Alarmhour = Alarmhour + 1;
    if (Alarmhour > 23) {
      Alarmhour = 0;
    }
  }
  
  rtc.setAlarm1Simple(Alarmhour, Alarmminute);
  rtc.turnOnAlarm(1);
  Serial.println();Serial.flush();

  attachInterrupt(0, rtcISR, LOW);
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_ON);
  detachInterrupt(0);
}

void new_filename() {
  filename = "log" + String(log_counter);
  log_counter++;
  
  if (log_counter == MAX_INT_SIZE) {
      Serial.println("No more files can be created");
      while(1) { ; }
    }
}

void remove_files() {
  int i;
  for (i = 0; i <= log_counter; i++) {
    String file = "log" + String(i);
    SD.remove(file);
  }
}

String get_datetime() {
  char date_time[20];
  sprintf(date_time,"%04d/%02d/%02d %02d:%02d", tc.year(), tc.month(), tc.day(), tc.hour(), tc.minute());
  return date_time;
}

/* All the code below comes from https://github.com/EKMallon/UNO-Breadboard-Datalogger/blob/master/_20160110_UnoBasedDataLogger_v1/_20160110_UnoBasedDataLogger_v1.ino */

// This is the Interrupt subroutine that only executes when the RTC alarm goes off
void rtcISR() {
    clockInterrupt = true;
  }

void clearClockTrigger()  // from  http://forum.arduino.cc/index.php?topic=109062.0
{
  byte bytebuffer1=0;
  Wire.beginTransmission(0x68);   //Tell devices on the bus we are talking to the DS3231
  Wire.write(0x0F);               //Tell the device which address we want to read or write
  Wire.endTransmission();         //Before you can write to and clear the alarm flag you have to read the flag first!
  Wire.requestFrom(0x68,1);       //Read one byte
  bytebuffer1=Wire.read();        //In this example we are not interest in actually using the bye
  Wire.beginTransmission(0x68);   //Tell devices on the bus we are talking to the DS3231 
  Wire.write(0x0F);               //Status Register: Bit 3: zero disables 32kHz, Bit 7: zero enables the main oscilator
  Wire.write(0b00000000);         //Write the byte.  //Bit1: zero clears Alarm 2 Flag (A2F), Bit 0: zero clears Alarm 1 Flag (A1F)
  Wire.endTransmission();
  clockInterrupt=false;           //Finally clear the flag we use to indicate the trigger occurred
}
