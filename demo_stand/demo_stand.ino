#include <OneWire.h>
#include <DallasTemperature.h>

// temperature

#define ONE_WIRE_BUS 2
#define TEMPERATURE_PRECISION 9

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

DeviceAddress addresses[4] = {
  { 0x28, 0x1D, 0x39, 0x31, 0x2, 0x0, 0x0, 0xF0 },
  { 0x28, 0x1D, 0x39, 0x31, 0x2, 0x0, 0x0, 0xF0 },
  { 0x28, 0x1D, 0x39, 0x31, 0x2, 0x0, 0x0, 0xF0 },
  { 0x28, 0x1D, 0x39, 0x31, 0x2, 0x0, 0x0, 0xF0 }
};

// flow

volatile int flow_frequency;
unsigned int l_hour;
unsigned char flowsensor = 2;
unsigned long currentTime;
unsigned long cloopTime;

void flow () {
   flow_frequency++;
}

// Pump 

#define PWM_PIN 5
#define START_DUTY_CYCLE 100

void setup(void) {
  Serial.begin(115200);
  Serial.println("Sensors deamo board starting...");

  // temperature
  sensors.begin();

  Serial.print("Locating temperature sensors...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices");

  Serial.print("Temperature sensors parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  for (byte i = 0; i < sizeof(addresses); i++) {
    if (!sensors.getAddress(addresses[i], i)) Serial.println("Unable to find address for temperature sensor " + i);

    Serial.print("Temperature sensor ");
    Serial.print(i);
    Serial.print(" address: ");
    printAddress(addresses[i]);
    Serial.println();

    sensors.setResolution(addresses[i], TEMPERATURE_PRECISION);

    Serial.print("Temperature sensor ");
    Serial.print(i);
    Serial.print(" resolution: ");
    Serial.print(sensors.getResolution(addresses[i]), DEC);
    Serial.println();
    Serial.println("Temperature sensors OK");
  }

  // flow

  Serial.println("Flow sensor setup...");
  pinMode(flowsensor, INPUT);
  digitalWrite(flowsensor, HIGH);
  attachInterrupt(0, flow, RISING);
  sei();
  currentTime = millis();
  cloopTime = currentTime;
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

void printTemperature(DeviceAddress deviceAddress) {
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print("Temp C: ");
  Serial.print(tempC);
}

void printResolution(DeviceAddress deviceAddress) {
  Serial.print("Resolution: ");
  Serial.print(sensors.getResolution(deviceAddress));
  Serial.println();
}

void printData(DeviceAddress deviceAddress) {
  Serial.print("Device Address: ");
  printAddress(deviceAddress);
  Serial.print(" ");
  printTemperature(deviceAddress);
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

  currentTime = millis();
  if(currentTime >= (cloopTime + 1000))
  {
    cloopTime = currentTime;
    l_hour = (flow_frequency * 60 / 7.5);
    flow_frequency = 0;
    Serial.println("Flow sensors data:");
    Serial.print(l_hour, DEC);
    Serial.println(" L/hour");
     
    // temperature
  
    sensors.requestTemperatures();
    Serial.println("Temperature sensors data:");
    for (int i = 0; i < sizeof(addresses); i++) {
      printData(addresses[i]);
    }
  }
}
