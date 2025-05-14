#include <Stepper.h>

#define LINE_BUFFER_LENGTH 512
/// final 
// Initialize steppers for X-, Y-, and Z-axis using ULN2003 driver
const int stepsPerRevolution = 2048; // 28BYJ-48 stepper: 2048 steps/rev in full-step mode

// Define pins for ULN2003 drivers (4 pins per motor)
const int motorPinX1 = 2;  // IN1 on ULN2003 for X
const int motorPinX2 = 3;  // IN2
const int motorPinX3 = 4; // IN3
const int motorPinX4 = 5; // IN4
const int motorPinY1 = 6;  // IN1 on ULN2003 for Y
const int motorPinY2 = 7;  // IN2
const int motorPinY3 = 8;  // IN3
const int motorPinY4 = 9;  // IN4
const int motorPinZ1 = 10;  // IN1 on ULN2003 for Z
const int motorPinZ2 = 11;  // IN2
const int motorPinZ3 = 12; // IN3
const int motorPinZ4 = 13; // IN4

// Initialize Stepper objects
Stepper myStepperX(stepsPerRevolution, motorPinX1, motorPinX3, motorPinX2, motorPinX4); // Note: pin order for proper sequence
Stepper myStepperY(stepsPerRevolution, motorPinY1, motorPinY3, motorPinY2, motorPinY4);
Stepper myStepperZ(stepsPerRevolution, motorPinZ1, motorPinZ3, motorPinZ2, motorPinZ4);

/* Structures, global variables */
struct point { 
  float x; 
  float y; 
  float z;
};

struct point actuatorPos;

// Drawing settings
float StepInc = 1;
int StepDelay = 5; // Delay for ULN2003 stability (ms)
int LineDelay = 10; // Delay after each line move

// Motor steps to go 1 millimeter (calibrate based on mechanics)
float StepsPerMillimeterX = 100.0; // Example: adjust for belt/lead screw
float StepsPerMillimeterY = 100.0;
float StepsPerMillimeterZ = 100.0; // Example: 2048 steps/2mm pitch = 1024 steps/mm

// Drawing robot limits, in mm
float Xmin = 0;
float Xmax = 280;
float Ymin = 0;
float Ymax = 280;
float Zmin = -10; // E.g., pen down
float Zmax = 50; // E.g., pen up

float Xpos = Xmin;
float Ypos = Ymin;
float Zpos = Zmin;

// Set to true to get debug output
boolean verbose = true;

void setup() {
  Serial.begin(9600);

  // Set speed for ULN2003-driven 28BYJ-48 steppers (max ~15 RPM for stability)
  myStepperX.setSpeed(15); // RPM, adjust if needed
  myStepperY.setSpeed(15);
  myStepperZ.setSpeed(15);

  Serial.println("Mini CNC Plotter - ULN2003 with Stepper Library (28BYJ-48)");
  Serial.print("X range is from "); 
  Serial.print(Xmin); 
  Serial.print(" to "); 
  Serial.print(Xmax); 
  Serial.println(" mm."); 
  Serial.print("Y range is from "); 
  Serial.print(Ymin); 
  Serial.print(" to "); 
  Serial.print(Ymax); 
  Serial.println(" mm."); 
  Serial.print("Z range is from "); 
  Serial.print(Zmin); 
  Serial.print(" to "); 
  Serial.print(Zmax); 
  Serial.println(" mm."); 
  Serial.println("Steps per revolution: 2048 (full-step mode)");
}

void loop() {
  delay(100);
  char line[LINE_BUFFER_LENGTH];
  char c;
  int lineIndex = 0;
  bool lineIsComment = false, lineSemiColon = false;

  while (1) {
    while (Serial.available() > 0) {
      c = Serial.read();
      if ((c == '\n') || (c == '\r')) {
        if (lineIndex > 0) {
          line[lineIndex] = '\0';
          if (verbose) {
            Serial.print("Received: "); 
            Serial.println(line);
          }
          processIncomingLine(line, lineIndex);
          lineIndex = 0;
        }
        lineIsComment = false;
        lineSemiColon = false;
        Serial.println("ok");
      } else {
        if (lineIsComment || lineSemiColon) {
          if (c == ')') lineIsComment = false;
        } else {
          if (c == '(') lineIsComment = true;
          else if (c == ';') lineSemiColon = true;
          else if (lineIndex < LINE_BUFFER_LENGTH - 1) {
            line[lineIndex++] = c;
          }
        }
      }
    }
  }
}

/*********************************
 * Function to process G-code commands
 *********************************/
void processIncomingLine(char* line, int charNB) {
  int currentIndex = 0;
  char buffer[64];
  struct point newPos;

  newPos.x = actuatorPos.x;
  newPos.y = actuatorPos.y;
  newPos.z = actuatorPos.z;

  while (currentIndex < charNB) {
    switch (line[currentIndex++]) {
    case 'G':
      buffer[0] = line[currentIndex++];
      buffer[1] = '\0';
      switch (atoi(buffer)) {
      case 0: // Rapid positioning
      case 1: { // Linear interpolation
        char* indexX = strchr(line + currentIndex, 'X');
        char* indexY = strchr(line + currentIndex, 'Y');
        char* indexZ = strchr(line + currentIndex, 'Z');

        if (indexX) newPos.x = atof(indexX + 1);
        if (indexY) newPos.y = atof(indexY + 1);
        if (indexZ) newPos.z = atof(indexZ + 1);

        drawLine(newPos.x, newPos.y, newPos.z);
        actuatorPos.x = newPos.x;
        actuatorPos.y = newPos.y;
        actuatorPos.z = newPos.z;
        break;
      }
      }
      break;
    }
  }
}

/*********************************
 * Draw a line from (x0;y0;z0) to (x1;y1;z1)
 *********************************/
void drawLine(float x1, float y1, float z1) {
  // Constrain coordinates to machine limits
  if (x1 > Xmax) x1 = Xmax;
  if (x1 < Xmin) x1 = Xmin;
  if (y1 > Ymax) y1 = Ymax;
  if (y1 < Ymin) y1 = Ymin;
  if (z1 > Zmax) z1 = Zmax;
  if (z1 < Zmin) z1 = Zmin;

  // Convert to steps
  x1 = (int)(x1 * StepsPerMillimeterX);
  y1 = (int)(y1 * StepsPerMillimeterY);
  z1 = (int)(z1 * StepsPerMillimeterZ);

  float x0 = Xpos;
  float y0 = Ypos;
  float z0 = Zpos;

  // Calculate differences and step directions
  long dx = abs(x1 - x0);
  long dy = abs(y1 - y0);
  long dz = abs(z1 - z0);
  int sx = x0 < x1 ? StepInc : -StepInc;
  int sy = y0 < y1 ? StepInc : -StepInc;
  int sz = z0 < z1 ? StepInc : -StepInc;

  long i;
  long over = 0;

  // Find the dominant axis (largest movement)
  long maxSteps = max(max(dx, dy), dz);

  if (maxSteps == dx) { // X-axis dominant
    for (i = 0; i < dx; ++i) {
      myStepperX.step(sx);
      over += dy;
      if (over >= dx) {
        over -= dx;
        myStepperY.step(sy);
      }
      over += dz;
      if (over >= dx) {
        over -= dx;
        myStepperZ.step(sz);
      }
      delay(StepDelay);
    }
  } else if (maxSteps == dy) { // Y-axis dominant
    for (i = 0; i < dy; ++i) {
      myStepperY.step(sy);
      over += dx;
      if (over >= dy) {
        over -= dy;
        myStepperX.step(sx);
      }
      over += dz;
      if (over >= dy) {
        over -= dy;
        myStepperZ.step(sz);
      }
      delay(StepDelay);
    }
  } else { // Z-axis dominant
    for (i = 0; i < dz; ++i) {
      myStepperZ.step(sz);
      over += dx;
      if (over >= dz) {
        over -= dz;
        myStepperX.step(sx);
      }
      over += dy;
      if (over >= dz) {
        over -= dz;
        myStepperY.step(sy);
      }
      delay(StepDelay);
    }
  }

  delay(LineDelay);
  Xpos = x1;
  Ypos = y1;
  Zpos = z1;
}