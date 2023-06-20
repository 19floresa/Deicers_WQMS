#include <Arduino.h>

#define TEMP_OUTPUT_PIN 8
#define TEMP_INPUT_PIN A5

#define PH_OUTPUT_PIN 7
#define PH_INPUT_PIN A4

void setup() {
	
	Serial.begin(9600);
	/* wait for Serial port to open for data transfer */
	while (!Serial) {;}
	
	pinMode(TEMP_OUTPUT_PIN, OUTPUT);
	pinMode(TEMP_INPUT_PIN, INPUT);
	
	//pinMode(PH_OUTPUT_PIN, OUTPUT);
	//pinMode(PH_INPUT_PIN, INPUT);
}

void loop() {
	
	PT1000Sensor();
	//pHSensor();
	
}

float sensorVoltage(uint8_t output_pin, uint8_t input_pin) {
	
	/* use 5v pin for a higher current */
	digitalWrite(output_pin, HIGH); 
	delay(1000);

	/* final_voltage = final_analog_value / max_analog_value * initial_voltage */
	float V = analogRead(input_pin) / 1024.0 * 5.0;

	digitalWrite(output_pin, LOW);

	return V;

}

void PT1000Sensor() {
 
	// might need to charge up for 8 seconds? 
	// documentation says Meter will output 0V-3V, so 3.0? So anything >3.0 is not supported.
	float V = sensorVoltage(TEMP_OUTPUT_PIN, TEMP_INPUT_PIN); // might want too loop so i can get an average

	/* Voltage to temperature (C) */
	float tempC = (V - 1.058) / .009;
	/* Temperature C to F */
	//float tempF = tempC * 1.8 + 32.0;

	/* Use either tempC or tempF, not both */
	Serial.print("PT ");
	Serial.println(tempC);
	//Serial.println(tempF);

}

void pHSensor() {
	
	float V = sensorVoltage(PH_OUTPUT_PIN, PH_INPUT_PIN);

	/* Voltage to pH value */
	float pH = (-5.6548 * V) + 15.509;

	Serial.print("PH ");
	Serial.println(pH);

}
