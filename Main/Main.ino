//ESP 32 Weather Checker

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <ArduinoJson.h>


//Button IO
#define BUTTONPIN 0

//PRIVATE INFORMATION
const char* ssid     = "";     // your network SSID (name of wifi network)
const char* password = ""; // your network password
const char* ApiKey = ""; //Your API Key here

//HTTPS Calls
WiFiClientSecure client;
const char* endpoint = "https://api.openweathermap.org/data/2.5/weather?id=2654675&&units=metric&appid=";
//const char* endpoint = "https://api.openweathermap.org/data/2.5/forecast?id=2654675&units=metric&cnt=3&APPID=";
const char*  server = "api.openweathermap.org";  // Server URL

//Display Set Up
#define SCL 15
#define SDA 4
#define RESET 16

#define WEATHERHEIGHT 20
#define WEATHERWIDTH 0

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL, SDA, RESET);

// https://openweathermap.org

const char* test_root_ca = \
                           "" ; //Your Auth ticket here

#define UPDATETIME 300000

long LastRequest = 30;

//Weather Values
String OverallWeather;

int weather_0_id;
const char* weather_0_main;
const char* weather_0_description;
float main_temp;
float main_feels_like;

long sys_sunrise;
long sys_sunset;

void setup()
{
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  delay(100);

  u8g2.begin();

  //Button IO Setup
  pinMode(BUTTONPIN, INPUT);

  //Intro Animation
  IntroAnimation(); //for style points


}

void loop()
{
  //Checks the last time we got fresh data
  //(digitalRead(BUTTONPIN) == LOW)

  if ((millis() > LastRequest) || (digitalRead(BUTTONPIN) == LOW)) //If timed out get new data or if button pressed
  {
    WifiConnection();
    GetRequest();

    LastRequest = millis() + UPDATETIME;
    
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_logisoso18_tf);
    if(OverallWeather.length() < 9)
    {
    char OutputBuffer [(OverallWeather.length() + 1)];
    OverallWeather.toCharArray(OutputBuffer, (OverallWeather.length() + 1));

    u8g2.drawStr(WEATHERWIDTH, (21 * 1), OutputBuffer);
    
    }
    else
    {
      byte WhitespacePos = 0;
      for(byte i = 0; i < OverallWeather.length(); i++)
      {
       if(isWhitespace(OverallWeather[i]))
       {
        //Whitespace has been found
        WhitespacePos = i;
        break;
       }
      }
      if (WhitespacePos != 0)
      {
        char FirstLineBuffer [10];
        //first line
        String AdjFirstBuffer = OverallWeather.substring(0,(WhitespacePos));
        AdjFirstBuffer.toCharArray(FirstLineBuffer, (AdjFirstBuffer.length() + 1));

        u8g2.drawStr(WEATHERWIDTH, (21 * 1), FirstLineBuffer);
        
        //second line
        char SecondLineBuffer [WhitespacePos + 1];
        String AdjSecondBuffer = OverallWeather.substring(WhitespacePos,OverallWeather.length());
        AdjSecondBuffer.toCharArray(SecondLineBuffer, (AdjSecondBuffer.length() + 1));

        u8g2.drawStr(WEATHERWIDTH, (21 * 2), SecondLineBuffer);
      }
      else
      {
        //No whitespace has been found so just got 8 in and place a dash
        
        char FirstLineBuffer [9];
        //first line
        String FirstBuffer = OverallWeather.substring(0,7); 
        FirstBuffer += '-'; //Adds the dash on to the end
        FirstBuffer.toCharArray(FirstLineBuffer, (FirstBuffer.length() + 1));
        
        //second line
        char SecondLineBuffer [10];
        String SecondBuffer = OverallWeather.substring(8,OverallWeather.length());
        SecondBuffer.toCharArray(SecondLineBuffer, SecondBuffer.length());

        u8g2.drawStr(WEATHERWIDTH, (21 * 2), SecondLineBuffer);
      }
     
    }

    char Temperature [10];

    dtostrf(main_temp, 3, 2, Temperature);

    u8g2.drawStr(20, (21 * 3), Temperature);

    u8g2.sendBuffer();
  }
}

void IntroAnimation() //Intro Just for Style points
{
  long Introtimer = (millis() + 5000);
  long WaitTimer = (millis() + 300);

  u8g2.setFont(u8g2_font_maniac_te);

  while ((millis() < Introtimer) && (digitalRead(BUTTONPIN) == HIGH))
  {

    u8g2.clearBuffer();
    u8g2.drawStr(WEATHERWIDTH, (23 * 1), "Weather");
    u8g2.drawStr(WEATHERWIDTH, (23 * 2), "rWeathe");
    u8g2.drawStr(WEATHERWIDTH, (23 * 3), "atherWe");
    u8g2.sendBuffer();

    WaitTimer = (millis() + 300);

    while ((millis() < WaitTimer) && (digitalRead(BUTTONPIN) == HIGH)) {}

    u8g2.clearBuffer();
    u8g2.drawStr(WEATHERWIDTH, (23 * 1), "atherWe");
    u8g2.drawStr(WEATHERWIDTH, (23 * 2), "Weather");
    u8g2.drawStr(WEATHERWIDTH, (23 * 3), "atherWe");
    u8g2.sendBuffer();

    WaitTimer = (millis() + 300);
    while ((millis() < WaitTimer) && (digitalRead(BUTTONPIN) == HIGH)) {}

    u8g2.clearBuffer();
    u8g2.drawStr(WEATHERWIDTH, (23 * 1), "herWeat");
    u8g2.drawStr(WEATHERWIDTH, (23 * 2), "atherWe");
    u8g2.drawStr(WEATHERWIDTH, (23 * 3), "rWeathe");
    u8g2.sendBuffer();

    WaitTimer = (millis() + 300);
    while ((millis() < WaitTimer) && (digitalRead(BUTTONPIN) == HIGH)) {}

    u8g2.clearBuffer();
    u8g2.drawStr(WEATHERWIDTH, (23 * 1), "rWeathe");
    u8g2.drawStr(WEATHERWIDTH, (23 * 2), "herWeat");
    u8g2.drawStr(WEATHERWIDTH, (23 * 3), "Weather");
    u8g2.sendBuffer();

    WaitTimer = (millis() + 300);
    while ((millis() < WaitTimer) && (digitalRead(BUTTONPIN) == HIGH)) {}
  }
}

void GetRequest()
{
  Serial.println("Checking the CACert");

  client.setCACert(test_root_ca);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_CursivePixel_tr);
  u8g2.drawStr(WEATHERWIDTH, 20, "Getting");
  u8g2.drawStr(WEATHERWIDTH, 40, "Weather");
  u8g2.sendBuffer();

  if (!client.connect(server, 443))
  {
    Serial.println("No HTTPS Connection has been made!");

  }
  else
  {

    Serial.println("Connected to server!");

    // Make a HTTP request:
    String HttpRequest = "GET ";
    HttpRequest.concat(endpoint);
    HttpRequest.concat(ApiKey);

    //Sends the get request
    Serial.println("GET Request sent");
    client.println(HttpRequest);

    //Gets the api call and returns it
    String Payload = client.readString();

    //Data has been fetched close connection
    client.println("Connection: close");
    client.println();
    client.stop();

    Serial.println(Payload);

    Serial.println("Passing Payload into function");

    char CharBuffer [(Payload.length() + 1)];

    Payload.toCharArray(CharBuffer, (Payload.length() + 1));
    SetValues(CharBuffer);
  }
}

void Output() //Outputs
{
  //Print the weather data to the screen
  //weather_0_id = 210;
  switch (weather_0_id)
  {
    case 200:
      OverallWeather = "Light Storm"; //Storms
      break;
    case 201:
      OverallWeather = "Storm";
      break;
    case 210:
      OverallWeather = "Light Storm";
      break;
    case 211:
      OverallWeather = "Storm";
      break;
    case 212:
      OverallWeather = "Heavy Storm";
      break;
    case 230:
      OverallWeather = "Light Storm";
      break;
    case 231:
      OverallWeather = "Storm";
      break;
    case 232:
      OverallWeather = "Bad";
      break;
    case 300:
      OverallWeather = "Light Drizzle"; //Rain / Drizzle
      break;
    case 301:
      OverallWeather = "Drizzle";
      break;
    case 302:
      OverallWeather = "Spitting";
      break;
    case 311:
      OverallWeather = "Rain";
      break;
    case 312:
      OverallWeather = "Showers";
      break;
    case 313:
      OverallWeather = "Rainy Drizzle";
      break;
    case 314:
      OverallWeather = "Heavy Showers";
      break;
    case 321:
      OverallWeather = "Showers";
      break;
    case 500:
      OverallWeather = "Light Rain";
      break;
    case 501:
      OverallWeather = "Rain";
      break;
    case 502:
      OverallWeather = "Heavy Rain";
      break;
    case 503:
      OverallWeather = "Heavy Rain";
      break;
    case 504:
      OverallWeather = "HEAVY RAIN";
      break;
    case 511:
      OverallWeather = "Freezing Rain";
      break;
    case 520:
      OverallWeather = "Showers";
      break;
    case 521:
      OverallWeather = "Rain Showers";
      break;
    case 531:
      OverallWeather = "Showers";
      break;
    case 600:
      OverallWeather = "Light Snow"; //Snow
      break;
    case 601:
      OverallWeather = "Snow!"; //Snow
      break;
    case 602:
      OverallWeather = "Heavy Snow";
      break;
    case 611:
      OverallWeather = "Sleet";
      break;
    case 612:
      OverallWeather = "Sleety Snow";
      break;
    case 613:
      OverallWeather = "Sleet Showers";
      break;
    case 615:
      OverallWeather = "Rainy Sleet";
      break;
    case 616:
      OverallWeather = "Rainy Sleet";
      break;
    case 620:
      OverallWeather = "Snow Showers";
      break;
    case 621:
      OverallWeather = "Shower Snow";
      break;
    case 622:
      OverallWeather = "Heavy Snow";
      break;
    case 701:
      OverallWeather = "Mist"; //Atmospheric
      break;
    case 711:
      OverallWeather = "Smoke";
      break;
    case 721:
      OverallWeather = "Haze";
      break;
    case 731:
      OverallWeather = "Dust";
      break;
    case 741:
      OverallWeather = "Fog";
      break;
    case 751:
      OverallWeather = "Sand?";
      break;
    case 761:
      OverallWeather = "Dust";
      break;
    case 762:
      OverallWeather = "Ash";
      break;
    case 771:
      OverallWeather = "Micro Storm";
      break;
    case 781:
      OverallWeather = "TORNADO!";
      break;
    case 800:
      OverallWeather = "Clear Skies";
      break;
    case 801:
      OverallWeather = "Light Clouds";
      break;
    case 802:
      OverallWeather = "Scattered Clouds";
      break;
    case 803:
      OverallWeather = "Broken Clouds";
      break;
    case 804:
      OverallWeather = "Overcast Clouds";
      break;
    default:
      OverallWeather = "ID: " + String(weather_0_id);
      break;
  }
}

void WifiConnection() //Checks if we are connected to wifi and reconnected if not
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_CursivePixel_tr);
  u8g2.drawStr(WEATHERWIDTH, 20, "Connecting");
  u8g2.drawStr(WEATHERWIDTH, 40, "Wifi");
  u8g2.sendBuffer();

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Connecting to Wifi");

    WiFi.begin(ssid, password);

    // attempt to connect to Wifi network:
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      // wait 1 second for re-trying
      delay(500);
    }

  }
  else
  {
    Serial.print("Wifi Already Connected");
  }
  Serial.println("Wifi is connected");
}

void SetValues(char* json)
{
  const size_t capacity = JSON_ARRAY_SIZE(1) + 2 * JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(14) + 290;
  DynamicJsonDocument doc(capacity);

  deserializeJson(doc, json);

  float coord_lon = doc["coord"]["lon"]; // -2.6
  float coord_lat = doc["coord"]["lat"]; // 51.46

  JsonObject weather_0 = doc["weather"][0];
  weather_0_id = weather_0["id"]; // 521
  weather_0_main = weather_0["main"]; // "Rain"
  weather_0_description = weather_0["description"]; // "shower rain"
  const char* weather_0_icon = weather_0["icon"]; // "09n"

  const char* base = doc["base"]; // "stations"

  JsonObject main = doc["main"];
  main_temp = main["temp"]; // 3.81
  main_feels_like = main["feels_like"]; // -4.07
  int main_temp_min = main["temp_min"]; // 2
  float main_temp_max = main["temp_max"]; // 5.56
  int main_pressure = main["pressure"]; // 1012
  int main_humidity = main["humidity"]; // 57

  int visibility = doc["visibility"]; // 10000

  float wind_speed = doc["wind"]["speed"]; // 7.7
  int wind_deg = doc["wind"]["deg"]; // 280

  float rain_1h = doc["rain"]["1h"]; // 0.25

  int clouds_all = doc["clouds"]["all"]; // 40

  long dt = doc["dt"]; // 1581451005

  JsonObject sys = doc["sys"];
  int sys_type = sys["type"]; // 1
  int sys_id = sys["id"]; // 1398
  const char* sys_country = sys["country"]; // "GB"
  sys_sunrise = sys["sunrise"]; // 1581406380
  sys_sunset = sys["sunset"]; // 1581441386

  int timezone = doc["timezone"]; // 0
  long id = doc["id"]; // 2654675
  const char* name = doc["name"]; // "Bristol"
  int cod = doc["cod"]; // 200

  Serial.println("main");
  Serial.println(weather_0_main);
  Serial.println("temp");
  Serial.println(main_temp);
  Serial.println("Weather ID");
  Serial.println(weather_0_id);
  Serial.println("feels like");
  //Serial.println(New_main_feels_like);
  Serial.println("overall weather");
  Serial.println(weather_0_description);

  Output();

}
