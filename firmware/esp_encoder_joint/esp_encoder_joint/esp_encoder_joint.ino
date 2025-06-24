/**
 * @file AS5048A_Example.ino
 * @brief Example usage of the AS5048A library
 * @date April 2025
 * 
 * This example shows how to use the AS5048A magnetic rotary encoder
 * with an ESP32 C3 mini 1 module.
 * 
 * Connections:
 * AS5048A   | ESP32 C3 mini 1
 * ---------------------
 * VDD3V     | 3.3V
 * GND       | GND
 * CLK       | SCK (GPIO6)
 * MISO      | MISO (GPIO5)
 * MOSI      | MOSI (GPIO4)
 * CSn       | GPIO7 (or any available GPIO pin)
 * PWM       | GPIO2 (optional)
 */

#include <SPI.h>
#include "AS5048.h"
#include <esp_now.h>
#include <thread>
#include <WiFi.h>


uint8_t broadcast_address[]= {0x34 ,0x85 ,0x18 ,0xac ,0xc9 ,0x18};
esp_now_peer_info_t  peerInfo;

int in_cmd = 0;

typedef struct message{
  int joint_id;
  float angle;
} message;

message send_angle;
message incoming_command;

// Define the CS pin for the AS5048A
const int CS_PIN = 20;  // GPIO7 on ESP32 C3 mini 1
const int spi_ena = 7;  // GPIO7 on ESP32 C3 mini 1

// Create an AS5048A object
AS5048A encoder(CS_PIN);



void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t sendStatus) {
  if(sendStatus ==0) {
   Serial.println("Data sent!");
  }
  else{
 Serial.println("Failed to send data");
  }
}

void OnDataRecv(const esp_now_recv_info_t *mac, const uint8_t *incoming_data, int len) {
  Serial.println("Data requested!");
  memcpy(&incoming_command, incoming_data, sizeof(incoming_command));

  float angleDegrees = encoder.readAngleDegrees();

  send_angle.angle=angleDegrees;
  send_angle.joint_id = incoming_command.joint_id;

  esp_now_send(broadcast_address, (uint8_t*)&send_angle, sizeof(send_angle));
}



void printAngleReadings() {
  // Read raw angle value (0-16383)
  uint16_t rawAngle = encoder.readRawAngle();
  
  // Read angle in degrees (0-359.99)
  float angleDegrees = encoder.readAngleDegrees();
  
  // Read angle in radians (0-2π)
  float angleRadians = encoder.readAngleRadians();
  
  // Display values
  Serial.print("Raw angle: ");
  Serial.print(rawAngle);
  Serial.print(" | Degrees: ");
  Serial.print(angleDegrees, 2);  // 2 decimal places
  Serial.print(" | Radians: ");
  Serial.print(angleRadians, 4);  // 4 decimal places
    
  // Check for errors
  if (encoder.hasError()) {
    Serial.println("Error detected in communication!");
    encoder.clearErrors();
  }
}

void printDiagnostics() {
  // Read diagnostic values
  uint8_t diag = encoder.readDiagnostics();
  uint8_t agc = encoder.readAGC();
  uint16_t magnitude = encoder.readMagnitude();
 // turn the LED off by making the voltage LOW


  Serial.println("Diagnostics:");
  Serial.print("  OCF (Offset Compensation Finished): ");
  Serial.println((diag & encoder.DIAG_OCF) ? "Yes" : "No");
  
  Serial.print("  COF (CORDIC Overflow): ");
  Serial.println((diag & encoder.DIAG_COF) ? "Yes (ERROR!)" : "No");
  
  Serial.print("  Magnetic Field: ");
  if (diag & encoder.DIAG_COMP_HIGH) {
    Serial.println("Too strong");
  } else if (diag & encoder.DIAG_COMP_LOW) {
    Serial.println("Too weak");
  } else {
    Serial.println("OK");
  }
  
  Serial.print("  AGC Value: ");
  Serial.print(agc);
  Serial.println(" (0=strong field, 255=weak field)");
  
  Serial.print("  Magnitude: ");
  Serial.println(magnitude);
}

void setPermanentZero() {
  Serial.println("WARNING: About to program PERMANENT zero position.");
  Serial.println("This operation can only be performed ONCE on the chip!");
  Serial.println("Are you sure? Type 'YES' to continue...");
  
  // Wait for input
  while (!Serial.available()) {
    delay(100);
  }
  
  String input = Serial.readString();
  input.trim();
  
  if (input == "YES") {
    Serial.println("Programming permanent zero position...");
    
    if (encoder.programZeroPosition()) {
      Serial.println("Zero position successfully programmed!");
    } else {
      Serial.println("Failed to program zero position!");
    }
  } else {
    Serial.println("Operation cancelled.");
  }
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(1000);  // Wait for serial connection
  pinMode(spi_ena, OUTPUT);
  digitalWrite(spi_ena, HIGH);  
  delay(50);  // Wait for serial connection
  // Initialize the encoder
  if (encoder.begin()) {
    Serial.println("Encoder initialized successfully");
  } else {
    
    while (1){
      Serial.println("Failed to initialize encoder!");
      delay(1000);
    }  // Halt if initialization fails
  }
  
  // Clear any previous errors
  encoder.clearErrors();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if(esp_now_init()!=0){
     while (1){
      Serial.println("Failed to initialize coms!");
      delay(1000);
    }  // 
    
  }

    esp_now_register_send_cb(OnDataSent);


    memcpy(peerInfo.peer_addr, broadcast_address, 6);
    peerInfo.channel=0;
    peerInfo.encrypt=false;

    if(esp_now_add_peer(&peerInfo)!=ESP_OK){
      return;
    }

  
  esp_now_register_recv_cb(OnDataRecv);
  // // Check magnetic field strength
  // int fieldStatus = encoder.checkMagneticField();
  // if (fieldStatus == 0) {
  //   Serial.println("Magnetic field strength: OK");
  // } else if (fieldStatus > 0) {
  //   Serial.println("Warning: Magnetic field too strong");
  // } else {
  //   Serial.println("Warning: Magnetic field too weak");
  // }
  
  // // Read automatic gain control value
  // Serial.print("AGC value: ");
  // Serial.println(encoder.readAGC());
  
  // // Read magnitude
  // Serial.print("Magnitude: ");
  // Serial.println(encoder.readMagnitude());
  

  
  // Serial.println("\nAvailable commands:");
  // Serial.println(" r - Read angle values");
  // Serial.println(" z - Set current position as zero (temporary)");
  // Serial.println(" p - Program current position as permanent zero (CAUTION!)");
  // Serial.println(" d - Read diagnostic information");
  // Serial.println(" c - Clear errors");
  
  // Serial.println("\nReading angle values...");
}


void loop() {
  Serial.println("Active...");
  delay(1000);

  // // Check for user commands
  // if (Serial.available()) {
  //   char cmd = Serial.read();
    
  //   switch (cmd) {
  //     case 'r':
  //       Serial.println("Reading angle values:");
  //       printAngleReadings();
  //       break;
        
  //     case 'z':
  //       Serial.println("Setting current position as temporary zero...");
  //       encoder.setZeroPosition();
  //       Serial.println("Done. Angle should now read close to zero.");
  //       break;
        
  //     case 'p':
  //       setPermanentZero();
  //       break;
        
  //     case 'd':
  //       printDiagnostics();
  //       break;
        
  //     case 'c':
  //       Serial.println("Clearing errors...");
  //       encoder.clearErrors();
  //       Serial.println("Done.");
  //       break;
        
  //     default:
  //       // Ignore other characters like newlines
  //       break;
  //   }
  // }
  
  // // Periodically read and display angle values
  static unsigned long lastReadTime = 0;
  if (millis() - lastReadTime > 500) {  // Update every 500ms
    printAngleReadings();
    lastReadTime = millis();
  }
}