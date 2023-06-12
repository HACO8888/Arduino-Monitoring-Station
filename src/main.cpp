#include <EasyNextionLibrary.h>
#include <SoftwareSerial.h>
#include <SCD30.h>

EasyNex myNex(Serial2);

int CurrentAirTemp = 25;

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
}

void setup()
{
  Wire.begin();
  myNex.begin(9600);
  Serial.begin(9600);
  Serial2.begin(115200);
  scd30.initialize();
  myNex.writeStr("page 0");
}

void loop()
{
  myNex.NextionListen();
  if (myNex.currentPageId == 0)
  {
    SCD30dataClass SCD30data = SCD30sensor();
    String humiString = String(SCD30data.humid, 1);
    String tempString = String(SCD30data.temp, 1);
    String co2String = String(SCD30data.co2, 0);
    myNex.writeStr("Humi.txt", humiString + "%");
    myNex.writeStr("Temp.txt", tempString + "℃");
    myNex.writeStr("CO2.txt", co2String + " PPM");

    PMdataClass PMdata = PMsensor();
    String pm25String = String(PMdata.pm25, 0);
    myNex.writeStr("PM25.txt", co2String + " ug/m3");
  }
}