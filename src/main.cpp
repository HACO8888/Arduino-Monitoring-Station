#include <EasyNextionLibrary.h>
#include <SensirionI2CSen5x.h>
#include <WiFiEsp.h>
#include <secret.h>
#include <SCD30.h>

WiFiEspClient client;
EasyNex myNex(Serial1);
SensirionI2CSen5x sen5x;

int CurrentAirTemp = 25;
int status = WL_IDLE_STATUS;

char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASS;

char server[] = "api.thingspeak.com";
char apiKey[] = API_KEY;

unsigned long lastTime = 0;
unsigned long interval = 20000;

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

  // Setup SCD30 And Sen5x
  Wire.begin();
  Serial.begin(115200);
  scd30.initialize();
  sen5x.begin(Wire);
  sen5x.deviceReset();
  sen5x.startMeasurement();

  // Serial output setup
  Serial.begin(9600);

  // Setup PM2
  Serial2.begin(9600);

  // ESP-01S setup
  Serial3.begin(9600);
  WiFi.init(&Serial3);
  while (status != WL_CONNECTED)
  {
    status = WiFi.begin(ssid, pass);
  }
}

void loop()
{
  myNex.NextionListen();
  SCD30dataClass SCD30data = SCD30sensor();
  String humidString = String(SCD30data.humid, 1);
  String tempString = String(SCD30data.temp, 1);
  String co2String = String(SCD30data.co2, 0);
  myNex.writeStr("Humid.txt", humidString + "%");
  myNex.writeStr("Temp.txt", tempString + "℃");
  myNex.writeStr("CO2.txt", co2String + " PPM");

  float massConcentrationPm1p0;
  float massConcentrationPm2p5;
  float massConcentrationPm4p0;
  float massConcentrationPm10p0;
  float ambientHumidity;
  float ambientTemperature;
  float vocIndex;
  float noxIndex;

  sen5x.readMeasuredValues(massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0, massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex, noxIndex);

  String pm25String = String(massConcentrationPm2p5);
  myNex.writeStr("PM25.txt", pm25String + " ug/m3");

  if (millis() - lastTime >= interval)
  {
    if (client.connect(server, 80))
    {
      client.print("GET /update?api_key=");
      client.print(apiKey);
      client.print("&field1=");
      client.print(SCD30data.temp);
      client.print("&field2=");
      client.print(SCD30data.humid);
      client.print("&field3=");
      client.print(SCD30data.co2);
      client.print("&field4=");
      client.print(noxIndex);
      client.print("&field5=");
      client.print(vocIndex);
      client.print("&field6=");
      client.print(massConcentrationPm1p0);
      client.print("&field7=");
      client.print( massConcentrationPm2p5);
      client.print("&field8=");
      client.println(massConcentrationPm10p0);
    }
    client.stop();
    lastTime = millis();
  }
}