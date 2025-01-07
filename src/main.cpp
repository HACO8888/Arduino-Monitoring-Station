#include <Adafruit_I2CDevice.h>
#include <SensirionI2CSen5x.h>
#include <SoftwareSerial.h>
#include <Nextion.h>
#include <secret.h>
#include <RTCLib.h>
#include <SCD30.h>
#include <SPI.h>
#include <SD.h>

// TDT components define
NexPage page0 = NexPage(0, 0, "Landing");
NexPage page1 = NexPage(1, 0, "Home");
NexPage page2 = NexPage(2, 0, "Monitor");
NexPage page3 = NexPage(3, 0, "Light");
NexPage page4 = NexPage(0, 0, "QRCode");
NexProgressBar j0 = NexProgressBar(0, 1, "j0");
NexText t0 = NexText(1, 1, "time");
NexText temp = NexText(2, 1, "temp");
NexText humid = NexText(2, 2, "humid");
NexText co2 = NexText(2, 3, "co");
NexText pm25 = NexText(2, 4, "pm");
NexSlider led0 = NexSlider(3, 1, "led0");

// Sensor and Modules define
RTC_DS3231 rtc;
SensirionI2CSen5x sen5x;

// Pin define
int LEDPin = 2;

// num define
uint32_t CurrentLED = 0;
int CurrentAirTemp = 25;

// SCD30 define
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
  nexInit();
  // led0.attachPop(led0PopCallback);

  for (int i = 5; i <= 100; i += 1)
  {
    j0.setValue(i);
    delay(50);
  }
  page1.show();

  // Setup SCD30 And Sen5x And RTC
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

  // Setup Pin
  pinMode(2, INPUT); // LED

  // Setup RTC
  if (!rtc.begin())
  {
    Serial.println("No DS3231 RTC found!");
  }

  if (rtc.lostPower())
  {
    Serial.println("RTC Reset！");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void loop()
{
  DateTime now = rtc.now();
  SCD30dataClass SCD30data = SCD30sensor();
  String humidString = String(SCD30data.humid, 1);
  String tempString = String(SCD30data.temp, 1);
  String co2String = String(SCD30data.co2, 0);

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

  // Send data to Nextion
  // const char *humid = humidString.c_str();
  // const char *temp = tempString.c_str();
  // const char *co2 = co2String.c_str();
  // const char *pm25 = pm25String.c_str();
  sendCommand(("humid.txt=\"" + humidString + "\"").c_str());
  sendCommand(("temp.txt=\"" + tempString + "\"").c_str());
  sendCommand(("co.txt=\"" + co2String + "\"").c_str());
  sendCommand(("pm.txt=\"" + pm25String + "\"").c_str());

  // Send time to Nextion
  // const char *time = now.toString("YYYY-MM-DD hh:mm:ss");
  // sendCommand("time.txt=\"" + now.toString("YYYY-MM-DD hh:mm:ss") + "\"");
  char timeBuffer[20];
  snprintf(timeBuffer, sizeof(timeBuffer), "%04d-%02d-%02d %02d:%02d:%02d",
           now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
  sendCommand(("time.txt=\"" + String(timeBuffer) + "\"").c_str());

  // Get LED value from Nextion
  led0.getValue(&CurrentLED);
  analogWrite(LEDPin, 255 - CurrentLED);
}