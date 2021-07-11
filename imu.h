//******************************* IMU **************************

// from Sparkfun_MPU-9250
#include "MPU9250.h"
MPU9250 myIMU;

float imuXbias = -72.2619;
float imuYbias = 1803.9835;
float imuXscale = 1.0276;
float imuYscale = 0.9739;

float lastX, lastY, lastZ;
float lastX1, lastY1;
float lastX2, lastY2;

time_t lastRead = 0;

//--------------------------------------------
void updatePosition() {
  // If intPin goes high, all data registers have new data
  // On interrupt, check if data ready interrupt
  //if (myIMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)   {  

  // if there's mag data ready:
  if (myIMU.readByte(AK8963_ADDRESS, AK8963_ST1) & 0x01)  {
    lastRead = millis();
    
    myIMU.readMagData(myIMU.magCount);  // Read the x/y/z adc values

    // User environmental offsets in milliGauss
    //myIMU.magbias[0] = -101.5924;
    //myIMU.magbias[1] = 1798.9951;
    //myIMU.magbias[0] = -69.2267;
    //myIMU.magbias[1] = 1784.6104;
//    myIMU.magbias[0] = -72.2619;
//    myIMU.magbias[1] = 1803.9835;
//    myIMU.magbias[2] = 758.5095;

    // Calculate the magnetometer values in milliGauss
    myIMU.mx = (float)myIMU.magCount[0] * myIMU.mRes * myIMU.magCalibration[0] - imuXbias;
    myIMU.my = (float)myIMU.magCount[1] * myIMU.mRes * myIMU.magCalibration[1] - imuYbias;
//    myIMU.mz = (float)myIMU.magCount[2] * myIMU.mRes * myIMU.magCalibration[2] - myIMU.magbias[2];
    
    // scaling factors
    myIMU.mx *= imuXscale;
    myIMU.my *= imuYscale;
    //myIMU.mz *= 0.9526;

    // coordinate orientation is: Y-axis points forward; X-axis point to right; Z-axis points up
    // "right-hand convention"
    // MPU-9250: Y-axis forward (parallel to pins); X-axis to left, so MX below is negative.
    //myIMU.yaw  = atan2(-myIMU.mx, myIMU.my);
    //myIMU.yaw  = atan2(-(myIMU.mx+lastX)/2.0, (myIMU.my+lastY)/2.0);
    myIMU.yaw  = atan2(-(myIMU.mx+lastX+lastX1)/3.0, (myIMU.my+lastY+lastY1)/3.0);

    lastX2 = lastX1; lastX1 = lastX; lastX = myIMU.mx;
    lastY2 = lastY1; lastY1 = lastY; lastY = myIMU.my;
    
    myIMU.yaw   *= RAD_TO_DEG;

    // subtract declination
    // http://www.ngdc.noaa.gov/geomag-web/#declination
    // 12.02 degrees E (2017-06-07)
    // 11.70 degrees E (2021-07-03)
    //myIMU.yaw   -= 11.70;
    myIMU.yaw     -= 5.85;    // hmm...
    if ( myIMU.yaw < 0 ) myIMU.yaw += 360;
    if ( myIMU.yaw >= 360 ) myIMU.yaw -= 360;

  } // if there is data to read...
}

//--------------------------------------------
void imuSetup() {
  // Read the WHO_AM_I register, this is a good test of communication
  byte c = myIMU.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);

  if (c != 0x71) {
    while(1) ; // Loop forever if communication doesn't happen
  }

  EEPROM.get(0,imuXbias);
  EEPROM.get(4,imuYbias);
  EEPROM.get(8,imuXscale);
  EEPROM.get(12,imuYscale);
  
  myIMU.calibrateMPU9250(myIMU.gyroBias, myIMU.accelBias);
  // Function which accumulates gyro and accelerometer data after device
  // initialization. It calculates the average of the at-rest readings and then
  // loads the resulting offsets into accelerometer and gyro bias registers.

  myIMU.initMPU9250();
  // wake up device
  // get stable time source
  // Configure Gyro and Thermometer
  // Set sample rate = gyroscope output rate/(1 + SMPLRT_DIV)
  // Set gyroscope full scale range
  // Set accelerometer full-scale range configuration
  // Set accelerometer sample rate configuration
  // Configure Interrupts and Bypass Enable

  myIMU.initAK8963(myIMU.magCalibration);
  // Get magnetometer calibration from AK8963 ROM
  // Configure the magnetometer for continuous read and highest resolution
  // set Mscale bit 4 to 1 (0) to enable 16 (14) bit resolution in CNTL register,
  // and enable continuous mode data acquisition Mmode (bits [3:0]), 0010 for 8 Hz and 0110 for 100 Hz sample rates
  // Mscale defaults to MFS_16BITS; Mmode defaults to 0x02, or 8Hz
  // Mmode is Protected, so changing it is a problem...

  // set sensors scales
  myIMU.getAres();		// AFS_2G
  myIMU.getGres();		// GFS_250DPS
  myIMU.getMres();		// MFS_16BITS


}
