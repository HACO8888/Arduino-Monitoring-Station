#include <EasyNextionLibrary.h>
#include <SCD30.h>

EasyNex myNex(Serial1);

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

void setup()
{
  // Setup TDT
  myNex.begin(9600);

  // Setup SCD30
  Wire.begin();
  Serial.begin(115200);
  scd30.initialize();

  // Setup PM2
  Serial2.begin(9600);

  // Serial output setup
  Serial.begin(9600);
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

    // String pm25String = String(PUT_DATA_HERE);
    // myNex.writeStr("PM25.txt", pm25String + " ug/m3");
  }
}