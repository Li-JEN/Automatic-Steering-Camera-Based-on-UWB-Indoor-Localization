/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Blink
*/
const byte addr[] = "1Node";
#include <SoftwareSerial.h> 
#include "RF24.h"
#include <SPI.h>
String S;
float dis;
String SL;
String FS;
// 定義連接藍牙模組的序列埠
SoftwareSerial BT(3, 4); // 接收腳, 傳送腳
RF24 rf24(7, 8); // CE腳, CSN腳
char val;  // 儲存接收資料的變數
char loc[10]; 
void setup() {
  // 設定HC-06藍牙模組，AT命令模式的連線速率。
  BT.begin(115200);
  Serial.begin(115200);
  rf24.begin();
  rf24.setChannel(83);       // 設定頻道編號
  rf24.openWritingPipe(addr); // 設定通道位址
  rf24.setPALevel(RF24_PA_MIN);   // 設定廣播功率
  rf24.setDataRate(RF24_250KBPS); // 設定傳輸速率
  rf24.stopListening();       // 停止偵聽；設定成發射模式

  // 若收到藍牙模組的資料，則送到「序列埠監控視窗」
}
void loop() {
    // 若收到「序列埠監控視窗」的資料，則送到藍牙模組
//  rf24.write(&angle,sizeof(angle));
  if (Serial.available()) {
    SL=Serial.readStringUntil('\n');
    SL.toCharArray(loc,sizeof(loc));
    rf24.write(&loc,sizeof(loc));
  }
 
  // 若收到藍牙模組的資料，則送到「序列埠監控視窗」
  if (BT.available()) {
    
    val = BT.read();
    if(val=='/'){
      val='.';}
    if(val == '@'){
      Serial.println(S);
      S="";}
    if(val != '@'){  
    S+=val;}
  }
}
