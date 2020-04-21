#include <SPI.h>
#include <mcp2515.h>
#include "main.h"
#include <Arduino.h>






void SET_SPEED(void) {
  
  if (incoming_ref_speed > 230) {
    incoming_ref_speed = 230.0;
  }
  else if (incoming_ref_speed < 25) {
    incoming_ref_speed = 25.0;
  }
  // take the SS pin low to select the chip:
  digitalWrite(SS, LOW);
  //  send in the address and value via SPI:
  SPI.transfer((byte)0); // address
  SPI.transfer((byte)incoming_ref_speed + 20); // +20 due to neutral position iт PALESSE FS8060 isn't 256/2=127 but it is 147
  if (incoming_ref_speed> 125 && incoming_ref_speed< 129) {
    digitalWrite(relay2, HIGH);
  }
  else {
    digitalWrite(relay2, LOW);
  }
  // take the SS pin high to de-select the chip:
  digitalWrite(SS, HIGH);
}



void GET_HANDLE(void)
{
  handle_pos = (int)(analogRead(signal_pin) * 0.25);
}

void GET_WIPER_POS(void)
{
  wiper_pos=analogRead(wiper)*0.25;
}

void SET_MANUAL(void)
{
  digitalWrite(relay1, LOW);
  current_mode = "manual";
}

void SET_ROBOT()
{
  digitalWrite(relay1, HIGH);
  current_mode = "robot";

}


void SEND_FOR_HUMAN()
{
  if (millis()-prevSendTime>300){
    prevSendTime=millis();
    Serial.print((String)"Handle position: " + handle_pos + ";Pot. value: " + incoming_ref_speed + ";Actual pot.value: "+wiper_pos+";Neutral: " + digitalRead(neutral) + ";Mode: " + current_mode + ";Speed from CAN1: " + String(speedFbCAN) + ";Speed from CAN2: " + String(speedFbCAN_1) +";err: "+errorNames[status]+ "\n");
  }
}
/*
void SEND_FOR_PLOTTER(){
  Serial.print(incoming_ref_speed);
  Serial.print(" ");
  Serial.println(wiper_pos);
}
*/
void readCAN()
{
   if (interrupt)
   {
    if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK)
     { // && (canMsg.can_id == 0x98FF0102) ) {
        speedFbCAN = 0.2 * canMsg.data[5];
    }
     interrupt--;
   }
}
void irqHandler()
{
  interrupt++;
}
void sendCAN()
{
  if (millis()-canSendTimer>10){
    canSendTimer=millis();
    mcp2515_1.sendMessage(&canMsgToSend);
  }
}

int is_in_range(int w){
  if (w>(incoming_ref_speed+18)&&w<(incoming_ref_speed+20)){
    return 1;
  }
  else {
    errCounter++;
    return 0;
  }
}

void err_counter_check(){
  if ((millis()-errTimer)>500){
    errTimer=millis();
    if (errCounter>2){
      status=ERR;
    }
    else 
    {
      errCounter=0;
      status=NORMAL;
    }
  }
}


//////////////////////////////////////////////////////////////////////SETUP//////////////////////////////////////////////////////////////////////

void setup() {

  SPI.begin();
  delay(200);
  pinMode(SS, OUTPUT); // switch chip select pin to output mode
  pinMode(relay1, OUTPUT); // switch relay 1 pin to output mode
  pinMode(relay2, OUTPUT);
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  pinMode(neutral, INPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), irqHandler, FALLING);
  digitalWrite(neutral, HIGH);
  mcp2515.reset();
  mcp2515.setFilterMask(MCP2515::MASK0, true, 0x1FFFFFFF);
  mcp2515.setFilterMask(MCP2515::MASK1, true, 0x1FFFFFFF);
  mcp2515.setFilter(MCP2515::RXF0, true, 0x18FF0202);
  mcp2515.setFilter(MCP2515::RXF1, true, 0x18FF0202);
  mcp2515.setFilter(MCP2515::RXF2, true, 0x18FF0202);
  mcp2515.setFilter(MCP2515::RXF3, true, 0x18FF0202);
  mcp2515.setFilter(MCP2515::RXF4, true, 0x18FF0202);
  mcp2515.setFilter(MCP2515::RXF5, true, 0x18FF0202);
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  // mcp2515_1.reset();
  // mcp2515_1.setBitrate(CAN_500KBPS, MCP_8MHZ);
  // mcp2515_1.setNormalMode();
  
  
  Serial.begin(115200);
  Serial.setTimeout(10);
  while (!Serial) {
    ;
  }

  prevSendTime = millis();
  errTimer = millis();
  canMsgToSend.can_id=0x12345678;
canMsgToSend.can_dlc=8;
canMsgToSend.data[0]=0xFF;
canMsgToSend.data[1]=0xFF;
canMsgToSend.data[2]=0xFF;
canMsgToSend.data[3]=0xFF;
canMsgToSend.data[4]=0xFF;
canMsgToSend.data[5]=0xFF;
canMsgToSend.data[6]=0xFF;
canMsgToSend.data[7]=0xFF;
}

//////////////////////////////////////////////////////////////////////LOOP//////////////////////////////////////////////////////////////////////


void loop() {

  // Read serial input:
  if (Serial.available() > 0) {
    incoming_mes = Serial.readString();
    if (incoming_mes == "restart") {
      errCounter=0;
      status=NORMAL;
    }
    else{
      incoming_ref_speed = incoming_mes.substring(0, 3).toInt();
      incoming_mode = incoming_mes.substring(3, 4);
      if (incoming_mode == "m") {
        SET_MANUAL();
      }
      if (incoming_mode == "r") {
        SET_ROBOT();
      }
    }
  }
  readCAN();
  sendCAN();
  GET_HANDLE();
  SEND_FOR_HUMAN();
  SET_SPEED();
  GET_WIPER_POS();
  is_in_range(wiper_pos);
  err_counter_check();
  
}