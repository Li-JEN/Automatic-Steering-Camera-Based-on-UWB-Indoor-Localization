/**
 * 
 * @todo
 *  - move strings to flash (less RAM consumption)
 *  - fix deprecated convertation form string to char* startAsTag
 *  - give example description
 */
#include <SPI.h>
#include "DW1000Ranging.h"
#include <SoftwareSerial.h>   // 引用程式庫
// connection pins
const uint8_t PIN_RST = 9; // reset pin
const uint8_t PIN_IRQ = 2; // irq pin
const uint8_t PIN_SS = SS; // spi select pin
int devicenum=0;
int ID;
int VanishID;
float Range,Power;
String A_string = "2.0",B_string = "2.0", C_string = "2.0", D_string = "2.0", final_string = "2.0,2.0,2.0,2.0,2.0";
float a_arr[10] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float b_arr[10] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float c_arr[10] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float d_arr[10] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float a_avg=0.0 ,b_avg = 0.0 , c_avg = 0.0 , d_avg = 0.0 ;
unsigned int current_time =0, last_time = 0; 
// 定義連接藍牙模組的序列埠
SoftwareSerial BT(3, 4); // 接收腳, 傳送腳
void setup() {
  Serial.begin(115200);
  //Serial.println("BT is ready!");
  BT.begin(115200);
  delay(1000);
  //init the configuration
  DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ); //Reset, CS, IRQ pin
  //define the sketch as anchor. It will be great to dynamically change the type of module
  DW1000Ranging.attachNewRange(newRange);
  DW1000Ranging.attachNewDevice(newDevice);
  DW1000Ranging.attachInactiveDevice(inactiveDevice);
  //Enable the filter to smooth the distance
  //DW1000Ranging.useRangeFilter(true);
  //we start the module as a tag
  DW1000Ranging.startAsTag("EE:00:22:EA:82:60:3B:9C", DW1000.MODE_LONGDATA_RANGE_LOWPOWER,false);
}

void loop() {
 DW1000Ranging.loop();
 current_time=millis();
  if(devicenum>=1 && (current_time-last_time)>400){
    show();
    last_time=current_time;
    }
}

void newRange() {
  ID=DW1000Ranging.getDistantDevice()->getShortAddress();
  Range=DW1000Ranging.getDistantDevice()->getRange();
  //Power=DW1000Ranging.getDistantDevice()->getRXPower();
  if(ID==170){if(abs(Range-a_arr[9])<10){updatearray(170);a_arr[9]=Range;}}
  if(ID==187){if(abs(Range-b_arr[9])<10){updatearray(187);b_arr[9]=Range;}}
  if(ID==204){if(abs(Range-c_arr[9])<10){updatearray(204);c_arr[9]=Range;}}
  if(ID==221){if(abs(Range-d_arr[9])<10){updatearray(221);d_arr[9]=Range;}}
}

void newDevice(DW1000Device* device) {
  //Serial.print("ranging init; 1 device added ! -> ");Serial.print(" short:");Serial.println(device->getShortAddress(), HEX);
  //BT.print("ranging init; 1 device added ! -> ");BT.print(" short:");BT.println(device->getShortAddress(), HEX);
  devicenum++;
}

void inactiveDevice(DW1000Device* device) {
  //Serial.print("delete inactive device: ");Serial.println(device->getShortAddress(), HEX);
  //BT.print("delete inactive device: ");BT.println(device->getShortAddress(), HEX);
  VanishID=device->getShortAddress();
  if(VanishID==170){zero(170);}
  if(VanishID==187){zero(187);}
  if(VanishID==204){zero(204);}
  if(VanishID==221){zero(221);}
  devicenum--;
}
void updatearray(int ID){
  if(ID==170){
    for (int i=1;i<=9;i++){
      a_arr[i-1] = a_arr[i];}}
  if(ID==187){
    for (int i=1;i<=9;i++){
      b_arr[i-1] = b_arr[i];}}
  if(ID==204){
    for (int i=1;i<=9;i++){
      c_arr[i-1] = c_arr[i];}}
  if(ID==221){
    for (int i=1;i<=9;i++){
      d_arr[i-1] = d_arr[i];}  
  }
}

void zero(int ID){
  if(ID==170){
    for (int i=0;i<=9;i++){
      a_arr[i] = 0.0;}}
  if(ID==187){
    for (int i=0;i<=9;i++){
      b_arr[i] = 0.0;}}
  if(ID==204){
    for (int i=0;i<=9;i++){
      c_arr[i] = 0.0;}}
  if(ID==221){
    for (int i=0;i<=9;i++){
      d_arr[i] = 0.0;}}
}
int count(float x[10]){
  int num=0;
  for(int i=3;i<10;i++){
    if(x[i]!=0.0){num=num+(i-2);}}
  if (num==0){return 1;}
  return num;
}
void show(){
  a_avg=0;b_avg=0;c_avg=0;d_avg=0;
  for(int times=3;times<=9;times++){
    a_avg+=(times-2)*a_arr[times];
    b_avg+=(times-2)*b_arr[times];
    c_avg+=(times-2)*c_arr[times];
    d_avg+=(times-2)*d_arr[times];
    }
   a_avg=a_avg/count(a_arr);
   b_avg=b_avg/count(b_arr);
   c_avg=c_avg/count(c_arr);
   d_avg=d_avg/count(d_arr);
  A_string=String(a_avg);B_string=String(b_avg);C_string=String(c_avg);D_string=String(d_avg);
  final_string=A_string+","+B_string+","+C_string+","+D_string;
  Serial.println(final_string);
  BT.print(final_string);BT.print('@');
 }
