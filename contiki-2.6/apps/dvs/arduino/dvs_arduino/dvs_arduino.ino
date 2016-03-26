
#ifndef _H_dvs_arudio
  #define _H_dvs_arudio
  #include <Adafruit_NeoPixel.h>
  #ifdef __AVR__
    #include <avr/power.h>
  #endif
  
  // Neopixel
  #define PIN            38
  #define NUMPIXELS      16
  Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
  int delayval = 500; // delay for half a second
  uint8_t r,g,b;
  
  // MPU6050
  #include "I2Cdev.h"
  #include "MPU6050_6Axis_MotionApps20.h"
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
      #include "Wire.h"
  #endif
  MPU6050 mpu;
  #define OUTPUT_READABLE_YAWPITCHROLL
  // MPU control/status vars
  uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
  uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
  uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
  uint16_t fifoCount;     // count of all bytes currently in FIFO
  uint8_t fifoBuffer[64]; // FIFO storage buffer
  
  // orientation/motion vars
  Quaternion q;           // [w, x, y, z]         quaternion container
  VectorInt16 aa;         // [x, y, z]            accel sensor measurements
  VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
  VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
  VectorFloat gravity;    // [x, y, z]            gravity vector
  float euler[3];         // [psi, theta, phi]    Euler angle container
  float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector
  
  boolean mpuInterrupt = false;
  // packet structure for InvenSense teapot demo
  uint8_t teapotPacket[14] = { '$', 0x02, 0,0, 0,0, 0,0, 0,0, 0x00, 0x00, '\r', '\n' };

//#define printf(...) 
#endif

void setup() {
 //   Serial1.begin(19200);

  r = 255;
  g = 0;
  b = 0;
  pixels.begin(); // This initializes the NeoPixel library.
  setPixel(0,0,0,0);
  
  
  // MPU6050 Init
  Wire.begin();
  TWBR = 24; 
  mpu.initialize();
  printf("%s\n",mpu.testConnection() ? "[MPU6050] connection successful" : "[MPU6050] connection failed");
  
  devStatus = mpu.dmpInitialize();
  printf("[MPU6050] MPU Status: %d\n", devStatus);

  // supply your own gyro offsets here, scaled for min sensitivity
  mpu.setXGyroOffset(220);
  mpu.setYGyroOffset(76);
  mpu.setZGyroOffset(-85);
  mpu.setZAccelOffset(1788); // 1688 factory default for my test chip
  mpu.setDMPEnabled(true);
  
  packetSize = mpu.dmpGetFIFOPacketSize();
  
  
  DDRB &= ~(1 << DDB5);
  PORTB |= (1 << PORTD5);
  PCICR |= (1 << PCIE0);
  PCMSK0 |= (1 << PCINT5);
    
}

void loop() {
  mpu6050();
}

void setPixel(int id, char r, char g, char b){
   pixels.setPixelColor(id, pixels.Color(r,g,b));
   pixels.show(); 
}


char mpu6050(){
    if (devStatus) return -1;
    if (!mpuInterrupt && fifoCount < packetSize) return 0;

    mpuInterrupt = false;
    mpuIntStatus = mpu.getIntStatus();
    fifoCount = mpu.getFIFOCount();
    
    if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
        mpu.resetFIFO();
        printf("[MPU6050] FIFO overflow!");
    } else if (mpuIntStatus & 0x02) {
        while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();
        mpu.getFIFOBytes(fifoBuffer, packetSize);
       
        fifoCount -= packetSize;
        
      mpu.dmpGetQuaternion(&q, fifoBuffer);
      mpu.dmpGetGravity(&gravity, &q);
      mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
      mpu.dmpGetEuler(euler, &q);
      return 1;
    }
    
  return 0;
}

