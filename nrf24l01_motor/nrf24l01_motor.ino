
#include <SPI.h>
#include "RF24.h"
#include <Servo.h> 
RF24 rf24(7, 8); // CE腳, CSN腳
const byte addr[] = "1Node";
const byte pipe = 1;  // 指定通道編號
float Rotate_angle=0;
String Ls;
//float loc_cameraX= 4.25;
//float loc_cameraY= 6; //318
float loc_cameraX= 3.8;
float loc_cameraY= 0.8; //503
int index;
float A,B,C,loc_X,loc_Y,calangle,preangle=90;
float Ax =loc_cameraX+7;
float Ay =loc_cameraY;
unsigned int last_sec=0;
void servoangle(float angle){
  //0.5-2.4 ms
  float r=1900/180;
  float pwmdutysec= 0.5+r/1000*angle;
  float duty=pwmdutysec/4.10;
  float comparevalue=256*duty-1;
  OCR2B=comparevalue;// compare with TCCR controll pin 11 ;
//  OCR2B =  20  compare with TCCR controll pin 3;
//  sSerial.println(angle); 
//  Serial.println(comparevalue); 
  }
void setup() {
  Serial.begin(115200);
  rf24.begin();
  rf24.setChannel(83);  // 設定頻道編號
  rf24.setPALevel(RF24_PA_MIN);
  rf24.setDataRate(RF24_250KBPS);
  rf24.openReadingPipe(pipe, addr);  // 開啟通道和位址
  rf24.startListening();  // 開始監聽無線廣播
  pinMode(3, OUTPUT);   //B;  
  pinMode(11, OUTPUT); //A;  
  TCCR2A = 0b10100011;   //fast PWM,non invert-pwm modeS
  TCCR2B = 0b00000110;  //prescaler=64 basic frequency 16M/256= 62500 8-bit -> 62500/256= 244.14 hz => 4.10 ms
  servoangle(90);
  delay(3000);
}
 
void loop() {
  if (rf24.available(&pipe)) {
    char Array_loc[10]="";
    rf24.read(&Array_loc, sizeof(Array_loc));
    Ls=Array_loc;
    index=Ls.indexOf('/');
    loc_X=Ls.substring(0,index).toFloat();
    loc_Y=Ls.substring(index+1).toFloat();  
    A=7;
    B=sqrt(sq(loc_X-loc_cameraX)+sq(loc_Y-loc_cameraY));
    C=sqrt(sq(loc_X-Ax)+sq(loc_Y-Ay));
    calangle=acos((sq(A)+sq(B)-sq(C))/(2*A*B));
    calangle=calangle*180/PI;
    while ((rf24.available(&pipe) == 0) && (preangle != int(calangle))){
      if (preangle > calangle){
        preangle--;
        servoangle(preangle);
        delay(10);
        }
    else{
      preangle++;
      servoangle(preangle);
      delay(10);
      } //待確認哪邊為正
    }
  }
}
