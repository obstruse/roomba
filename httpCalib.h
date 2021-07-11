#ifndef HTTP_H
#define HTTP_H
#include <ESP8266WebServer.h>

ESP8266WebServer server ( 80 );
#endif

time_t lastCalib = 0;
boolean calibRun = false;

float xMax, yMax, zMax;
float xMin, yMin, zMin;
float xBias, yBias, zBias;
float xMag, yMag, zMag;
float xScale, yScale, zScale;

int calibIndex;
struct CALIB {
  float x;
  float y;
};
//#define calibMax 1024
#define calibMax 256
CALIB calibData[calibMax];
boolean calibAverage = false;
int calibRotation;                 // count of calibration rotations

int    calibRate = 0;           // ms between calib calls

//--------------------------------------------
void httpImuCalibrate() {

  if ( calibIndex > 0 ) {
    // calculate calibration parameters
    xBias = (xMax + xMin)/2.0;
    yBias = (yMax + yMin)/2.0;

    xMag = xMax - xMin;
    yMag = yMax - yMin;

    xScale = (xMag + yMag)/2.0/(xMag == 0 ? 1 : xMag);
    yScale = (xMag + yMag)/2.0/(yMag == 0 ? 1 : yMag);
  }
  
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.sendHeader("Content-Type","text/html",true);
  server.sendHeader("Cache-Control","no-cache");
  server.send(200);

  /*-------------------------------------------*/
  sprintf ( temp,
            "<html>\
<head><title>Calibrate</title></head>\
<style> body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; } </style>\
<body>\
<h1>IMU Calibrate</h1>" );
  server.sendContent ( temp );

  sprintf ( temp,
    "<form action='/calibControl'>\
    <table border=1 cellpadding=5>\
    <tr><th colspan=3>Calibration</th></tr>\
    <tr><td colspan=3><table border=0 cellpadding=5>\
                      <tr><td><input type='radio' name='motor' value='left'>Left</td>\
                          <td>Right<input type='radio' name='motor' value='right'></td></tr>\
                      <tr><td><center><input type='submit' name='submit' value='start'></center></td>\
                          <td><center><input type='submit' name='submit' value='stop'></center></td></tr>\
                      </table>\
    </tr>\
    " );
  server.sendContent( temp );

  sprintf ( temp,
    "<tr><th colspan=3>Result</th></tr>\
    <tr><td></td><th>Bias</th><th>Scale</th></tr>\
    <tr><td>X</td><td>%s</td><td>%s</td></tr>\
    <tr><td>Y</td><td>%s</td><td>%s</td></tr>\
    </table><br>",
    String(xBias,4).c_str(), String(xScale,4).c_str(),
    String(yBias,4).c_str(), String(yScale,4).c_str() );
  server.sendContent ( temp );

  sprintf ( temp,
    "<form action='/calibControl'>\
    <table border=1 cellpadding=5>\
    <tr><th colspan=3>Calibration Setting</th></tr>\
    <tr><td></td><th>Bias</th><th>Scale</th></tr>\
    <tr><td>X</td><td><input type=number step='0.0001' name='imuXbias' value='%s'></td><td><input type=number step='0.0001' name='imuXscale' value='%s'></td></tr>\
    <tr><td>Y</td><td><input type=number step='0.0001' name='imuYbias' value='%s'></td><td><input type=number step='0.0001' name='imuYscale' value='%s'></td></tr>\
    <tr><td colspan=3><center><input type='submit' name='submit' value='Set'></center></td></tr>\
    </table></form>",
    String(imuXbias,4).c_str(),
    String(imuXscale,4).c_str(),
    String(imuYbias,4).c_str(),
    String(imuYscale,4).c_str()
    );
  server.sendContent ( temp );   

/*
  sprintf ( temp, "accelerometer: <pre>%s %s %s </pre>",
    String(myIMU.ax,4).c_str(),String(myIMU.ay,4).c_str(),String(myIMU.az,4).c_str()
  );
  server.sendContent ( temp );

  sprintf ( temp, "magnetometer: <pre>%s %s %s </pre>",
    String(myIMU.mx,4).c_str(),String(myIMU.my,4).c_str(),String(myIMU.mz,4).c_str()
  );
  server.sendContent ( temp );

  sprintf ( temp, "p/r/y: <pre>%s %s %s </pre>",
    String(myIMU.pitch,4).c_str(),String(myIMU.roll,4).c_str(),String(myIMU.yaw,4).c_str()
  );
  server.sendContent ( temp );

  sprintf ( temp, "angleDirection/calcHeading: <pre>%d %s</pre>",
    angleDirection,String(myIMU.yaw,4).c_str()
  );
  server.sendContent ( temp );
*/
  sprintf ( temp, "calibRate: <pre>%d</pre>",
    calibRate
  );
  server.sendContent ( temp );


  sprintf ( temp, "</body></html>" );
  server.sendContent ( temp );

  server.sendContent (" ");

}

//--------------------------------------------
void httpImuCalibControl() {
  String inSubmit = server.arg("submit");

  if ( inSubmit == "Set" ) {
    imuXbias = server.arg("imuXbias").toFloat();
    imuYbias = server.arg("imuYbias").toFloat();
    imuXscale = server.arg("imuXscale").toFloat();
    imuYscale = server.arg("imuYscale").toFloat();

    EEPROM.put(0,imuXbias);
    EEPROM.put(4,imuYbias);
    EEPROM.put(8,imuXscale);
    EEPROM.put(12,imuYscale);
    EEPROM.commit();

    // redirect to calibrate
    server.sendHeader("Location", String("/calibrate"), true);
    server.send ( 302, "text/plain", "");      
  }

  if ( inSubmit == "start" ) {
    String inMotor = server.arg("motor");

   // clear calibData
    xMax = yMax = 0;
    xMin = yMin = 99999;
    xBias = yBias = 0;
    xScale = yScale = zScale = 1;

    calibIndex = 0;
    calibAverage = false;
    calibRotation = 0;

    if ( inMotor == "left" )  { roomba.drive(20,Roomba::DriveInPlaceCounterClockwise); }
    if ( inMotor == "right" ) { roomba.drive(20,Roomba::DriveInPlaceClockwise); }
    
    calibRun = true;
    
    // return to /calibrate, no update
    server.send ( 204, "text/plain", "");
  }
  
  if ( inSubmit == "stop" ) {
    // stop rotating
    roomba.drive(0,Roomba::DriveStraight); 
    
    calibRun = false;
    
    
    // redirect to calibrate
    server.sendHeader("Location", String("/calibrate"), true);
    server.send ( 302, "text/plain", "");

  }
    
}



//--------------------------------------------
void imuCalib() {

    const int rotSeconds = 5;  // rough seconds to rotate at speed 11 (26.4 seconds to rotate at speed 11)
    //const int calibMS = rotSeconds * 1000 / calibMax;     // fill calibData in one rotation
    const int calibMS = 15;
    const int calibRotationMax = 20;                          // number of repetitions (rotations)
    
  if ( millis() - lastCalib >= calibMS ) {
    
    calibRate = millis() - lastCalib;    
    lastCalib = millis();

    float mx = myIMU.magCount[0] * myIMU.mRes * myIMU.magCalibration[0];
    float my = myIMU.magCount[1] * myIMU.mRes * myIMU.magCalibration[1];

    calibData[calibIndex].x = mx;
    calibData[calibIndex].y = my;

    calibIndex++;
    if (calibIndex >= calibMax) {
      calibIndex = 0;
      calibAverage = true;
      calibRotation++;
    }
    
    if (calibAverage) {
      double xTotal = 0, yTotal = 0;
      for ( int i = 0; i < calibMax; i++) {
          xTotal += calibData[i].x;
          yTotal += calibData[i].y;
      }
      float xAvg = xTotal / calibMax;
      float yAvg = yTotal / calibMax;

      if (xMax < xAvg) xMax = xAvg;
      if (yMax < yAvg) yMax = yAvg;

      if (xMin > xAvg) xMin = xAvg;
      if (yMin > yAvg) yMin = yAvg;
      
    } 
    

    if ( calibRotation > calibRotationMax ) {
      // stop rotating
      roomba.drive(0,Roomba::DriveStraight); 
    
      calibRun = false;      
    }

    
  }
}


//--------------------------------------------
void httpCalibSetup() {
  server.on ( "/calibrate", httpImuCalibrate );
  server.on ( "/calibControl", httpImuCalibControl );

  server.begin();
}
