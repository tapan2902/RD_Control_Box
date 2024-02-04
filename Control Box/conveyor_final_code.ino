
#include <MCUFRIEND_kbv.h>

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define CW  HIGH
#define CCW  LOW

#define current_sens A9
#define voltage_sens A8
#define rpm_sens 19
#define en_button  31    // Button pin
#define EN_pin  39    // Motor enable pin
#define DIR_pin  37 // Motor direction pin (F/R pin)
#define rightIR_sens  33   // Right IR sensor pin
#define leftIR_sens  35    // Left IR sensor pin


MCUFRIEND_kbv tft;


boolean motorEnabled = false;


//input voltage , current & Motor_RPM 
float voltage;
float current;


//current_sensor variables 
int current_sensor_value = 0;
float current_sensor_voltage = 0;
float current_reading = 0.0;
float mVperAmp = 185;        // Sensitivity of the ACS712 sensor 

//voltage_sensor variables 
float adc_voltage = 0.0;
float ref_voltage = 5;
float adc2_voltage = 0.0;
float voltage_reading = 0.0;
float r1 = 35600.0;
float r2 = 7500.0;
float r3 = 10000.0;
float r4 = 10000.0;
int adc_value = 0;

const int motorPoles = 4; // Number of poles in the motor

volatile unsigned long pulseCount = 0;

/*float voltage =24.5;
float current = 0.25;
float motor_RPM = 4125;
float RPM = motor_RPM/64; // motor rpm / ratio
float Speed = (3.14*0.076*RPM)/60;
*/
int screenHeight;
int screenWidth;
unsigned long lastUpdateTime = 0; // Variable to store the last update time
const unsigned long updateInterval = 500; // Update every 1000 milliseconds (1 second)

void setup()
{
  Serial.begin(9600);
  uint16_t ID=tft.readID();
  tft.begin(ID);
  tft.setRotation(1);
  tft.fillScreen(BLACK);
  screenWidth = tft.width();//480
  screenHeight = tft.height();//320
  draw_structure();
  attachInterrupt(digitalPinToInterrupt(rpm_sens), countPulses, CHANGE);
  pinMode(en_button, INPUT_PULLUP);  // Internal pull-up resistor for the button
  pinMode(EN_pin, OUTPUT);         // Motor enable pin
  pinMode(DIR_pin, OUTPUT);      // Motor direction pin
  pinMode(rightIR_sens, INPUT);        // Right IR sensor pin
  pinMode(leftIR_sens, INPUT);         // Left IR sensor pin

  // Ensure the motor starts in a disabled state with the initial direction set
  disableMotor();

}

void loop()
{
  unsigned long currentTime = millis();
  draw_structure();
  // Read the state of the button and IR sensors
  int buttonState = digitalRead(en_button);
  int rightIRState = digitalRead(rightIR_sens);
  int leftIRState = digitalRead(leftIR_sens);
  handleButton(buttonState);
  handleIRSensors(rightIRState, leftIRState);
  if (currentTime - lastUpdateTime >= updateInterval)
  {
    tft.fillRect(15, 55, 100, 50, BLACK);   // Clear Voltage field
    tft.fillRect(177, 55, 100, 50, BLACK);  // Clear Current field
    tft.fillRect(339, 55, 100, 50, BLACK);  // Clear Power field
    tft.fillRect(15, 155, 120, 50, BLACK);  // Clear RPM field
    tft.fillRect(270, 155, 120, 50, BLACK); // Clear Speed field
    print_supply_para(read_voltage(),read_current());  
  }
  unsigned long currentRPM = measureRPM();
  print_motor_para(currentRPM);
  Serial.println(currentRPM);
  
  tft.fillCircle(40,283,20,GREEN);//circle 1
  tft.fillCircle(110,283,20,RED);//circle 2
  
}

void draw_structure()
{
  // Draw border
    tft.drawRect(0, 0, screenWidth - 1, screenHeight - 1, 0xFFFF);
  // Draw the first horizontal line
  
  int line1Y = screenHeight / 3;//106.6
  
  tft.drawLine(0, line1Y, screenWidth, line1Y,WHITE);//0,42.67,128,42.67

  int line1Y1 = 35;//line1Y/2;//53.33
  tft.drawLine(0,line1Y1,screenWidth,line1Y1,WHITE);

  int line2Y1 = 145;//line1Y/2;//53.33
  tft.drawLine(0,line2Y1,screenWidth,line2Y1,WHITE);
  
  // Draw the second horizontal line
  int line2Y = 2 * screenHeight / 3; //85.33
  tft.drawLine(0, line2Y, screenWidth, line2Y,WHITE);//0,85.33,120,85.33

    int line1X = screenWidth / 3;//53.33
  tft.drawLine(line1X, 0, line1X, line1Y,WHITE);//53.33,0,53.33,42.67

    int line2X = 2*screenWidth/3;//106.6
  tft.drawLine(line2X,0,line2X,line1Y,WHITE);//106.6,0,106.6,42.67

    int line4X= screenWidth / 2;//80
    tft.drawLine(line4X,line2Y,line4X,line2Y,WHITE);
    tft.drawLine(line1X,line2Y,line1X,screenHeight,WHITE);
    tft.drawLine(line4X,145,line4X,line2Y,WHITE);  
    
    tft.drawLine(0,250,screenWidth,250,WHITE);
  //TFTscreen.line(0,line1Y/2,lineX,line1Y/2);
  tft.setTextColor(WHITE);
  tft.setTextSize(4);
  tft.setCursor(125,55);
  tft.print("V");
  tft.setCursor(287,55);
  tft.print("A");
  tft.setCursor(449,55);
  tft.print("W");
  tft.setCursor(160,165);
  tft.print("RPM");
  tft.setCursor(400,165);
  tft.print("m/s");
   tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setCursor(38,10);
  tft.print("Voltage");
  tft.setCursor(200,10);
  tft.print("Current");
  tft.setCursor(368,10);
  tft.print("Power");
  tft.setCursor(200,120);
  tft.print("Speed");
  tft.setCursor(45,225);
  tft.print("Sensor");
  tft.setCursor(280,225);
  tft.print("Mfg. By");
  tft.setTextColor(WHITE);
  tft.setTextColor(CYAN);
  tft.setTextSize(4);
  tft.setCursor(200,270);
  tft.print("RollnDrive");
  
  
}
void print_supply_para(float p_voltage,float p_current)
{
  tft.setTextColor(WHITE);
  tft.setTextSize(4);
  tft.setCursor(15,55);
  tft.print(String(p_voltage,1).c_str());
  tft.setCursor(177,55);
  tft.print(String(p_current,2).c_str());
  tft.setCursor(339,55);
  tft.print(String(p_voltage*p_current,1).c_str());
  tft.setCursor(15,165);
  
  
  
  
}
void print_motor_para(unsigned long rpm)
{
  unsigned long a = rpm;
  //float a=rpm/ratio;
  tft.print(String(a).c_str());
  unsigned long b=(3.14*0.076*a)/60;
  tft.setCursor(270,165);
  tft.print(String(b).c_str());
    
}

float read_current()
{
  current_sensor_value = analogRead(current_sens);
  current_sensor_voltage = current_sensor_value * (5.0 / 1023.0) * 1000;
  current_reading = (current_sensor_voltage - 2500) / mVperAmp;
  return abs(current_reading);
 
}

float read_voltage()
{
  adc_value = analogRead(voltage_sens);
  adc_voltage = (adc_value*ref_voltage) / 1024.0;
  adc2_voltage = adc_voltage*(r1+r2)/r2;
  voltage_reading = adc2_voltage*(r3+r4)/r4;
  return voltage_reading;
}

unsigned long measureRPM() 
{
  // Update RPM every second
  delay(500);
  
  // Calculate RPM using the formula N=(F/P)*60/3
  noInterrupts(); // Disable interrupts to prevent race condition
  unsigned long rpm = (pulseCount / motorPoles) * 60 / 3;
  pulseCount = 0; // Reset pulse count
  interrupts(); // Enable interrupts
  
  return rpm;
}



void countPulses()
{
  // This function is called every time a falling edge is detected on the sensor pin
  pulseCount++;
}


void handleButton(int state)
{
  // Check if the button is pressed (LOW)
  if (state == LOW) {
    // Toggle the motor state
    motorEnabled = !motorEnabled;

    // Enable or disable the motor based on the state
    digitalWrite(EN_pin, motorEnabled ? LOW : HIGH);

    // Print motor state to Serial Monitor
    //Serial.print("Motor State: ");
    //Serial.print(motorEnabled ? "Enabled" : "Disabled");
  }
}

void handleIRSensors(int rightState, int leftState)
{
  if (rightState == LOW && leftState == HIGH)
  {
    // Object detected on the right, rotate CW
    setMotorDirection(CW);
    tft.fillCircle(40,283,20,GREEN);//circle 1
    tft.fillCircle(110,283,20,RED);//circle 2
    //Serial.print(", Object detected on the right! Rotating CW.");
  } 
  else if (rightState == HIGH && leftState == LOW) 
  {
    // Object detected on the left, rotate CCW
    setMotorDirection(CCW);
    tft.fillCircle(40,283,20,RED);//circle 1
    tft.fillCircle(110,283,20,GREEN);//circle 2
    //Serial.print(", Object detected on the left! Rotating CCW.");
  } 
  else if (rightState == HIGH && leftState == HIGH)
  {
    tft.fillCircle(40,283,20,RED);//circle 1
    tft.fillCircle(110,283,20,RED);//circle 2
    //Serial.print(", No Object Detected");
    // If needed, add additional logic for handling the case when no object is detected
  }
  
  // Print motor direction and enable state on the Serial Monitor
  //Serial.print(", Motor Direction: ");
  //Serial.print(digitalRead(DIR_pin) == CW ? "CW" : "CCW");
  //Serial.print(", Motor State: ");
  //Serial.println(motorEnabled ? "Enabled" : "Disabled");
}

void disableMotor() 
{
  motorEnabled = false;
  digitalWrite(EN_pin, HIGH);  // Disable the motor
  setMotorDirection(CW);    // Set the initial motor direction (CW)
}

void setMotorDirection(int direction)
{
  // Set the motor direction based on the provided parameter
  digitalWrite(DIR_pin, direction);
}
