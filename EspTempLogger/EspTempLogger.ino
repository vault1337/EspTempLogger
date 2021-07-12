/*
The Developer Guy @ Vault 1337
Have fun!
*/



#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DHTPIN 2
#define DHTTYPE    DHT22

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define SCREEN_ADDRESS 0x3C

// One measurement per second
// Defaults to ~24 hour history on graph
#define FILTER_AMOUNT (24 * 60 * 60 / SCREEN_WIDTH)

void oledSetup(void);
void temperatureSetup(void);
void drawMeasurement(void);
void drawGraph(void);
void filterTemperature(void);
void addNewSample(float sample);

DHT_Unified dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

sensor_t sensor;
  
float temperature;
float humidity;

float tempGraph[SCREEN_WIDTH];
float filtered;
int filterCounter = 0;

void setup() {
  Serial.begin(115200);
  temperatureSetup();
  oledSetup();

  for(int i = 0; i < SCREEN_WIDTH; i++)
  {
    tempGraph[i] = 0.0;
  }

  filtered = 0.0;
  filterCounter = 0;
}

void loop() {
  sensors_event_t event;
  
  dht.temperature().getEvent(&event);
  if (!isnan(event.temperature)) 
  {
    temperature = event.temperature;
    filterTemperature();
  }
  
  dht.humidity().getEvent(&event);
  if (!isnan(event.relative_humidity))
  {
    humidity = event.relative_humidity;
  }

  drawMeasurement();
  drawGraph();  
  display.display();
  delay(1000);
}

void oledSetup(void)
{
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.cp437(true);
  display.clearDisplay();
  display.display();
}

void temperatureSetup(void)
{
  dht.begin();
  dht.temperature().getSensor(&sensor);
}

void drawMeasurement(void)
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(temperature, 1);
  display.print((char)248);
  display.print("C");
  display.setCursor(90, 0);
  display.print((int)humidity);
  display.print(F("%"));
  display.setCursor(40, 16);
}

void drawGraph(void)
{
  float minTemp = tempGraph[0];
  float maxTemp = tempGraph[0];
  
  for(int i = 1; i < SCREEN_WIDTH; i++)
  {
    if(minTemp > tempGraph[i])
    {
      minTemp = tempGraph[i];
    }
    if(maxTemp < tempGraph[i])
    {
      maxTemp = tempGraph[i];
    }
  }
  
  float diff = maxTemp - minTemp;
  float slope = 0;
  if(diff != 0.0)
  {
    slope = 15 / diff;
  }
  int previousPixelHeight = (tempGraph[0] - minTemp) * slope;
  for(int i = 1; i < SCREEN_WIDTH; i++)
  {
    int pixelHeight = (tempGraph[i] - minTemp) * slope;
    display.drawLine(i-1, 31 - previousPixelHeight, i, 31 - pixelHeight, SSD1306_WHITE);
    previousPixelHeight = pixelHeight;
  }
}

void filterTemperature(void)
{
  filtered += temperature;
  filterCounter++;
  if(filterCounter >= FILTER_AMOUNT)
  {
    addNewSample(filtered / filterCounter);
    filtered = 0;
    filterCounter = 0;
  }
}

void addNewSample(float sample)
{
  Serial.print("New filtered value: ");
  Serial.println(sample);
  for(int i = SCREEN_WIDTH-1; i > 0; i--)
  {
    tempGraph[i] = tempGraph[i-1];
  }
  tempGraph[0] = sample;
}
