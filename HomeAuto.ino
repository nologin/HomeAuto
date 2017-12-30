
/* Pin Descriptors
Virtual Pins Used For sending Data to Server
   V1 - Fan Active 
   V2 - Fan Stopped
   V3 - Led Active
   V4 - Led Stopped
   V5 - Display Active
   V6 - Display Stop
   V7 - Temperature
   V8 - Humidity
   V9 - NTP Time
   V10 - Temp ESP1
   V11 - Hum ESP 1

Digital Pins For Sensors
  D3 - GPIO0 - DHT22 Sensor
  
Digital Pins For Reley Management
  D1 - GPIO5 - Fan Management /reley 1/
  D2 - GPIO4 - Led Management /reley 2/
*/
/* Define Colors */
#define BLYNK_GREEN     "#23C48E"
#define BLYNK_BLUE      "#04C0F8"
#define BLYNK_YELLOW    "#ED9D00"
#define BLYNK_RED       "#D3435C"
#define BLYNK_DARK_BLUE "#5F7CD8"

String gaugeColor;

#define BLYNK_PRINT Serial


#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <DHT_U.h>
#include <NTPClient.h>
#include <WiFiUdp.h>


#include <ArduinoJson.h>
#include <PubSubClient.h>
// Display
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <Fonts/FreeMono9pt7b.h>

#define TFT_CS 2
#define TFT_RST 14
#define TFT_DC   12
#define TFT_SCLK 15   // set these to be whatever pins you like!
#define TFT_MOSI 13   // set these to be whatever pins you like!
#define GREEN    0x07E0
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

#define TOKEN "YoutTokenHere"
char thingsboardServer[] = "blynk.energy-bg.org";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

#define DHTPIN 0
#define DHTTYPE DHT22
#define GPIO0_PIN 4
#define GPIO2_PIN 5

DHT dht(DHTPIN, DHTTYPE);
  float h;
  float hum_out;
  float t;
  float temp_out;

  float t_d;
  float h_d;
  float hum_out_d;
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "BlynkAuthToken";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "WiFi SSiD";
char pass[] = "SSID PAssword";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 10800, 60000);

  int prevState2 = -1;
  int currState2 = -1;
  long lastChangeTime2 = 0;
  int display_active;
  int display_stop;

  int prevState1 = -1;
  int currState1 = -1;
  long lastChangeTime1 = 0;
  int led_active;
  int led_stop;

  int prevState = -1;
  int currState = -1;
  long lastChangeTime = 0;
  int fan_active;
  int fan_stop;

SimpleTimer timer;
BLYNK_WRITE(V127) {
int gaugeValue = param.asInt();

  String newColor;
  if (gaugeValue > 80) {

    newColor = BLYNK_RED;
  } else if (gaugeValue > 50) {
    newColor = BLYNK_YELLOW;
  } else {
    newColor = BLYNK_GREEN;
  }

  // Send only if changed
  if (newColor != gaugeColor) {
    gaugeColor = newColor;
//    Blynk.setProperty(V3, "color", gaugeColor);

  }

//  Blynk.virtualWrite(V0, gaugeValue);
}

BLYNK_WRITE(V10)
{
  temp_out = param.asInt();
}
BLYNK_WRITE(V11)
{
  hum_out = param.asFloat(); 

}
void sendTemps()
{
BLYNK_WRITE(V10);
BLYNK_WRITE(V11);
  tft.setTextColor(ST7735_BLACK);
  tft.setFont(&FreeMono9pt7b);
  tft.setTextWrap(true);
  tft.setCursor(0, 26);
  tft.print(t_d);
//  tft.print("C");
  tft.setCursor(70, 26);
  tft.print(h_d);
  tft.setCursor(70, 39);
  tft.print(hum_out_d);
//  tft.println("%");
//  tft.println(timeClient.getFormattedTime());
  
  h = dht.readHumidity();
  t = dht.readTemperature();

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");

  String temperature = String(t);
  String humidity = String(h);
  String temperature_out = String(temp_out);
  String humidity_out = String(hum_out);
  
  // Prepare a JSON payload string
  String payload = "{";
  payload += "\"temperature\":"; payload += temperature; payload += ",";
  payload += "\"humidity\":"; payload += humidity; payload += ",";
  payload += "\"temperature_out\":"; payload += temperature_out; payload += ",";
  payload += "\"humidity_out\":"; payload += humidity_out;  
  payload += "}";

  // Send payload
  char attributes[100];
  payload.toCharArray( attributes, 100 );
  client.publish( "v1/devices/me/telemetry", attributes );
  Serial.println( attributes );
  

  Blynk.virtualWrite(V7, t);
  Blynk.virtualWrite(V8, h);
  Blynk.virtualWrite(V9, timeClient.getFormattedTime());
  

  int color = 0xF800;
  
  tft.setTextColor(ST7735_GREEN);
  tft.setFont(&FreeMono9pt7b);
  tft.setTextWrap(true);
  tft.setCursor(0, 26);
  tft.print(t);
//  tft.print("C");
  tft.setCursor(70, 26);
  tft.print(h);
  tft.setCursor(70, 39);
  tft.print(hum_out); 
//  tft.println(timeClient.getFormattedTime());
  t_d = t;
  h_d = h;
  hum_out_d = hum_out;
}


void checkPin()
{

  // Invert state, since button is "Active LOW"
  int state = !digitalRead(5);

  // Debounce mechanism
  long t = millis();
  if (state != prevState) {
    lastChangeTime = t;
  }
  if (t - lastChangeTime > 50) {
    if (state != currState) {
      currState = state;
      if (digitalRead(5)==HIGH) {
        fan_active = 255;
        fan_stop = 0;
      } else {
        fan_active = 0;
        fan_stop = 255;
      }
      Blynk.virtualWrite(V1, fan_active);
      Blynk.virtualWrite(V2, fan_stop);
    }
  }
  prevState = state;


  // Invert state, since button is "Active LOW"
  int state1 = !digitalRead(4);

  // Debounce mechanism
  long t1 = millis();
  if (state1 != prevState1) {
    lastChangeTime1 = t1;
  }
  if (t1 - lastChangeTime1 > 50) {
    if (state1 != currState1) {
      currState1 = state1;
      if (digitalRead(4)==HIGH) {
        led_active = 255;
        led_stop = 0;
      } else {
        led_active = 0;
        led_stop = 255;
      }
      Blynk.virtualWrite(V3, led_active);
      Blynk.virtualWrite(V4, led_stop);
    }
  }
  prevState1 = state1;

  // Invert state, since button is "Active LOW"
  int state2 = !digitalRead(16);

  // Debounce mechanism
  long t2 = millis();
  if (state2 != prevState2) {
    lastChangeTime2 = t2;
  }
  if (t2 - lastChangeTime2 > 50) {
    if (state2 != currState2) {
      currState2 = state2;
      if (digitalRead(16)==HIGH) {
        display_active = 255;
        display_stop = 0;
      } else {
        display_active = 0;
        display_stop = 255;
      }
      Blynk.virtualWrite(V5, display_active);
      Blynk.virtualWrite(V6, display_stop);
    }
  }
  prevState2 = state2;

}

void setup()
{
  // Debug console
  Serial.begin(115200);

  tft.initR(INITR_144GREENTAB);
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(1); 

  tft.setTextColor(ST7735_GREEN);
  tft.setFont(&FreeMono9pt7b);
  tft.setTextWrap(false);
  tft.setCursor(0, 10);
  tft.print(F("Temp"));
  tft.setCursor(80, 10);
  tft.print ("Hum");
  tft.drawFastHLine(0,13,128, GREEN);
  tft.drawFastHLine(0,14,128, GREEN);

  //Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);
  Blynk.begin(auth, ssid, pass, IPAddress(62,204,159,20), 8442);
  Serial.println("Connected");
  //setup MQTT
  client.setServer( thingsboardServer, 1883 );

//  checkPin();
 // checkPinLed();
  // Make pin 2 default HIGH, and attach INT to our handler
  timer.setInterval(5000L, sendTemps);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  timeClient.begin();
}

void loop()
{
    if (!client.connected()) {
    reconnect();
  }
  timeClient.update();
  Blynk.run();
  checkPin();
  timer.run();
  client.loop();
  
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    //if you MQTT broker has clientID,username and password
    //please change following line to    if (client.connect(clientId,userName,passWord))
//    if (client.connect(clientId, TOKEN, NULL))
    if ( client.connect("Esp8266 Master", TOKEN, NULL) )
    {
      Serial.println("connected");
     //once connected to MQTT broker, subscribe command if any
      client.subscribe("OsoyooCommand");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
} //end reconnect()

