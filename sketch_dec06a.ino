#include <ESP8266WiFi.h>
#include <limits.h>

#define SELPIN 0   //Selection Pin (CS/SHDN)   (wifi=GPIO0)  10
#define DATAOUT 1 //Data in  (Din)  (wifi=RX)   11
#define DATAIN 2  //Data out (Dout)  (wifi=GPIO2)   12
#define SPICLOCK 3  //Clock (CLK)   (wifi=tx)   13

#define f(i,n) for(i=0;i<n;i++)

int i,j,k;
struct dat {
  int va;
  int timex;
};

String apiKey = "CP2IX9ZRBIN2EM4B";
const char* ssid = "!tsme";
const char* password = "password";
const char* server = "api.thingspeak.com";

WiFiClient client;

void setup() {
  pinMode(SELPIN, OUTPUT);
  pinMode(DATAOUT, OUTPUT);
  pinMode(DATAIN, INPUT);
  pinMode(SPICLOCK, OUTPUT);

  digitalWrite(SELPIN,HIGH);
  digitalWrite(DATAOUT,LOW);
  digitalWrite(SPICLOCK,LOW);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

/********************This portion is taken from a website on Internet************************
*********************to send analog data to esp through ADC chip via*************************
*********************SPI protocol.**********************************************************/

int read_adc(int channel) {
  int adcvalue = 0;       //a variable to hold the digital value given by the ADC
  byte commandbits = B11000000; //command bits - start, mode, chn (3), dont care (3)

  commandbits|=((channel-1)<<3);

  digitalWrite(SELPIN, LOW); //turn on adc
  for (int i=7; i>=3; i--) {
    digitalWrite(DATAOUT, commandbits&1<<i);
    //cycle clock
    digitalWrite(SPICLOCK, HIGH);
    digitalWrite(SPICLOCK, LOW);
  }

  //cycle clock up and down twice to ignore 2 null bits
  digitalWrite(SPICLOCK, HIGH);   
  digitalWrite(SPICLOCK, LOW);
  digitalWrite(SPICLOCK, HIGH);
  digitalWrite(SPICLOCK, LOW);

  //now read bits from adc
  //??why does it only read in 11 bits instead of 12? should this be "=" instead of ">="?
  for (int i= 11; i>= 0; i--) {
    adcvalue+= digitalRead(DATAIN)<<i;
    //cycle clock
    digitalWrite(SPICLOCK,HIGH);
    digitalWrite(SPICLOCK,LOW);
  }
  digitalWrite(SELPIN, HIGH); //turn off device
  return adcvalue;
}
/*******************************************************************************************/

int heart_rate(struct dat input[250]) {
  int st,en,res;
  struct dat m,maxblock[15][6],ans[6];

  f(k,25) {
    st=k;en=44+k;
    while (st<250) {
      m.va=INT_MIN;
      for(j=st;j<=en && j<250;j++) if(input[j].va>m.va) m=input[j];
      if(m.va<0)  m.va=0;//cout<<m.va<<endl;
      maxblock[k][(int)(st/45)]=m;
      st=en+1;en=st+44;
    }
  }

  f(j,6) {
    m=maxblock[0][j];
    f(k,15) if(maxblock[k][j].va>m.va) m=maxblock[k][j];
    ans[j]=m;
  }
  res = (int)((ans[4].timex - ans[1].timex)/3);
  return(res);
}

void loop() {
  int a, b = micros();    //for timex
  int n=0;
  struct dat inp[250];

  f(n, 250) {
    inp[n].va = read_adc(0);
    inp[n].timex = micros() - a;
    delay(20);
  }

  int h = heart_rate(inp);
  int t = read_adc(1);

  if(client.connect(server,80)) { // "184.106.153.149" or api.thingspeak.com
    String postStr = apiKey;
    postStr +="&field1=";
    postStr += String(t);
    postStr +="&field2=";
    postStr += String(h);
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

  delay(10000);
}
