
// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID           "TMPLLljzArm4"
#define BLYNK_DEVICE_NAME           "Vertical Garden"
#define BLYNK_AUTH_TOKEN            "zhrpUfHFN_eTiTCI-FO6P04JBfDVcLlC"

// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <LiquidCrystal.h>      // Display
#include <EEPROM.h>             // TDS Sensor
#include <Ultrasonic.h>         // Distance Sensor
#include <OneWire.h>            // Temp Sensor
#include <DallasTemperature.h>  // Temp Sensor
#include <Blynk.h>              // Blynk Stuffs
#include <ESP8266WiFi.h>        // Blynk Stuffs
#include <TimeLib.h>
#include <WidgetRTC.h>

// Define os pinos para o trigger e echo
#define pino_trigger 12
#define pino_echo 14
Ultrasonic ultrasonic(pino_trigger, pino_echo);

// Define Temp Pin
#define ONE_WIRE_BUS 13

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "?";
char pass[] = "internet";

WidgetLED led1(V9); //Led for temp changing perc getting Cold
WidgetLED led2(V10); //Led for temp changing perc getting High
BlynkTimer timer;

// TDS Parameters
namespace pin {const byte tds_sensor = A0;}   // TDS Sensor 
namespace device {float aref = 5;}
namespace sensor {
  float ec = 0;
  unsigned int tds = 0;
  float ecCalibration = 1;
} 

// Temp Settings
OneWire oneWire(ONE_WIRE_BUS);
float tempMin = 999;
float tempMax = 0;
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1;

// RTC Settings
WidgetRTC rtc;

int NextHWater = 55; //Value to cover the scenario where the devide starts. It means
int MissingHWater = 0;
int MissingMWater = 0;

void clockDisplay()
{
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  Serial.print("Current time: ");
  Serial.println(currentTime);
  
  //NextHWater = hour()+4;
  if (NextHWater == hour() || NextHWater == 55 && ((MissingHWater * 60) - minute())){NextHWater = hour()+4;}
  if (NextHWater > 23){NextHWater = NextHWater-24;}

  MissingHWater = NextHWater - hour();
  if (NextHWater < 1){MissingMWater = 60 - minute();}
 
  // Send time to the App
  Blynk.virtualWrite(V5, "Next: " + String(NextHWater)+" Hrs //" + " Missing: " + String((MissingHWater * 60) - minute()) + " Min");
  Blynk.virtualWrite(V4, currentTime);

}

BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin();
}

int cmMsec, inMsec;
float aux_cmMsec, tempC, tempC_old=1, tempC_old_02, change_tmpC_perc, preview_tmp_1hr;

void myTimerEvent()
  {

  // TDS Sensor
  if(tempC < 0){tempC = 25;}else{tempC = tempC;} //Script to avoid temp sensor modificate TDS sensor
  double waterTemp = tempC;
  waterTemp  = waterTemp*0.0625; // conversion accuracy is 0.0625 / LSB
  
  float rawEc = analogRead(pin::tds_sensor) * device::aref / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
  float temperatureCoefficient = 1.0 + 0.02 * (waterTemp - 25.0); // temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
  
  sensor::ec = (rawEc / temperatureCoefficient) * sensor::ecCalibration; // temperature and calibration compensation
  sensor::tds = (133.42 * pow(sensor::ec, 3) - 255.86 * sensor::ec * sensor::ec + 857.39 * sensor::ec) * 0.5; //convert voltage value to tds value
  
  // Ultrasonic Sensor
  long microsec = ultrasonic.timing();
  aux_cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);
  inMsec = ultrasonic.convert(microsec, Ultrasonic::IN);

  Serial.print(" RAW_DIST: ");
  Serial.print(aux_cmMsec); Serial.print(" -- ");
  Serial.println("");

  aux_cmMsec = 40 - aux_cmMsec;
  aux_cmMsec = aux_cmMsec / 40;

  aux_cmMsec = (aux_cmMsec * 100) + 15;//20 is the value to consider the difference between the can and the water;
  
  //if (aux_cmMsec > 100) {aux_cmMsec = 100;} else {aux_cmMsec;};
  //if (aux_cmMsec < 0) {aux_cmMsec = 0;}     else {aux_cmMsec;};

  if (aux_cmMsec > 95){aux_cmMsec=100;} //The goal here is to keep values divisible by 5. E.g 5, 15, 20...
  else if (aux_cmMsec > 90){aux_cmMsec=95;}
  else if (aux_cmMsec > 85){aux_cmMsec=90;}
  else if (aux_cmMsec > 80){aux_cmMsec=85;}
  else if (aux_cmMsec > 75){aux_cmMsec=80;}
  else if (aux_cmMsec > 70){aux_cmMsec=75;}
  else if (aux_cmMsec > 65){aux_cmMsec=70;}
  else if (aux_cmMsec > 60){aux_cmMsec=65;}
  else if (aux_cmMsec > 55){aux_cmMsec=60;}
  else if (aux_cmMsec > 50){aux_cmMsec=55;}
  else if (aux_cmMsec > 45){aux_cmMsec=50;}
  else if (aux_cmMsec > 40){aux_cmMsec=45;}
  else if (aux_cmMsec > 35){aux_cmMsec=40;}
  else if (aux_cmMsec > 30){aux_cmMsec=35;}
  else if (aux_cmMsec > 25){aux_cmMsec=30;}
  else if (aux_cmMsec > 20){aux_cmMsec=25;}
  else if (aux_cmMsec > 15){aux_cmMsec=20;}
  else if (aux_cmMsec > 10){aux_cmMsec=15;}
  else if (aux_cmMsec > 5){aux_cmMsec=10;}
  else if (aux_cmMsec > 0){aux_cmMsec=5;}
  else if (aux_cmMsec < 5){aux_cmMsec=0;}

  // Temp Sensor
  sensors.requestTemperatures();
  tempC = sensors.getTempC(sensor1);
  if (tempC < tempMin)
  {
    tempMin = tempC;
  }
  if (tempC > tempMax)
  {
    tempMax = tempC;
  }

  Blynk.virtualWrite(V0, sensor::tds);
  Blynk.virtualWrite(V1, sensor::ec);
  Blynk.virtualWrite(V2, int(aux_cmMsec));
  Blynk.virtualWrite(V3, tempC);

  }

void changePerc()
{

  // Temp Sensor
  sensors.requestTemperatures();
  tempC = sensors.getTempC(sensor1);
  if (tempC < tempMin)
  {
    tempMin = tempC;
  }
  if (tempC > tempMax)
  {
    tempMax = tempC;
  }

  tempC_old_02 = tempC_old; // Aux variable to keep the previous temp

  change_tmpC_perc = 100*(tempC/tempC_old-1); //Percentage of change acoording to previous temp
  preview_tmp_1hr = tempC+((tempC/tempC_old)*60); //Expected temp after 1 hour
  tempC_old = tempC;
  if( change_tmpC_perc < 0 )
  {
    led1.on();
  } else {
    led1.off();
  }

  if( change_tmpC_perc > 0 ) // Condition to turn On the LED in during high temp
  {
    led2.on();
  } else {
    led2.off();
  }

  Blynk.virtualWrite(V6, change_tmpC_perc); //Percentage of the difference
  Blynk.virtualWrite(V7, tempC_old_02); //recorded temp. The reason why is here is to display the previous temp
  Blynk.virtualWrite(V8, tempC); //Current tmp
  Blynk.virtualWrite(V11, preview_tmp_1hr); 

  Serial.println("");
  Serial.println("");
  Serial.print("change_tmpC_perc: "); Serial.println(change_tmpC_perc);
  Serial.print("tempC_old: "); Serial.println(tempC_old);
  Serial.print("tempC: "); Serial.println(tempC);
  Serial.println("");
  Serial.println("");
}


void setup()
{
  // Debug console
  Serial.begin(115200);

  Blynk.begin(auth, ssid, pass);

  //Pin Settings
  pinMode(2, OUTPUT); //Pump Relay
  digitalWrite(2, HIGH); // Watering Pump On

    sensors.begin();
  // Localiza e mostra enderecos dos sensores
  Serial.println("Localizando sensores DS18B20...");
  Serial.print("Foram encontrados ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" sensores.");
  if (!sensors.getAddress(sensor1, 0)) 
     Serial.println("Sensores nao encontrados !"); 
  // Mostra o endereco do sensor encontrado no barramento
  Serial.print("Endereco sensor: ");
  mostra_endereco_sensor(sensor1);
  Serial.println();
  Serial.println();

  //RTC
  setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)

  // Display digital clock every 10 seconds
  timer.setInterval(10000L, clockDisplay);
  timer.setInterval(5000L, myTimerEvent);

  // Display changing Percentage
  timer.setInterval(60000L, changePerc); //5 min Interval
  
}

void mostra_endereco_sensor(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // Adiciona zeros se necessário
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void loop() {
  Blynk.run();
  timer.run();

  Serial.print(F("TDS:"));                    Serial.print(sensor::tds);
  Serial.print(F(" -//- EC:"));               Serial.print(sensor::ec, 2);
  Serial.print(F(" -//- TEMP:"));             Serial.print(tempC);
  Serial.print(F(" -//- NEXT WATER:"));       Serial.print(NextHWater);
  Serial.print(F(" -//- MISSING M WATER:"));  Serial.print(MissingHWater);
  Serial.print(F(" -//- RAW DIST: "));        Serial.print(cmMsec);
  Serial.print(F(" -//- DIST_final: "));      Serial.print(aux_cmMsec);
  
  Serial.println("");

}
