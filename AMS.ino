// Section 1
#include <SoftwareSerial.h>
#define RX 2
#define TX 3
String SSID = "ECE Department ";          //WiFi Name
String PASS = "20402040";         //WiFi PASSWORD
String API = "1PJ0V276PDAILE7O";   //ThingSpeak Key
String HOST = "api.thingspeak.com";
String PORT = "80";
int countTrueCommand;
int countTimeCommand; 
boolean found = false; 
SoftwareSerial esp8266(RX,TX); 



//Section 2

// Rain Sensor
#define rainPin A0
short rainVal;


//DHT
#include "DHT.h"
#define PULSE DHT22 
#define DHTPIN 4 
float temp;
float humi;
DHT dht(DHTPIN, PULSE);


//BMP280
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
Adafruit_BMP280 bmp;
float AirPressure;


//Dust Sensor
#define mPin A2
#define lPin 5
short sampTime=280;
short dTime=40;
short sTime=9680;
float vMeasure=0;


//UV Sensor
#define UVOUT A3
short uvLevel;
short refLevel;

int averageAnalogRead(int pinToRead)
{
  byte numberOfReadings = 8;
  unsigned int runningValue = 0; 
 
  for(int x = 0 ; x < numberOfReadings ; x++)
    runningValue += analogRead(pinToRead);
  runningValue /= numberOfReadings;
 
  return(runningValue);
}
 
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}



  
void setup() {
  Serial.begin(9600);
  esp8266.begin(115200);
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ SSID +"\",\""+ PASS +"\"",20,"OK");


  dht.begin();
  pinMode(rainPin, INPUT);
  pinMode(lPin, OUTPUT);
  pinMode(UVOUT, INPUT); 
  pinMode(mPin, INPUT);
  
  
  Serial.println("ML8511 example");

  Serial.println(F("BMP280 sensor test"));
  if (!bmp.begin(0x76)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,  
                  Adafruit_BMP280::SAMPLING_X2,  
                  Adafruit_BMP280::SAMPLING_X16,   
                  Adafruit_BMP280::FILTER_X16,     
                  Adafruit_BMP280::STANDBY_MS_500);
  

}

void loop() {
  
 String getData = "GET /update?api_key="+ API +"&field1="+getrainValue()+"&field2="+gettemperetureValue()
                  +"&field3="+gethumidityValue()+"&field4="+getairPressureValue()+"&field5="+getdustValue()
                  +"&field6="+getuvValue();
 sendCommand("AT+CIPMUX=1",5,"OK");
 sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
 sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
 esp8266.println(getData);
 delay(1000); 
 countTrueCommand++;
 sendCommand("AT+CIPCLOSE=0",5,"OK");
  
}

String getuvValue(){
  uvLevel = averageAnalogRead(UVOUT);
  refLevel = 670;  
  //float outputVoltage = 3.3 / refLevel * uvLevel;
 // float uvIntensity = mapfloat(outputVoltage, 0.99, 2.8, 0.0, 15.0);

  Serial.print(uvLevel);
  Serial.print("  ");

  
  return String(uvLevel);
  }

String getdustValue(){
  digitalWrite(lPin, LOW);
  delayMicroseconds(sampTime);
  vMeasure= analogRead(mPin);
  delayMicroseconds(dTime);
  digitalWrite(lPin, HIGH);
  delayMicroseconds(sTime);

  Serial.print(vMeasure);
  Serial.print("  ");

  return String(vMeasure);
  }

String getairPressureValue(){
  AirPressure= bmp.readPressure() / 100.0F;

  Serial.print(AirPressure);
  Serial.print("  ");  

      return String(AirPressure);
  }
  
String gettemperetureValue(){
  temp= dht.readTemperature();

  if(isnan(temp)){
    Serial.print("Failed...");
    return;    
    }

  Serial.print(temp);
  Serial.print("  ");
  
    return String(temp);
  }

String gethumidityValue(){
  humi= dht.readHumidity();

  if(isnan(humi)){
    Serial.print("Failed...");
    return;    
    }
  Serial.print(humi);
  Serial.print("  ");

    return String(humi);
  }

String getrainValue(){
  rainVal= analogRead(rainPin);
  Serial.print(rainVal);
  Serial.print("  ");
  
  return String(rainVal);
  }

void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);         //at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  if(found == true)
  {
    Serial.println("Ok");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  
  found = false;
 }


 
