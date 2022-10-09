#include <Servo.h>
#include <Wire.h>

int servoPin = 3;
Servo myServo;
const int mpu = 0x68; // I2C address of the mpu
float accelX, accelY, accelZ;
float roll, pitch;
float elapsedTime, currentTime, previousTime;
int angle;
int offset = 90;

const byte numChars = 32;
char receivedChars[numChars]; // An array to store the data

boolean newData = false;

int dataNumber = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // start serial monitor 
  myServo.attach(servoPin);
  myServo.write(0);

  Wire.begin(); // Init communication
  Wire.beginTransmission(mpu); // Start communication with MPU
  Wire.write(0x6B); // register address
  Wire.write(0);
  Wire.endTransmission(true);
  setupMPU();
}

void loop() {
  // put your main code here, to run repeatedly:
  recordAccelRegisters();
  
  endMarker();
  angle = angleInput();
  
  myServo.write(angle + (offset - roll) - pitch);
  delay(10);

  printData();
  delay(100);
}

void setupMPU() {
  // Establish communication with MPU and set up register
  Wire.beginTransmission(mpu);
  Wire.write(0x6B); // Accessing the register 6B - Power Management
  Wire.write(0b00000000); // Setting SLEEP
  Wire.endTransmission();

  Wire.beginTransmission(mpu);
  Wire.write(0x1B); // Accessing the register 1B - Gyroscope Config
  Wire.write(0x00000000); // Setting Gyroscope to full scale +/- 250 deg
  Wire.endTransmission();
  
  Wire.beginTransmission(mpu);
  Wire.write(0x1C); // Accessing the register 1C - Accelerometer Config
  Wire.write(0b00000000); // Setting the accel to +/- 2g
  Wire.endTransmission();
}

void recordAccelRegisters() {
  Wire.beginTransmission(mpu);
  Wire.write(0x3B); // Starting register for accelerometer reading
  Wire.endTransmission(false);
  Wire.requestFrom(mpu, 6, true); // Request Accel registers
  // For a range of +/- 2g, divide raw values by 16384.0
  accelX = (Wire.read()<<8|Wire.read()) / 16384.0; // Store first two bytes into X
  accelY = (Wire.read()<<8|Wire.read()) / 16384.0; // Store middle two bytes into Y
  accelZ = (Wire.read()<<8|Wire.read()) / 16384.0; // Store last two bytes into Z

  pitch = (-180 * atan (accelY/sqrt(pow(accelX, 2) + pow(accelZ, 2)))/M_PI) - 1.17;
  roll = (180 * atan (accelX/sqrt(pow(accelY, 2) + pow(accelZ, 2)))/M_PI) + 5.41;
}

// Receiving a single number from the Serial Monitor
void endMarker() {
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;

  if(Serial.available() > 0) {
    rc = Serial.read();

    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    }
    else {
      receivedChars[ndx] = '\0';
      ndx = 0;
      newData = true;
    }
  }
}

int angleInput() {
  if (newData == true) {
    dataNumber = 0;
    dataNumber = atoi(receivedChars);
    newData = false;
    if(dataNumber >= -90 || dataNumber <= 90) {
      return dataNumber;
    }
  }
  return angle;
}

void printData() {
  Serial.print("Accelerometer X: ");
  Serial.println(accelX);
  Serial.print("Accelerometer Y: ");
  Serial.println(accelY);
  Serial.print("Accelerometer Z: ");
  Serial.println(accelZ);
  Serial.print("Roll: ");
  Serial.println(roll);
  Serial.print("Pitch: ");
  Serial.println(pitch);
} 