#include <ESP8266WiFi.h>
#include <TinyGPS.h>

String apiKey = "CP2IX9ZRBIN2EM4B";
const char* ssid = "GenPuchchu";
const char* password = "motamagga";
const char* server = "api.thingspeak.com";

long lati, lon;
unsigned long tym, lastUpd;

TinyGPS gps;

WiFiClient client;

void setup(){
  Serial.begin(9600);
  delay(10);
  
  WiFi.begin(ssid,password);
  
  while(WiFi.status() != WL_CONNECTED){
  delay(500);
  }
}

void loop(){
  
  while(Serial.available()){
       if (gps.encode(Serial.read())){
         gps.get_position( &lati, &lon );
         gps.get_datetime( NULL, &tym, &lastUpd);
       }
    }
  if (client.connect(server,80)){
    String postStr = apiKey;
    postStr +="&field1=";
    postStr += String(lati);
    postStr +="&field2=";
    postStr += String(lon);
    postStr +="&field3=";
    postStr += String(tym);
    postStr +="&field4=";
    postStr += String(lastUpd);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
  }
  client.stop();

  delay(15000);
}
