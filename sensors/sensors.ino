#include <Arduino.h>

#include "rtd_grav.h"
#include "ph_grav.h"  
#include <OneWire.h>
#include <DallasTemperature.h>   

#include <EEPROM.h>
#include "GravityTDS.h"           

/* -------------------------------------------- */ 

#define TEMP_OUTPUT_PIN 1 // 8
#define TEMP_INPUT_PIN A0

Gravity_RTD RTD = Gravity_RTD(TEMP_INPUT_PIN);

/* -------------------------------------------- */

#define PH_OUTPUT_PIN 7
#define PH_INPUT_PIN A3

Gravity_pH pH = Gravity_pH(PH_INPUT_PIN);

/* -------------------------------------------- */
 
#define TURB_OUTPUT_PIN 5 // 3
#define TURB_INPUT_PIN A1

/* -------------------------------------------- */

#define TDS_OUTPUT_PIN 6
#define TDS_INPUT_PIN A2 //A5

GravityTDS gravityTds;
float temperature = 25,tdsValue = 0;

/* -------------------------------------------- */

#define DS_OUTPUT_PIN 9 // 5 
#define ONE_WIRE 8 // 4

OneWire one_wire(ONE_WIRE);
DallasTemperature ds_sensor(&one_wire);

/* -------------------------------------------- */


void setup() {
	
	Serial.begin(9600);
	/* wait for Serial port to open for data transfer */
	while (!Serial) {;}

	/* load calibration from EEPROM */
	
	if (RTD.begin()) Serial.println("Loaded rtd EEPROM");
	if (pH.begin()) Serial.println("Loaded pH EEPROM");
	
	/* digital output pinMode need to be set, can prob ignore anything else */
	//pinMode(TEMP_OUTPUT_PIN, OUTPUT);
	pinMode(TEMP_INPUT_PIN, INPUT);
	pinMode(PH_OUTPUT_PIN, OUTPUT);
	pinMode(PH_INPUT_PIN, INPUT);
	pinMode(TURB_OUTPUT_PIN, OUTPUT);
	pinMode(TURB_INPUT_PIN, INPUT);
	
	pinMode(DS_OUTPUT_PIN, OUTPUT);
	pinMode(ONE_WIRE, INPUT);
  	ds_sensor.begin();

  	pinMode(TDS_OUTPUT_PIN, OUTPUT);
  	pinMode(TDS_INPUT_PIN, INPUT);

  	gravityTds.setPin(TDS_INPUT_PIN);
    gravityTds.setAref(5.0);  
    gravityTds.setAdcRange(1024);  
    gravityTds.begin();  
	
}

void loop() {
	Serial.println("-----START-----");
	PT1000Sensor();
	pHSensor();
	ds18b20Sensor();
	turbSensor();
	TDSSensor();
	Serial.println("------END------");
}


float sensorVoltage(uint8_t output_pin, uint8_t input_pin) {
	
	/* use 5v pin for a higher current */
	digitalWrite(output_pin, HIGH); 
	delay(800);

	/* final_voltage = final_analog_value / max_analog_value * initial_voltage */
	float V = ( analogRead(input_pin) / 1024.0 ) * 5.0;

	digitalWrite(output_pin, LOW);

	return V;

}

void PT1000Sensor() {

	// digitalWrite(TEMP_OUTPUT_PIN, HIGH); 
	// delay(800);

	/* Voltage to temperature (C) */
	float tempC = RTD.read_RTD_temp_C();
	/* Temperature C to F */
	//float tempF = tempC * 1.8 + 32.0;

	/* Use either tempC or tempF, not both */
	Serial.print("PT ");
	Serial.println(tempC);
	

	// digitalWrite(TEMP_OUTPUT_PIN, LOW); 
	// delay(100);
}

void pHSensor() {
	
	digitalWrite(PH_OUTPUT_PIN, HIGH);
	delay(800);
	
	float pH_value = pH.read_ph();

	Serial.print("PH ");
	Serial.println(pH_value);

	digitalWrite(PH_OUTPUT_PIN, LOW); 
	delay(100);
}

void ds18b20Sensor() {
	/* need output pin */

	digitalWrite(DS_OUTPUT_PIN, HIGH);
	delay(800);

	ds_sensor.requestTemperatures();
	delay(1000);
	
	Serial.print("DS18B20 ");
	temperature = ds_sensor.getTempCByIndex(0);
  	Serial.println(temperature);

  	digitalWrite(DS_OUTPUT_PIN, LOW);  	
  	delay(100);
}

void turbSensor() {
	float V = sensorVoltage(TURB_OUTPUT_PIN, TURB_INPUT_PIN);

	/* formula from DFROBOT */
	float turbidity = ( -1120.4 * V * V )  + ( 5742.3 * V ) - 4352.9;
	//Serial.println(V);

	Serial.print("TURB ");
	Serial.println(turbidity);

	delay(800);
}

void TDSSensor() {

	digitalWrite(TDS_OUTPUT_PIN, HIGH);
	delay(800);

	gravityTds.setTemperature(temperature); 
    gravityTds.update(); 
    tdsValue = gravityTds.getTdsValue();
    Serial.print("TDS ");
    Serial.println(tdsValue, 0);
    
    digitalWrite(TDS_OUTPUT_PIN, LOW);
    delay(800);
}

