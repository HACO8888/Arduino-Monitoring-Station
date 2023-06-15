#include <EasyNextionLibrary.h>
#include <WiFiEsp.h>
#include <SCD30.h>

WiFiEspClient client;
EasyNex myNex(Serial1);

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

  // Setup SCD30
  Wire.begin();
  Serial.begin(115200);
  scd30.initialize();

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

  // String pm25String = String(PUT_DATA_HERE);
  // myNex.writeStr("PM25.txt", pm25String + " ug/m3");

  if (millis() - lastTime >= interval)
  {
    if (client.connect(server, 80))
    {
      Serial.println("Connecting to ThingSpeak...");
      client.print("GET /update?api_key=");
      client.print(apiKey);
      client.print("&field1=");
      client.print(SCD30data.temp);
      client.print("&field2=");
      client.print(SCD30data.humid);
      client.print("&field3=");
      client.println(SCD30data.co2);
    }
    else
    {
      Serial.println("Connection to ThingSpeak failed");
    }
    lastTime = millis();
  }
}