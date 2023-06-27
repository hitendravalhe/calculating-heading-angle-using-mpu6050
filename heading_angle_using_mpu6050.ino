#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

// MPU6050 calibration offsets
int16_t ax_offset = 0;
int16_t ay_offset = 0;
int16_t az_offset = 0;
int16_t gx_offset = 0;
int16_t gy_offset = 0;
int16_t gz_offset = 0;

// Constants for converting raw accelerometer values to degrees
const float AccelScaleFactor = 16384.0;
const float GyroScaleFactor = 131.0;

unsigned long prevT = 0; // Declaration added here

void setup() {
  Wire.begin();
  Serial.begin(9600);

  mpu.initialize();
  
  // Calibrate MPU6050
  calibrateMPU6050();
}

void loop() {
  int16_t ax, ay, az, gx, gy, gz;  // Declarations added here
  
  // Read accelerometer and gyroscope data
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  
  // Apply calibration offsets
  ax -= ax_offset;
  ay -= ay_offset;
  az -= az_offset;
  gx -= gx_offset;
  gy -= gy_offset;
  gz -= gz_offset;
  
  // Convert raw accelerometer values to degrees
  float AccXangle = atan2(ay, az) * (180.0 / PI);
  float AccYangle = atan2(ax, az) * (180.0 / PI);
  
  // Convert raw gyroscope values to degrees per second
  float GyroXrate = gx / GyroScaleFactor;
  float GyroYrate = gy / GyroScaleFactor;
  float GyroZrate = gz / GyroScaleFactor;
  
  // Calculate the elapsed time since the last loop iteration
  unsigned long t = micros();
  float dt = (t - prevT) / 1000000.0; // Convert microseconds to seconds
  prevT = t;
  
  // Calculate the change in angles using the gyroscope values
  float GyroXangle = GyroXrate * dt;
  float GyroYangle = GyroYrate * dt;
  float GyroZangle = GyroZrate * dt;
  
  // Calculate the complementary filter angles
  float CFangleX = 0.98 * (GyroXangle + AccXangle) + 0.02 * AccXangle;
  float CFangleY = 0.98 * (GyroYangle + AccYangle) + 0.02 * AccYangle;
  
  // Print the heading angle
  Serial.print("Heading Angle: ");
  Serial.print(CFangleX);
  Serial.println(" degrees");

  delay(100);
}

void calibrateMPU6050() {
  int32_t ax_sum = 0;
  int32_t ay_sum = 0;
  int32_t az_sum = 0;
  int32_t gx_sum = 0;
  int32_t gy_sum = 0;
  int32_t gz_sum = 0;
  const uint16_t numSamples = 500;

  for (uint16_t i = 0; i < numSamples; i++) {
    int16_t ax, ay, az, gx, gy, gz;  // Declarations added here
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    ax_sum += ax;
    ay_sum += ay;
    az_sum += az;
    gx_sum += gx;
    gy_sum += gy;
    gz_sum += gz;
    delay(2); // Small delay between readings
  }

  ax_offset = ax_sum / numSamples;
  ay_offset = ay_sum / numSamples;
  az_offset = az_sum / numSamples;
  gx_offset = gx_sum / numSamples;
  gy_offset = gy_sum / numSamples;
  gz_offset = gz_sum / numSamples;
}
