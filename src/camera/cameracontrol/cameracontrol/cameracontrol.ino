/*
* Copyright I3D Robotics Ltd, 2020
* Author: Ben Knight (bknight@i3drobotics.com)
*/

#define PHOBOS_USB_TRIGGER_1 2
#define PHOBOS_USB_TRIGGER_2 2

#define PHOBOS_GIGE_TRIGGER_1 3
#define PHOBOS_GIGE_TRIGGER_2 2

#define TITANIA_USB_TRIGGER_1 1
#define TITANIA_USB_TRIGGER_2 3

//Choose trigger pins based on the camera being used
#define CAMERA_TRIGGER_1 TITANIA_USB_TRIGGER_1 
#define CAMERA_TRIGGER_2 TITANIA_USB_TRIGGER_2

//Comment out line when not using the IMU
#define USE_IMU
#ifdef USE_IMU
  #include <Wire.h>
  #include <Adafruit_Sensor.h>
  #include <Adafruit_BNO055.h>
  #include <utility/imumaths.h>

  //Setup imu device
  Adafruit_BNO055 bno = Adafruit_BNO055(55);
#endif

double frame_delay;      // amount of time between triggered (1/fps)
int trigger_time = 10;   // time for trigger to be registered by camera
double fps = 5;          // inital fps
String inString = "";    // string to hold input
unsigned long lastTriggerTime = 0;
unsigned long timeSinceTrigger;
#ifdef USE_IMU
  bool imuInit = false;
#endif

void setup() {
  Serial.begin(115200);
  #ifdef USE_IMU
    // Initialise the IMU sensor
    if(!bno.begin())
    {
      // Problem detecting the BNO055 ... check your connections
      // serial will return -1000 in all axis in loop to indicate the problem
      imuInit = false;
    } else {
      bno.setExtCrystalUse(true);
      imuInit = true;
    }
  #endif
  pinMode(CAMERA_TRIGGER_1, OUTPUT);
  pinMode(CAMERA_TRIGGER_2, OUTPUT);
  pinMode(LED_BUILTIN,OUTPUT);
  
  lastTriggerTime = millis();
}

void loop() {
  // Check time since last trigger
  timeSinceTrigger = millis() - lastTriggerTime;
  // Calculate frame delay from fps (can change based on user input)
  frame_delay = 1000/fps;

  // Wait for time to trigger the camera (without stopping the program)
  if (timeSinceTrigger > frame_delay){
    // Triggger camera (pulse high and then low
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(CAMERA_TRIGGER_1, HIGH);
    digitalWrite(CAMERA_TRIGGER_2, HIGH);
    lastTriggerTime = millis();
    delay(trigger_time); // Wait small time for high to be registered by camera
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(CAMERA_TRIGGER_1, LOW);
    digitalWrite(CAMERA_TRIGGER_2, LOW);
  }
  
  // Read trigger frame rate from serial
  if (Serial.available() > 0) { //Only read if data is available
    int inChar = Serial.read();
    if (isDigit(inChar)) {
      // Convert the incoming byte to a char and add it to the string:
      inString += (char)inChar;
    }
    // If you get a newline, print the string, then the string's value:
    if (inChar == '\n') {
      fps = inString.toInt();
      // clear the string for new input:
      inString = "";
    }
  #ifdef USE_IMU
  } else {
    if (imuInit){
      // Get a new IMU sensor event
      sensors_event_t event; 
      bno.getEvent(&event);
      
      // Display the floating point data
      Serial.print(event.orientation.x, 4);
      Serial.print(",");
      Serial.print(event.orientation.y, 4);
      Serial.print(",");
      Serial.print(event.orientation.z, 4);
      Serial.println();
    } else {
      // IMU failed to initalise return -1000 in all axis
      Serial.println("-1000,-1000,-1000");
    }
  }
  #else
  }
  #endif
}
