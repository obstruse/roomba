
//******************************* global **************************

#include <Wire.h>
#include <ESP_EEPROM.h>

char fileName[] = __FILE__;

// in the hardware subfolder of the Arduino IDE installation folder, create platform.txt:
//       compiler.cpp.extra_flags=-D__PATH__="{build.source.path}"
char pathname[] = __PATH__;


// pins 
//#define P0              0
//#define P2              2
#define SDA             4  
#define SCL             5 
#define buttonReset     12 
#define RX2             13 
#define buttonBlack     14 
#define TX2             15 
//#define P16             16 

char temp[2500];

//int angleMotion = 0;
int angleStart = 0;

boolean BumpnTurn = false;
int bntStep = 0;
int bntSpeed = 0;
int bntAngle = 0;

char httpMessage[200];


#include "steps.h"
#include "imu.h"
#include "wifi.h"
#include "roomba.h"
#include "http.h"
#include "httpCalib.h"
#include "httpWifi.h"

//--------------------------------------------
//--------------------------------------------
void setup()
{
  Serial.begin(115200);

  pinMode(buttonReset, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  EEPROM.begin(16);             // 16-bytes
  
  Wire.begin();

  wifiSetup();

  
  httpSetup();
  httpCalibSetup();
  httpWifiSetup();

  imuSetup();

  roombaSetup();
  
  Serial.println("Server ready");
  digitalWrite(LED_BUILTIN, HIGH);
}


//--------------------------------------------
//--------------------------------------------
void loop() {


  server.handleClient();
  //ArduinoOTA.handle();
  wifiOTA();
  wifiDNS();
  
  updatePosition();

  if (calibRun) {
    imuCalib();
  }
  
  if (roombaStatus == 99) {
    roombaStatus = 0;
    rStep++;
  }

  if (stepsRunning) {
    int stepCommand = Pattern[stepMode].Steps[rStep]/10000;
    int stepData    = Pattern[stepMode].Steps[rStep]%10000;
    switch( stepCommand ) {
      case 0:   rStep = stepData; roombaStatus = 0; break;
      case 20:  driveDistance(stepData); break;
      case 21:  driveDistance(-stepData); break;
      case 22:  driveVelocity(stepData); break;
      case 30:  turnAngle(stepData); break;
      case 39:  turnNoStop(stepData); break; // turn without stopping
      case 40:  arcAngle(stepData); break;
      case 50:  setArcRadius(stepData); break;
      case 51:  decArcRadius(stepData); break;
      case 52:  incArcRadius(stepData); break;
      case 99:  stopMotion(); break;
      default: rStep = 0; roombaStatus = 0; break;
    }
  }
  
  
}
