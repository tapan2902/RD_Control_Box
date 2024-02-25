#include <TFT.h>
#include <SPI.h>

#define cs   10
#define dc   9
#define rst  8

TFT TFTscreen = TFT(cs, dc, rst);

float voltage =24.5;
float current = 0.25;
float motor_RPM = 4125;
float RPM = motor_RPM/64; // motor rpm / ratio
float Speed = (3.14*0.076*RPM)/60;
int screenHeight;
int screenWidth;
void setup() {
  Serial.begin(9600);

   TFTscreen.begin();
  TFTscreen.setRotation(3);
  TFTscreen.background(0, 0, 0); // Set background color (black)
  TFTscreen.setTextSize(2);
}

void drawLines() {
  // Set the line color to white
  TFTscreen.stroke(255, 255, 255);

  screenHeight = TFTscreen.height();//128
  screenWidth = TFTscreen.width();//160

  // Draw the first horizontal line
  int line1Y = screenHeight / 3;//42.67
  TFTscreen.line(0, line1Y, screenWidth, line1Y);//0,42.67,128,42.67

  // Draw the second horizontal line
  int line2Y = 2 * screenHeight / 3; //85.33
  TFTscreen.line(0, line2Y, screenWidth, line2Y);//0,85.33,120,85.33

  //TFTscreen.line(0, 98, screenWidth, 98);//0,85.33,120,85.33
  
    
    int line4X= screenWidth / 2;//80
    TFTscreen.line(line4X,line2Y,line4X,screenHeight);

  //TFTscreen.line(0,line1Y/2,screenHeight,line1Y/2);
  

}


void print_value(int a)
{
  TFTscreen.setTextSize(3);
  TFTscreen.stroke(255,255,255);
  //TFTscreen.text(String(voltage,1).c_str(),2,4);
  //TFTscreen.text("V",20,22);
  //TFTscreen.text(String(current,2).c_str(),56,4);
  //TFTscreen.text("A",74,22);
  //TFTscreen.text(String(voltage*current,2).c_str(),110,4);
  //TFTscreen.text("W",128,22);
  TFTscreen.text(String(round(RPM)).c_str(),30,8);//RPM/ratio
  TFTscreen.setTextSize(2);
  TFTscreen.text("RPM",110,10);
  TFTscreen.setTextSize(3);
  TFTscreen.text(String(Speed).c_str(),25,52);
  TFTscreen.setTextSize(2);
  TFTscreen.text("m/s",115,58);
  TFTscreen.setTextSize(1);
  TFTscreen.text("Sens",24,90);
  TFTscreen.setTextSize(1);
  TFTscreen.text("mfg. by",104,90);
  if(a==1)
  {
    TFTscreen.fill(0,255,0);
    TFTscreen.circle(20,110,8);
    TFTscreen.fill(0,0,255);
    TFTscreen.circle(55,110,8);
  }
  else if(a==0)
  {
    TFTscreen.fill(0,0,255);
  TFTscreen.circle(20,110,8);
  TFTscreen.fill(0,255,0);
  TFTscreen.circle(55,110,8);
  
  }
  TFTscreen.setTextSize(1);
  TFTscreen.stroke(255,255,0);
  TFTscreen.text("RollNDrive",90,103);
  
  
}

void loop() {
  drawLines();
  print_value(1);
  }
