/*
 * Code to make any action on the time of sunrise and sunset 
 * released in the Public Domain
 * (c) Reinhard Nickels 2016
 */
extern "C" {
#include "user_interface.h"
}
#include <ESP8266WiFi.h>
#include <TimeLib.h>

tmElements_t SunA;
tmElements_t SunU;

const char* ssid     = "<your SSID>";
const char* password = "<your passcode>";
const char* host = "www.netzwolf.info";

String SonnenA, SonnenU, WeltZ;
boolean sommerzeit;
int timezone = 1;
unsigned int Hour, Minute, Second, Day, Month, Year;
unsigned long time2Action;
#define AUF true
#define ZU false
boolean action;

WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  int wcounter=0;
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    wcounter++;
    delay(500);
    Serial.print(".");
    if (wcounter > 20) {
      Serial.println("No connection - next try in 1 min");
      system_deep_sleep_set_option(0);
      system_deep_sleep(60UL * 1000 * 1000);  // deep sleep for 1 min, then try again
      delay(1000);
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void loop() {
  Serial.print("connecting to ");
  Serial.println(host);
  // Use WiFiClient class to create TCP connections
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request
  String url = "/astronomie/ephemeriden/sonne";

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  // start waiting for the response
  unsigned long lasttime = millis();
  while (!client.available() && millis() - lasttime < 1000) {
    delay(1); // wait max 1s for data
  }
  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\n');
    if (line.indexOf("Sonnenaufgang    : <b>") >= 0) { // Searchstring exists?
      Serial.println();
      Serial.println(line);
      SonnenA = line.substring(22, 27) + line.substring(32, 43);
      Serial.println(SonnenA);
      if (line.indexOf("(S)") >= 0) {
        sommerzeit = true;
      }
      else if (line.indexOf("(W)") >= 0) {
        sommerzeit = false;
      }
      if (sommerzeit) Serial.println("Es ist Sommerzeit"); else Serial.println("Es ist Winterzeit");
    }
    if (line.indexOf("Sonnenuntergang  : <b>") >= 0) { // Searchstring exists?
      Serial.println();
      Serial.println(line);
      SonnenU = line.substring(22, 27) + line.substring(32, 43);
      Serial.println(SonnenU);
    }
    if (line.indexOf("Weltzeit (UTC)   : <b>") >= 0) { // Searchstring exists?
      Serial.println();
      Serial.println(line);
      WeltZ = line.substring(22, 27) + line.substring(32, 43);
      Serial.println(WeltZ);
    }
  }
  Serial.println();
  Serial.println("closing connection");
  // start times calculations
  // set time to actual time
  Hour = atoi(WeltZ.substring(0, 2).c_str());
  Minute = atoi(WeltZ.substring(3, 5).c_str());
  Day = atoi(WeltZ.substring(6, 8).c_str());
  Month = atoi(WeltZ.substring(9, 11).c_str());
  Year = atoi(WeltZ.substring(13, 16).c_str());
  setTime(Hour, Minute, 0, Day, Month, Year);    // time is set to UTC
  Serial.print("Arduino is set to UTC ");
  digitalClockDisplay();
  Serial.print("eqivalent UNIX time is ");
  Serial.println(now());

  // make SonnenA struct
  SunA.Hour = atoi(SonnenA.substring(0, 2).c_str());
  SunA.Minute = atoi(SonnenA.substring(3, 5).c_str());
  SunA.Second = 0;
  SunA.Day = atoi(SonnenA.substring(6, 8).c_str());
  SunA.Month = atoi(SonnenA.substring(9, 11).c_str());
  SunA.Year = y2kYearToTm(atoi(SonnenA.substring(13, 16).c_str()));  // offset from 1970
  unsigned long SunA_ux = makeTime(SunA);     // unixtime
  SunA_ux = sommerzeit ? SunA_ux - 2 * timezone * 3600 : SunA_ux - timezone * 3600; // Timezone und summer/wintertime
  if (now() >= SunA_ux) {
    Serial.print("letzter Sonnenaufgang war ");
    Serial.println(SunA_ux);
    Serial.print("vor ");
    Serial.print(now() - SunA_ux);
    Serial.println(" Sekunden");
  }
  else {
    Serial.print("nächster Sonnenaufgang ist");
    Serial.println(SunA_ux);
    Serial.print("in ");
    Serial.print(SunA_ux - now());
    Serial.println(" Sekunden");
  }

  // make SonnenU struct
  SunU.Hour = atoi(SonnenU.substring(0, 2).c_str());
  SunU.Minute = atoi(SonnenU.substring(3, 5).c_str());
  SunU.Second = 0;
  SunU.Day = atoi(SonnenU.substring(6, 8).c_str());
  SunU.Month = atoi(SonnenU.substring(9, 11).c_str());
  SunU.Year = y2kYearToTm(atoi(SonnenU.substring(13, 16).c_str()));  // offset from 1970
  unsigned long SunU_ux = makeTime(SunU);
  SunU_ux = sommerzeit ? SunU_ux - 2 * timezone * 3600 : SunU_ux - timezone * 3600; // Timezone und summer/wintertime
  if (now() >= SunU_ux) {
    Serial.print("letzter Sonnenuntergang war ");
    Serial.println(SunU_ux);
    Serial.print("vor ");
    Serial.print(now() - SunU_ux);
    Serial.println(" Sekunden");
  }
  else {
    Serial.print("nächster Sonnenuntergang ist");
    Serial.println(SunU_ux);
    Serial.print("in ");
    Serial.print(SunU_ux - now());
    Serial.println(" Sekunden");
  }
  if (now() >= SunA_ux && now() >= SunU_ux) {
    Serial.println("Es ist Nacht - Türe zu");
    Serial.print("ich geh schlafen bis zur nächsten Aktion in ");
    Serial.print(SunA_ux - now());
    time2Action = SunA_ux - now();
    action = AUF;
    Serial.println(" Sekunden");
  }
  else if (now() <= SunA_ux && now() <= SunU_ux) {
    Serial.println("Es ist Nacht - Türe zu");
    Serial.print("ich geh schlafen bis zur nächsten Aktion in ");
    Serial.print(SunA_ux - now());
    time2Action = SunA_ux - now();
    action = AUF;
    Serial.println(" Sekunden");
  }
  else if (now() >= SunA_ux && now() <= SunU_ux) {
    Serial.println("Es ist Tag - Türe auf");
    Serial.print("ich geh schlafen bis zur nächsten Aktion in ");
    Serial.print(SunU_ux - now());
    time2Action = SunU_ux - now();
    action = ZU;
    Serial.println(" Sekunden");
  }
  Serial.print("Time2Action = ");
  Serial.println(time2Action);
  // Zeitabstand zum nächsten Schaltvorgang ist time2Action
  // wenn Abstand < 2min dann warte und schalte dann und geh 60 min schlafen
  if (time2Action < 120) {
    delay(time2Action * 1000);
    Tuerhandling(action);
    system_deep_sleep_set_option(0);
    system_deep_sleep(60UL * 60 * 1000 * 1000);      // deep sleep for 1h
    delay(1000);
  }
  // wenn Abstand < 65 min, dann schlafe bis 1 min vor Event
  else if (time2Action < (65UL * 60)) {
    system_deep_sleep_set_option(0);
    system_deep_sleep((time2Action - 60) * 1000 * 1000); // deep sleep
    delay(1000);
  }
  else {  // schlafe 60 min
    system_deep_sleep_set_option(0);
    system_deep_sleep(60UL * 60 * 1000 * 1000);      // deep sleep
    delay(1000);
  }
}

void digitalClockDisplay() {      // helper function to print time/date readable from structure
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(".");
  Serial.print(month());
  Serial.print(".");
  Serial.print(year());
  Serial.println();
}

void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void Tuerhandling(boolean action) {
  if (action) {     // Tür auf
    Serial.println("Mach auf !!");
  }
  else {      // Tür zu
    Serial.println("Mach zu !!");
  }
}

