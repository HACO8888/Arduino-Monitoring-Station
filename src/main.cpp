#include <EasyNextionLibrary.h>
#include <SoftwareSerial.h>
#include <SCD30.h>
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT11
#define GET_DATA_EVERY 2000
#define DATA_REFRESH_RATE 1000

EasyNex myNex(Serial1);
DHT dht(DHTPIN, DHTTYPE);

int CurrentAirTemp = 25;

void setup()
{
  dht.begin();
  myNex.begin(9600);
  Serial.begin(9600);
  myNex.writeStr("page 0");
}

void loop()
{
  myNex.NextionListen();
  if (myNex.currentPageId == 0)
  {
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    String humiString = String(humidity, 1);
    String tempString = String(temperature, 1);
    myNex.writeStr("Humi.txt", humiString + "%");
    myNex.writeStr("Temp.txt", tempString + "℃");
  }
}

class SCD30dataClass
{
public:
  float co2;
  float temp;
  float humid;
};

SCD30dataClass SCD30sensor()
{
  SCD30dataClass SCD30data;

  float result[3] = {0};

  if (scd30.isAvailable())
  {
    scd30.getCarbonDioxideConcentration(result);
    SCD30data.co2 = result[0];
    SCD30data.temp = result[1];
    SCD30data.humid = result[2];
  }

  return SCD30data;
}

class PMdataClass
{
public:
  int pm10;
  int pm25;
  int pm100;
};

PMdataClass PMsensor()
{
  PMdataClass PMdata;

  long pm10 = 0;
  long pm25 = 0;
  long pm100 = 0;

  int count = 0;

  unsigned char c;
  unsigned char high;

  while (Serial1.available())
  {
    c = Serial1.read();

    if ((count == 0 && c != 0x42) || (count == 1 && c != 0x4d))
    {
      Serial.println("check failed");
      break;
    }

    if (count > 15)
    {
      break;
    }
    else if (count == 4 || count == 6 || count == 8 || count == 10 || count == 12 || count == 14)
    {
      high = c;
    }
    else if (count == 5)
    {
      pm10 = 256 * high + c;
      PMdata.pm10 = pm10;
    }
    else if (count == 7)
    {
      pm25 = 256 * high + c;
      PMdata.pm25 = pm25;
    }
    else if (count == 9)
    {
      pm100 = 256 * high + c;
      PMdata.pm100 = pm100;
    }

    count++;
  }

  while (Serial1.available())
    Serial1.read();

  return PMdata;
