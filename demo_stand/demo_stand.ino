#include <OneWire.h>
#include <DallasTemperature.h>

//------НАСТРОЙКИ. МОЖНО (И НУЖНО) МЕНЯТЬ---------
#define HEAT_LIMIT 20
#define START_DUTY_CYCLE 100


// heat
#define RELAY_1 10
#define RELAY_2 11

//fan
#define RELAY_3 12
#define RELAY_4 13

// temperature
#define ONE_WIRE_BUS_1 4
#define ONE_WIRE_BUS_2 7
#define TEMPERATURE_PRECISION 9

OneWire one_wire_1(ONE_WIRE_BUS_1);
OneWire one_wire_2(ONE_WIRE_BUS_2);

DallasTemperature temp_sensors_1(&one_wire_1);
DallasTemperature temp_sensors_2(&one_wire_2);

DeviceAddress temp_addresses_1[2] = {
  { 0x28, 0xAA, 0x80, 0x16, 0x56, 0x14, 0x01, 0x54 },
  { 0x28, 0xAA, 0xF6, 0x2E, 0x56, 0x14, 0x01, 0xF1 }
};

DeviceAddress temp_addresses_2[2] = {
  { 0x28, 0xAA, 0xF9, 0xED, 0x3F, 0x14, 0x01, 0x04 },
  { 0x28, 0xAA, 0x7F, 0xBA, 0x56, 0x14, 0x01, 0x26 }
};

// flow
#define FLOW_METER_BUS_1 2
#define FLOW_METER_BUS_2 3

volatile int flow_frequency_1;
volatile int flow_frequency_2;

unsigned int l_hour_1;
unsigned int l_hour_2;

unsigned long currentTime;
unsigned long cloop_time_1;
unsigned long cloop_time_2;

void flow_1 () {
   flow_frequency_1++;
}

void flow_2 () {
   flow_frequency_2++;
}

// pump 
#define PWM_PIN 5

void test_relay(void) {
  Serial.println("Testing relays...");
  for (int i = 0; i < 4; i++) {
    delay(500);
    digitalWrite(RELAY_1, LOW);
    digitalWrite(RELAY_2, LOW);
    digitalWrite(RELAY_3, LOW);
    digitalWrite(RELAY_4, LOW);
    delay(500);
    digitalWrite(RELAY_1, HIGH);
    digitalWrite(RELAY_2, HIGH);
    digitalWrite(RELAY_3, HIGH);
    digitalWrite(RELAY_4, HIGH);
  }
  Serial.println("Testing relays OK");
}

void setup(void) {
  Serial.begin(115200);
  Serial.println("Sensors deamo board starting...");
  
  Serial.println("Configuring relays...");
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);
  Serial.println("Configuring relays OK");
  test_relay();

  // temperature
  temp_sensors_1.begin();
  temp_sensors_2.begin();

  Serial.print("Locating temperature sensors...");
  Serial.print("Found ");
  Serial.print(temp_sensors_1.getDeviceCount(), DEC);
  Serial.print(temp_sensors_2.getDeviceCount(), DEC);
  Serial.println(" devices");

  Serial.print("Temperature sensors 1 parasite power is: ");
  if (temp_sensors_1.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  Serial.print("Temperature sensors 2 parasite power is: ");
  if (temp_sensors_2.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  for (int i = 0; i < 2; i++) {
    if (!temp_sensors_1.getAddress(temp_addresses_1[i], i)) {
      Serial.print("Unable to find address for temperature sensor ");
      printAddress(temp_addresses_1[i]);
      Serial.println();
    }

    Serial.print("Temperature sensor ");
    Serial.print(i);
    Serial.print(" address: ");
    printAddress(temp_addresses_1[i]);
    Serial.println();

    temp_sensors_1.setResolution(temp_addresses_1[i], TEMPERATURE_PRECISION);

    Serial.print("Temperature sensor ");
    Serial.print(i);
    Serial.print(" resolution: ");
    Serial.print(temp_sensors_1.getResolution(temp_addresses_1[i]), DEC);
    Serial.println();
    Serial.println("Temperature sensors 1 OK");
  }

  for (int i = 0; i < 2; i++) {
    if (!temp_sensors_2.getAddress(temp_addresses_2[i], i)) {
      Serial.print("Unable to find address for temperature sensor ");
      printAddress(temp_addresses_2[i]);
      Serial.println();
    }

    Serial.print("Temperature sensor ");
    Serial.print(i);
    Serial.print(" address: ");
    printAddress(temp_addresses_2[i]);
    Serial.println();

    temp_sensors_2.setResolution(temp_addresses_2[i], TEMPERATURE_PRECISION);

    Serial.print("Temperature sensor ");
    Serial.print(i);
    Serial.print(" resolution: ");
    Serial.print(temp_sensors_2.getResolution(temp_addresses_2[i]), DEC);
    Serial.println();
    Serial.println("Temperature sensors 2 OK");
  }

  // flow
  Serial.println("Flow sensor setup...");
  pinMode(FLOW_METER_BUS_1, INPUT);
  pinMode(FLOW_METER_BUS_2, INPUT);
  digitalWrite(FLOW_METER_BUS_1, HIGH);
  digitalWrite(FLOW_METER_BUS_2, HIGH);
  attachInterrupt(0, flow_1, RISING);
  attachInterrupt(1, flow_2, RISING);
  sei();
  currentTime = millis();
  cloop_time_1 = currentTime;
  cloop_time_2 = currentTime;
  Serial.println("Flow sensors OK");

  // Pump 
  Serial.println("Pump enabling...");
  analogWrite(START_DUTY_CYCLE, PWM_PIN);
  Serial.print("Pump enabled on ");
  Serial.print((float)START_DUTY_CYCLE/255.0 * 100.0);
  Serial.print("% duty cycle OK");
}

void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void printTemperature(DallasTemperature sensor, DeviceAddress deviceAddress, bool heat = false) {
  float tempC = sensor.getTempC(deviceAddress);
  if (heat) {
    if (tempC > HEAT_LIMIT) {
      digitalWrite(RELAY_1, LOW);
      digitalWrite(RELAY_2, LOW);
    } else {
      digitalWrite(RELAY_1, HIGH);
      digitalWrite(RELAY_2, HIGH);
    }
  }
  Serial.print("Temp C: ");
  Serial.print(tempC);
}

void printResolution(DallasTemperature sensor, DeviceAddress deviceAddress) {
  Serial.print("Resolution: ");
  Serial.print(sensor.getResolution(deviceAddress));
  Serial.println();
}

void printData(DallasTemperature sensor, DeviceAddress deviceAddress, bool heat = false) {
  Serial.print("Device Address: ");
  printAddress(deviceAddress);
  Serial.print(" ");
  printTemperature(sensor, deviceAddress, heat);
  Serial.println();
}

void loop(void) {
  if (Serial.available()) {
    String line = Serial.readString();
    if (line.indexOf("PWM: ") >= 0) {
      String numberStr = line.substring(line.indexOf(':') + 1);
      int number = numberStr.toInt();
      if (number < 0) { number = 0; }
      if (number > 255) { number = 255; }
      analogWrite(number, PWM_PIN);
      Serial.print("Pump changed to ");
      Serial.print((float)number/255.0 * 100.0);
      Serial.println("% duty cycle OK");
    }
    else if (line.indexOf("ON") >= 0) {
        digitalWrite(RELAY_3, LOW);
        digitalWrite(RELAY_4, LOW);
      }
     else if (line.indexOf("OFF") >= 0) {
      digitalWrite(RELAY_3, HIGH);
      digitalWrite(RELAY_4, HIGH);
     }
  }
  
  boolean flag = false;
  currentTime = millis();

  if(currentTime >= (cloop_time_1 + 1000))
  {
    cloop_time_1 = currentTime;
    l_hour_1 = (flow_frequency_1 * 60 / 7.5);
    flow_frequency_1 = 0;
    Serial.print("Flow 1 sensors data: ");
    Serial.print(l_hour_1, DEC);
    Serial.print(" L/hour");
    Serial.print((float)(l_hour_1 / 60 * 1000), DEC);
    Serial.println(" mL/min");
    flag = true;
  }

  if(currentTime >= (cloop_time_2 + 1000))
  {
    cloop_time_2 = currentTime;
    l_hour_2 = (flow_frequency_2 * 60 / 7.5);
    flow_frequency_2 = 0;
    Serial.print("Flow 2 sensors data: ");
    Serial.print(l_hour_2, DEC);
    Serial.print(" L/hour, ");
    Serial.print((float)(l_hour_2 / 60 * 1000), DEC);
    Serial.println(" mL/min");
    flag = true;
  }
 
  // temperature
  if (flag) {
      temp_sensors_1.requestTemperatures();
      Serial.println("Temperature sensors 1 data: ");
      for (int i = 0; i < 2; i++) {
        printData(temp_sensors_1, temp_addresses_1[i], i == 0);
      }

      temp_sensors_2.requestTemperatures();
      Serial.println("Temperature sensors 2 data: ");
      for (int i = 0; i < 2; i++) {
        printData(temp_sensors_2, temp_addresses_2[i], i == 0);
      }

      flag = false;
  }
}
