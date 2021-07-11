
// lowercase roomba.h.  Uppercase Roomba.h is the system library
//******************************* Roomba ************************
#include <Roomba.h>

// Defines the Roomba instance and the HardwareSerial it connected to
//Roomba roomba(&Serial,Roomba::Baud115200);
Roomba roomba(&Serial,Roomba::Baud19200);

time_t lastSensor = 0;
time_t stopDrive = 0;

uint8_t roombaBumps;
uint8_t roombaLightBumper;
uint8_t roombaVirtualWall;
int bumps[6];

int16_t roombaAngle;

int driveSpeed = 200;

int angleDirection = 0;

//roombaStatus:
//  0 - idle
// 10 - executing command
// 99 - command complete
int roombaStatus = 0;

//--------------------------------------------
void driveDistance(int distance) {
  switch (roombaStatus) {
    case 0:
      roombaStatus = 10;
      stopDrive = millis() + abs(distance);
      if (distance > 0) {
        roomba.drive(driveSpeed,Roomba::DriveStraight);
      } else {
        roomba.drive(-driveSpeed,Roomba::DriveStraight);        
      }
      break;
      
    case 10:
      // check bumps; > 15 msec
      // also check that the wheels are turning (for backward bump)
      if ( millis() - lastSensor > 15 ) {
        lastSensor = millis();
        roomba.getSensors(7,   &roombaBumps,             1);
        roomba.getSensors(45,  &roombaLightBumper,       1);
        roomba.getSensors(13,  &roombaVirtualWall,       1);
        if ( roombaBumps == 255 ) { roombaBumps = 0; } 
        //if ( roombaBumps != 0 || roombaLightBumper != 0 || roombaVirtualWall != 0) {
        if ( roombaBumps & 0x1f != 0 || roombaLightBumper & 0x1f != 0 || roombaVirtualWall & 0x1f != 0) {
          sprintf(httpMessage,"driveDistance, bump detected. bump:%d light:%d virtual:%d",roombaBumps, roombaLightBumper,roombaVirtualWall);
          roomba.drive(0,Roomba::DriveStraight); 
          roombaStatus = 99; 
          break;       
        }
      }

      // check time (distance)
      if (millis() > stopDrive) {
        roomba.drive(0,Roomba::DriveStraight); 
        roombaStatus = 99; 
        break;               
      }
      break;

    case 99:
      break;
      
    default:
      // display message (http)
      break;
  }
}

//--------------------------------------------
void driveVelocity(int velocity) {
  // drive at speed until it hits something.
  switch (roombaStatus) {
    case 0:
      strcpy(httpMessage,"drivVelocity starting");
      roombaStatus = 10;
        roomba.drive(velocity,Roomba::DriveStraight);
      break;
      
    case 10:
      // check bumps; > 15 msec
      // also check that the wheels are turning (for backward bump)
      if ( millis() - lastSensor > 15 ) {
        lastSensor = millis();
        roomba.getSensors(7,   &roombaBumps,             1);
        roomba.getSensors(45,  &roombaLightBumper,       1);
        roomba.getSensors(13,  &roombaVirtualWall,       1);
        if ( roombaBumps == 255 ) { roombaBumps = 0; } 
        if ( roombaBumps & 0x1f != 0 || roombaLightBumper & 0x1f != 0 || roombaVirtualWall & 0x1f != 0) {
          sprintf(httpMessage,"driveVelocity, bump detected. bump:%d light:%d virtual:%d",roombaBumps, roombaLightBumper,roombaVirtualWall);
          roomba.drive(0,Roomba::DriveStraight); 
          roombaStatus = 99; 
          break;       
        }
      }

    case 99:
      break;
      
    default:
      // display message (http)
      break;
  }
}

//float calcHeading;
//--------------------------------------------
void turnAngle(int angle) {

  float distance;
  float deltaAngle;

  //Serial.printf("calcHeading %f roombaStatus %d finalAngle %f angledirection %d\n",calcHeading,roombaStatus, finalAngle, angleDirection);
  
  switch (roombaStatus) {
    case 0:
      roombaStatus = 10;

      //which direction?
      deltaAngle = (float)angle - myIMU.yaw;
      if (deltaAngle > 180) { deltaAngle -= 360; }
      if (deltaAngle < -180) { deltaAngle += 360; }
    
      if (deltaAngle > 0) {
        angleDirection = 1;   //CCW - incrementing
        roomba.drive(11,Roomba::DriveInPlaceCounterClockwise);
      } else {
        angleDirection = -1;  //CW - decrementing
        roomba.drive(11,Roomba::DriveInPlaceClockwise);
      }  
      break;

    case 10:
      distance = (float)angle - myIMU.yaw;
      if (angleDirection > 0 ) {      // CCW, incrementing
        if ((distance <= 0 && distance > -180) || distance > 180) {
          roomba.drive(0,Roomba::DriveStraight); 
          roombaStatus = 99;        
          break;
        }
      } else {                        // CW, decrementing
        if ((distance >= 0 && distance < 180 ) || distance < -180) {
          roomba.drive(0,Roomba::DriveStraight); 
          roombaStatus = 99;              
          break;
        }
      }
      break;

    case 99:
      break;
      
    default:
      // display message (http)
      break;
  }
  //Serial.printf("calcHeading %f distance %f finalAngle %f angledirection %d\n\n",calcHeading,distance, finalAngle, angleDirection);
  
}


//--------------------------------------------
void turnNoStop(int angle) {
  
  switch (roombaStatus) {
    case 0:
      roombaStatus = 10;
      roomba.drive(11,Roomba::DriveInPlaceClockwise);
      break;

    case 10:
      break;

    case 99:
      break;
      
    default:
      // display message (http)
      break;
  }
  
}

float initialAngle;
float calcAngle;
int16_t arcRadius = 300; // 300 == 6 inches (11.81)

//--------------------------------------------
void arcAngle(int angle) {

  float distance;
  float deltaAngle;
  
  switch (roombaStatus) {
    case 0:
      roombaStatus = 10;

      //which direction?
      deltaAngle = (float)angle - myIMU.yaw;
      if (deltaAngle > 180) { deltaAngle -= 360; }
      if (deltaAngle < -180) { deltaAngle += 360; }
    
      if (deltaAngle > 0) {
        angleDirection = 1;   //CCW - incrementing
        roomba.drive(driveSpeed, arcRadius);
      } else {
        angleDirection = -1;  //CW - decrementing
        roomba.drive(driveSpeed, -arcRadius);
      }  
      break;

    case 10:
      distance = (float)angle - myIMU.yaw;
      if (angleDirection > 0 ) {      // CCW, incrementing
        if ((distance <= 0 && distance > -180) || distance > 180) {
          roomba.drive(0,Roomba::DriveStraight); 
          roombaStatus = 99;        
          break;
        }
      } else {                        // CW, decrementing
        if ((distance >= 0 && distance < 180 ) || distance < -180) {
          roomba.drive(0,Roomba::DriveStraight); 
          roombaStatus = 99;              
          break;
        }
      }
      break;

    case 99:
      break;
      
    default:
      // display message (http)
      break;
  }
  //Serial.printf("calcHeading %f distance %f finalAngle %f angledirection %d\n\n",calcHeading,distance, finalAngle, angleDirection);
  
}



//--------------------------------------------
void setArcRadius(int radius) {
  arcRadius = radius;
  roombaStatus = 99;
}

//--------------------------------------------
void decArcRadius(int decr) {
  if (arcRadius > decr) {
    arcRadius -= decr;
    roombaStatus = 99;
  }
  // else just go into loop, stopping steps
}

//--------------------------------------------
void incArcRadius(int incr) {
    arcRadius += incr;
    roombaStatus = 99;
}

//--------------------------------------------
void stopMotion() {
  //strcpy(httpMessage,"stopMotion");
  stepsRunning = false;
  roomba.drive(0,Roomba::DriveStraight);
  roombaStatus = 0;
  digitalWrite(LED_BUILTIN, HIGH);
}

//--------------------------------------------
void driveMotion(int dMdist) {
/* 
 *  step 20xxxx and 21xxxx is a movement driveDistance = xxxx
 *  speed for driveDistance is driveSpeed, a global defined here, roomba.h
 */
  // setup def steps
  stepsRunning = false;
  stepMode = 0;
  rStep = 0; 
  roombaStatus = 0;
  if (dMdist > 0) {
    Pattern[0].Steps[0] = 200000 + dMdist;
  } else {
    Pattern[0].Steps[0] = 210000 - dMdist;
  }
  Pattern[0].Steps[1] = 999999;
  Pattern[0].Steps[2] = 0;
  stepsRunning = true;
    
}

//--------------------------------------------
void driveMotionVelocity(int dMspeed) {
/* dMspeed isn't SPEED at all...
 *  looking at defSteps, step 20xxxx and 21xxxx is a movement driveDistance = xxxx
 *  speed for driveDistance is driveSpeed, a global defined here, roomba.h
 */
  // setup def steps
  stepsRunning = false;
  stepMode = 0;
  rStep = 0; 
  roombaStatus = 0;
  Pattern[0].Steps[0] = 220000 + dMspeed;
  Pattern[0].Steps[1] = 999999;
  Pattern[0].Steps[2] = 0;
  stepsRunning = true;
    
}

//--------------------------------------------
void angleMotion(int aMangle) {

  // setup def steps
  stepsRunning = false;
  stepMode = 0;
  rStep = 0; 
  roombaStatus = 0;
  if (aMangle > 0) {
    Pattern[0].Steps[0] = 300000 + aMangle;
  } else {
    Pattern[0].Steps[0] = 300000 + 360 + aMangle;
  }
  Pattern[0].Steps[1] = 999999;
  Pattern[0].Steps[2] = 0;
  stepsRunning = true;

}


//--------------------------------------------
void getSensors() {

  uint8_t buf[2];
  uint8_t bumpBuf[12];
  
  roomba.getSensors(20, buf, 2);
  roombaAngle = (buf[0] << 8) | buf[1];

  roomba.getSensors(45, &roombaLightBumper, 1);
  roomba.getSensors(7,  &roombaBumps,       1);

  roomba.getSensors(106, bumpBuf, 12);
  for ( int i = 0; i < sizeof(bumpBuf); i+=2 ) {
    bumps[i/2] = (bumpBuf[i] << 8) | bumpBuf[i+1];
  }


}


//--------------------------------------------
//--------------------------------------------
void roombaSetup() {
  //******************************* Roomba ************************/
  roomba.start();
  Serial.swap();  //need to call swap after the Serial.begin in roomba.start
  Serial.write(128);  //passive mode
  delay(20);        // allow 20 msec between sending commands that change the SCI mode.
  roomba.fullMode();
  delay(20);

}







/*
GoToAngle - initialHeading = distanceAngle

// problem 1:  which direction to turn?
if distanceAngle > 180 then distanceAngle = distanceAngle - 360
if distanceAngle < -180 then distanceangle = distanceAngle + 360

finalAngle  = initialHeading + distanceAngle

if distanceAngle > 0 then CCW else CW

start motion

// problem 2:  handle 180/-180 sign change

calcHeading = heading

if CCW then                                       // incrementing
    if initialHeading > 0 then                    // if started positive, then stay positive
         calcHeading = (heading + 360)%360
    end
  until calcHeading >= finalAngle
    stop motion
end
  
if CW then                                        // decrementing

    if initialHeading < 0 then                    // if started negative, then stay negative
        calcHeading = (heading - 360)%360
    end
  until calcHeading <= finalAngle
    stop motion
end

GoTo  initial distanceAngle distanceAngle CW/CCW  finalAngle  calcGoto  calcHeading
45    90      -45           -45           CW      45                    45
-135  90      -225          135           CCW     225                   225
-45   90      -135          -135          CW      -45                   -45
135   90

45    -90     135           135           CCW     45                    45
-135  -90     -45           -45           CW      -135                  -135
-45   -90     45            45            CCW     -45                   -45
135   -90     225           -135          CW      -225                  -225

*/
