#include <SoftwareSerial.h>
#include "Adafruit_ST7735.h"

#include "TinyGPS++.h"

/* This sample code demonstrates the normal use of a TinyGPS object.
   It requires the use of SoftwareSerial, and assumes that you have a
   4800-baud serial GPS device hooked up on pins 4(rx) and 3(tx).
*/

#define GPS_RX 2
#define GPS_TX 3
SoftwareSerial gpsSerial =  SoftwareSerial(GPS_RX, GPS_TX);
TinyGPSPlus gps;
/*char sGPS[128]="";
int sGPSlen=0;
char slat[16]="";
char slon[16]="";*/

/*const char *gpsStream =
  "$GPRMC,045103.000,A,3014.1984,N,09749.2872,W,0.67,161.46,030913,,,A*7C\r\n"
  "$GPGGA,045104.000,3014.1985,N,09749.2873,W,1,09,1.2,211.6,M,-22.5,M,,0000*62\r\n"
  "$GPRMC,045200.000,A,3014.3820,N,09748.9514,W,36.88,65.02,030913,,,A*77\r\n"
  "$GPGGA,045201.000,3014.3864,N,09748.9411,W,1,10,1.2,200.8,M,-22.5,M,,0000*6C\r\n"
  "$GPRMC,045251.000,A,3014.4275,N,09749.0626,W,0.51,217.94,030913,,,A*7D\r\n"
  "$GPGGA,045252.000,3014.4273,N,09749.0628,W,1,09,1.3,206.9,M,-22.5,M,,0000*6F\r\n";*/

//char gpsStream[512];

void gpsSetup()
{
  pinMode(GPS_RX, INPUT);
  pinMode(GPS_TX, OUTPUT);
  gpsSerial.begin(9600);

  Serial.print(F("Testing TinyGPS++ library v. ")); Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F(" By Erkin Sattorov."));
  Serial.println();  
}  


void displayInfo()
{ tft.setCursor(0,25);
  Serial.print(F("Location: "));tft.print("Location:");//,2,25); 
  if (gps.location.isValid())
  { char s[16]; 
    Serial.print(gps.location.lat(), 6);sprintf(s,"%f",gps.location.lat());tft.print(s);//,65,25);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);sprintf(s,"%f",gps.location.lng());tft.print(s);//,90,25);
    Serial.print(gps.speed.kmph(), 6);sprintf(s,"%f",gps.speed.kmph());tft.print(s);//,90,25);
  }
  else
  {
    Serial.print(F("INVALID"));tft.print("INVALID");//,2,25); 
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}

void gps_loop()
{ //sGPSlen = 0;
  // This sketch displays information every time a new sentence is correctly encoded.
  gpsSerial.listen();
  while (gpsSerial.available() > 0)
  { char c = gpsSerial.read();
    Serial.print(c);tft.print(c);
    if (gps.encode(c))
      displayInfo();
    //if(sGPSlen<126)
    // sGPS[sGPSlen++]=c;
  }
  if(gps.location.isUpdated())
  { //sprintf(slat,"%6f",gps.location.lat());
    //sprintf(slon,"%6f",gps.location.lng());
    //Serial.print("Lat = ");Serial.print(gps.location.lat(),6);
    //Serial.print("Long = ");Serial.print(gps.location.lng(),6);
  }
  
  //Serial.println();
  //sGPS[sGPSlen]="\n";

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
}
//print_float(gps.f_speed_kmph(), TinyGPS::GPS_INVALID_F_SPEED, 6, 2);
