#include <OneWire.h>
#include <DallasTemperature.h>

#define HEAT_TEMP 25

#define RELAY_1 10
#define RELAY_2 11
#define RELAY_3 12
#define RELAY_4 13

// temperature

#define FLOW_METER_BUS_1 2
#define FLOW_METER_BUS_2 3

#define ONE_WIRE_BUS_1 4
#define ONE_WIRE_BUS_2 7
#define TEMPERATURE_PRECISION 9

OneWire oneWire1(ONE_WIRE_BUS_1);
OneWire oneWire2(ONE_WIRE_BUS_2);

DallasTemperature sensors1(&oneWire1);
DallasTemperature sensors2(&oneWire2);

DeviceAddress addresses1[2] = {
  { 0x28, 0xAA, 0x80, 0x16, 0x56, 0x14, 0x01, 0x54 },
  { 0x28, 0xAA, 0xF6, 0x2E, 0x56, 0x14, 0x01, 0xF1 }
};

DeviceAddress addresses2[2] = {
  { 0x28, 0xAA, 0xF9, 0xED, 0x3F, 0x14, 0x01, 0x04 },
  { 0x28, 0xAA, 0x7F, 0xBA, 0x56, 0x14, 0x01, 0x26 }
};

// flow

volatile int flow1_frequency;
volatile int flow2_frequency;

unsigned int l_hour1;
unsigned int l_hour2;

unsigned long currentTime;
unsigned long cloopTime1;
unsigned long cloopTime2;

void flow1 () {
   flow1_frequency++;
}

void flow2 () {
   flow2_frequency++;
}

// Pump 

#define PWM_PIN 5
#define START_DUTY_CYCLE 100

void setup(void) {

  pinMode(RELAY_1, OUTPUT);
  digitalWrite(RELAY_1, HIGH);
  pinMode(RELAY_2, OUTPUT);
  digitalWrite(RELAY_2, HIGH);
  pinMode(RELAY_3, OUTPUT);
  digitalWrite(RELAY_3, HIGH);
  pinMode(RELAY_4, OUTPUT);
  digitalWrite(RELAY_4, HIGH);
  
  Serial.begin(115200);
  Serial.println("Sensors deamo board starting...");

  // temperature
  sensors1.begin();
  sensors2.begin();

  Serial.print("Locating temperature sensors...");
  Serial.print("Found ");
  Serial.print(sensors1.getDeviceCount(), DEC);
  Serial.print(sensors2.getDeviceCount(), DEC);
  Serial.println(" devices");

  Serial.print("Temperature sensors parasite power is: ");
  if (sensors1.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
  if (sensors2.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  for (byte i = 0; i < sizeof(addresses1); i++) {
    if (!sensors1.getAddress(addresses1[i], i)) Serial.println("Unable to find address for temperature sensor " + i);

    Serial.print("Temperature sensor ");
    Serial.print(i);
    Serial.print(" address: ");
    printAddress(addresses1[i]);
    Serial.println();

    sensors1.setResolution(addresses1[i], TEMPERATURE_PRECISION);

    Serial.print("Temperature sensor ");
    Serial.print(i);
    Serial.print(" resolution: ");
    Serial.print(sensors1.getResolution(addresses1[i]), DEC);
    Serial.println();
    Serial.println("Temperature sensors OK");
  }

  for (byte i = 0; i < sizeof(addresses2); i++) {
    if (!sensors2.getAddress(addresses2[i], i)) Serial.println("Unable to find address for temperature sensor " + i);

    Serial.print("Temperature sensor ");
    Serial.print(i);
    Serial.print(" address: ");
    printAddress(addresses2[i]);
    Serial.println();

    sensors2.setResolution(addresses2[i], TEMPERATURE_PRECISION);

    Serial.print("Temperature sensor ");
    Serial.print(i);
    Serial.print(" resolution: ");
    Serial.print(sensors2.getResolution(addresses2[i]), DEC);
    Serial.println();
    Serial.println("Temperature sensors OK");
  }

  // flow

  Serial.println("Flow sensor setup...");
  pinMode(FLOW_METER_BUS_1, INPUT);
  pinMode(FLOW_METER_BUS_2, INPUT);
  digitalWrite(FLOW_METER_BUS_1, HIGH);
  digitalWrite(FLOW_METER_BUS_2, HIGH);
  attachInterrupt(0, flow1, RISING);
  attachInterrupt(1, flow2, RISING);
  sei();
  currentTime = millis();
  cloopTime1 = currentTime;
  cloopTime2 = currentTime;
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

void printTemperature(DallasTemperature sensor, DeviceAddress deviceAddress) {
  float tempC = sensor.getTempC(deviceAddress);
  Serial.print("Temp C: ");
  Serial.print(tempC);
}

void printResolution(DallasTemperature sensor, DeviceAddress deviceAddress) {
  Serial.print("Resolution: ");
  Serial.print(sensor.getResolution(deviceAddress));
  Serial.println();
}

void printData(DallasTemperature sensor, DeviceAddress deviceAddress) {
  Serial.print("Device Address: ");
  printAddress(deviceAddress);
  Serial.print(" ");
  printTemperature(sensor, deviceAddress);
  Serial.println();
}

void loop(void) {
  // PWM pump

  if (Serial.available()) {
    String line = Serial.readString();
    if (line.indexOf("PWM: ") >= 0) {
      String numberStr = line.substring(line.indexOf("PWM: "), line.length());
      int number = numberStr.toInt();
      if (number < 0) { number = 0; }
      if (number > 255) { number = 255; }
      analogWrite(number, PWM_PIN);
      Serial.print("Pump changed to ");
      Serial.print((float)number/255.0 * 100.0);
      Serial.print("% duty cycle OK");
    }
  }
  
  // flow
  boolean flag = false;
  currentTime = millis();
  if(currentTime >= (cloopTime1 + 1000))
  {
    cloopTime1 = currentTime;
    l_hour1 = (flow1_frequency * 60 / 7.5);
    flow1_frequency = 0;
    Serial.println("Flow 1 sensors data:");
    Serial.print(l_hour1, DEC);
    Serial.println(" L/hour");
    flag = true;
  }
  if(currentTime >= (cloopTime2 + 1000))
  {
    cloopTime2 = currentTime;
    l_hour2 = (flow2_frequency * 60 / 7.5);
    flow2_frequency = 0;
    Serial.println("Flow 2 sensors data:");
    Serial.print(l_hour2, DEC);
    Serial.println(" L/hour");
    flag = true;
  }
 
  // temperature
  if (flag) {
      sensors1.requestTemperatures();
      Serial.println("Temperature sensors data:");
      for (int i = 0; i < sizeof(addresses1); i++) {
        printData(sensors1, addresses1[i]);
      }
      sensors2.requestTemperatures();
      Serial.println("Temperature sensors data:");
      for (int i = 0; i < sizeof(addresses2); i++) {
        printData(sensors2, addresses2[i]);
      }
      flag = false;
  }
}
