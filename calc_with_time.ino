#include <TimeLib.h>
tmElements_t SunA;   
tmElements_t SunU;
//                0123456789012345 
String WeltZ =   "16:24 13.02.2016";
String SonnenA = "07:49 14.02.2016";    
String SonnenU = "17:47 14.02.2016";
boolean sommerzeit=false;
int timezone=1;
unsigned int Hour, Minute, Second, Day, Month, Year;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print("WeltZ (UTC) = ");
  Serial.println(WeltZ);
  Serial.print("SonnenA = ");
  Serial.println(SonnenA);
  Serial.print("SonnenU = ");
  Serial.println(SonnenU);
  
  // set time to actual time 
  Hour=atoi(WeltZ.substring(0,2).c_str());
  Minute=atoi(WeltZ.substring(3,5).c_str());
  Day=atoi(WeltZ.substring(6,8).c_str());
  Month=atoi(WeltZ.substring(9,11).c_str());
  Year=atoi(WeltZ.substring(13,16).c_str());
  setTime(Hour, Minute, 0, Day, Month, Year);    // time is set to UTC
  Serial.print("Arduino is set to UTC ");
  digitalClockDisplay();
  Serial.print("eqivalent UNIX time is ");
  Serial.println(now());

  // make SonnenA struct
  SunA.Hour=atoi(SonnenA.substring(0,2).c_str());
  SunA.Minute=atoi(SonnenA.substring(3,5).c_str());
  SunA.Second=0;
  SunA.Day=atoi(SonnenA.substring(6,8).c_str());
  SunA.Month=atoi(SonnenA.substring(9,11).c_str());
  SunA.Year=y2kYearToTm(atoi(SonnenA.substring(13,16).c_str()));     // offset from 1970
  unsigned long SunA_ux = makeTime(SunA);     // unixtime 
  SunA_ux = sommerzeit ? SunA_ux - 2*timezone*3600 : SunA_ux - timezone*3600;    // Timezone und summer/wintertime
  Serial.print("nächster Sonnenaufgang ");
  Serial.println(SunA_ux);  
  Serial.print("in ");
  Serial.print(SunA_ux-now());
  Serial.println(" Sekunden"); 

  // make SonnenU struct
  SunU.Hour=atoi(SonnenU.substring(0,2).c_str());
  SunU.Minute=atoi(SonnenU.substring(3,5).c_str());
  SunU.Second=0;
  SunU.Day=atoi(SonnenU.substring(6,8).c_str());
  SunU.Month=atoi(SonnenU.substring(9,11).c_str());
  SunU.Year=y2kYearToTm(atoi(SonnenU.substring(13,16).c_str()));     // offset from 1970
  unsigned long SunU_ux = makeTime(SunU);  
  SunU_ux = sommerzeit ? SunU_ux - 2*3600 : SunU_ux - 3600;
  Serial.print("nächster Sonnenuntergang ");
  Serial.println(SunU_ux);  
//  Serial.print((now()<SunU_ux)?"in ":"vor ");
  Serial.print("in ");
//  Serial.print((now()<SunU_ux)? SunU_ux-now():now()-SunU_ux);
  Serial.print(SunU_ux-now());
  Serial.println(" Sekunden"); 
}

void loop() {
  // put your main code here, to run repeatedly:
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
