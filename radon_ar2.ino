// Arduino (Uno) program to interface Safety Siren Pro Series 3 Radon Gas Detector Model No: HS71512
// Periodically read radon level from 7-segment indicators and publish to Xively service to build graphs.
// Note: Hs71512 requires hardware modification to connect to Arduino
// Based on works of Chris Nafis (http://www.howmuchsnow.com) for Safety Siren Radon Gas Detector Model No: HS80002
// Modified by Andriy Rokhmanov (http://rokhmanov.blogspot.com) for HS71512
//
#include <SPI.h>
#include <Ethernet.h>
#include <HttpClient.h>
#include <Xively.h>

#define FEED_ID 2076513519
#define LTL 2 // int 0
#define DIGIT_1 17
#define DIGIT_2 16
#define DIGIT_3 15
#define DIGIT_4 18
#define MENU_BUTTON 19
#define SEG_A 14
#define SEG_B 3
#define SEG_C 4
#define SEG_D 5
#define SEG_E 6
#define SEG_F 7
#define SEG_G 8
#define ST_SHORT 0
#define ST_LONG 1

char xivelyKey[] = "put your key here";
byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x42 };
IPAddress ip(192, 180, 1, 222);
char shortId[] = "short";
char longId[] = "long";
XivelyDatastream datastreams[] = {
  XivelyDatastream(shortId, strlen(shortId), DATASTREAM_FLOAT),
  XivelyDatastream(longId, strlen(longId), DATASTREAM_FLOAT)
};
XivelyFeed feed(FEED_ID, datastreams, 2);

EthernetClient client;
XivelyClient xivelyClient(client);
  
int segments[] = {
  0,0,0,0,0,0,0};
int leddigits[] = {
  1,1,1,1,1,1,1, // space
  1,1,1,0,0,0,1, // L
  0,0,0,0,0,0,1, // 0
  1,0,0,1,1,1,1, // 1
  0,0,1,0,0,1,0, // 2
  0,0,0,0,1,1,0, // 3
  1,0,0,1,1,0,0, // 4
  0,1,0,0,1,0,0, // 5 or S
  0,1,0,0,0,0,0, // 6
  0,0,0,1,1,1,1, // 7
  0,0,0,0,0,0,0, // 8
  0,0,0,1,1,0,0 // 9
};
char ledvalues[] = " L0123456789";
char leddisp[] = "    ";

int i = 0;
int j = 0;
int found = 0;
int ledvaluecnt = 0;

unsigned long duration;
volatile int state = 9999;
float shortValue = 0;
float longValue = 0;
int delayRequest = 0;

void setup() 
{  
  Serial.begin(9600);
  pinMode(DIGIT_1, INPUT); 
  pinMode(DIGIT_2, INPUT); 
  pinMode(DIGIT_3, INPUT); 
  pinMode(DIGIT_4, INPUT); 
  pinMode(SEG_A, INPUT); 
  pinMode(SEG_B, INPUT); 
  pinMode(SEG_C, INPUT); 
  pinMode(SEG_D, INPUT); 
  pinMode(SEG_E, INPUT); 
  pinMode(SEG_F, INPUT); 
  pinMode(SEG_G, INPUT); 
  pinMode(LTL, INPUT); // Long Term strobe
  pinMode(9, OUTPUT); // Radon Fan Control (not used in this implementation)
  pinMode(MENU_BUTTON, OUTPUT); // Menu Button Control
  digitalWrite(MENU_BUTTON, HIGH); // Pulse menu button LOW to switch 
  attachInterrupt(0, detectShortHandler, RISING); // Detect if we are in Long or Short reading mode
  delay(1000);
  if (Ethernet.begin(mac) == 0) 
  {
    Serial.println("Failed to configure Ethernet using DHCP, use fixed IP:" + ip);
    Ethernet.begin(mac, ip);
  }
  Serial.println("Starting...");
}

void loop() 
{
    delay(30*1000);  
    if (delayRequest >= 12)
    {
      delayRequest = 0;
      processSingleSet();
      toggleLongShortReading(); 
    } else
    {
      Serial.println(delayRequest);
      delayRequest++;
    }
}

void processSingleSet()
{
    duration = pulseIn(DIGIT_1, LOW);
    processDigitNumber(0);
    duration = pulseIn(DIGIT_2, LOW);
    processDigitNumber(1);
    duration = pulseIn(DIGIT_3, LOW);
    processDigitNumber(2);
    duration = pulseIn(DIGIT_4, LOW);
    processDigitNumber(3);
    if (state == ST_SHORT)
    {
      shortValue = atof(leddisp)/10;
    }
    if (state == ST_LONG)
    {
      longValue = atof(leddisp)/10;
    }
    
    datastreams[0].setFloat(shortValue);
    datastreams[1].setFloat(longValue);
    Serial.print( "short:");
    Serial.println(shortValue);
    Serial.print( "long:");
    Serial.println(longValue);
// The Radon Detector HS71512 collects radon data for 48 hours after reset
// LInes below prevent populate 0's to Xively.
//    if (0 != shortValue && 0 != longValue)
    if (0 != longValue)
    {
      int ret = xivelyClient.put(feed, xivelyKey);
      Serial.print("xivelyclient.put returned:");
      Serial.println(ret);
      Serial.println();      
    }   
}

// Interrupt handler to detect if we are in Long or Short term reading mode
void detectShortHandler()
{
  delayMicroseconds(100);
  if (digitalRead(DIGIT_4) == LOW)
    state = ST_SHORT;
  else
    state = ST_LONG;
}

void processDigitNumber(int digitNumber)
{
  delay(40);
  segments[0] = digitalRead(SEG_A);
  segments[1] = digitalRead(SEG_B);
  segments[2] = digitalRead(SEG_C);
  segments[3] = digitalRead(SEG_D);
  segments[4] = digitalRead(SEG_E);
  segments[5] = digitalRead(SEG_F);
  segments[6] = digitalRead(SEG_G);  
  ledvaluecnt = 0;
  for (i=0;i<sizeof(leddigits)/sizeof(int);i=i+7)
  {
    found = 1;
    for (j=0;j<7;j++)
    {
      if (segments[j] != leddigits[i+j]){
        found = 0;
        break;
      }
    }
    if (found == 1)
    {
      leddisp[digitNumber] = ledvalues[ledvaluecnt];      
      break;
    }
    ledvaluecnt++;
  }  
}

// Switch between Long and Short term reading
// Pulse logical 0 for 1 second
void toggleLongShortReading()
{
  digitalWrite(MENU_BUTTON, LOW);
  delay(1000);
  digitalWrite(MENU_BUTTON, HIGH);
}


