#include <Servo.h>

Servo myServo;
const int level = 113;
const int safeDelta = 30;
String readLineBuffer = "";

// the setup function runs once when you press reset or power the board
void setup() {
  myServo.attach(3);
  myServo.write(level);
  
  Serial.begin(9600);
  
  // Flush all unread incoming data
  if (Serial.available()) {
    // Flush all unread incoming data
    Serial.read();
  }
  
  // Print banner
  Serial.println("BeatTheBot Arduino Controller v0.1 is ready!");
}

// the loop function runs over and over again forever
void loop() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n') {
      int angle = readLineBuffer.toInt();

      // Safety
      if (angle >= level - safeDelta && angle <= level + safeDelta) {
        myServo.write(angle);
      }
      readLineBuffer = "";
    } else {
      readLineBuffer += c;
    }
  }
}

