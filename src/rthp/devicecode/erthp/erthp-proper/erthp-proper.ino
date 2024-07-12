/*
 * ERTHP device thing (proper!)
 * designed SPECIFICALLY for the YM2413
 * and supports a little i2c display
 * (some libraries required)
 */

// comment this line to disable display
#define USE_DISPLAY

#ifdef USE_DISPLAY
#include <OneBitDisplay.h>

// the pin for the display mode button (pullup)
#define BTN_PIN 6
// the display type (see OneBitDisplay.h, line 76)
#define DISP_TYPE OLED_128x32

OBDISP disp;
bool dispInit;

int displayMode=0;

#endif

// serial baud rate
#define SERIAL_BAUD 1000000
// YM2413 pins
#define YM_CS 7
#define YM_IC 8
#define YM_AO 9
// SN74HC595N pins
#define SN_SER 2
#define SN_OE 3
#define SN_RCLK 4
#define SN_SRCLK 5

// serial available data
int avail=0;
// serial data buffer
String buffer;
int bufIdx;
// YM2413 registers
uint8_t regPool[0x3f];

#define PW(p,v) digitalWrite(p,v);

// helper functions
void writeBus(uint8_t b) {
  PW(SN_SRCLK,0);
  PW(SN_OE,1);
  PW(SN_SRCLK,1);
  PW(SN_RCLK,0);
  for (int i=0;i<8;i++) {
    PW(SN_SRCLK,0);
    PW(SN_SER,(b>>i)&1);
    PW(SN_SRCLK,1);
  }
  PW(SN_RCLK,1);
  PW(SN_OE,0);
}

void resetYM() {
  PW(YM_IC,0);
  delay(1);
  PW(YM_IC,1);
  PW(YM_CS,1);
}

void writeYM(uint8_t a,uint8_t d) {
  PW(YM_CS,0);
  PW(YM_AO,0);
  writeBus(a);
  PW(YM_CS,1);
  PW(YM_CS,0);
  PW(YM_AO,1);
  writeBus(d);
  PW(YM_CS,1);
}

int err;

uint8_t addr, data;

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.setTimeout(10);
#ifdef USE_DISPLAY
  dispInit = false;
  err = obdI2CInit(&disp,DISP_TYPE,-1,0,0,1,-1,-1,-1,400000L);
  if (err != OLED_NOT_FOUND) {
    obdFill(&disp,0,1);
    obdWriteString(&disp,0,0,0,(char*)"ERTHP device v2.0",FONT_6x8,1,1);
    obdWriteString(&disp,0,0,8,(char*)"waiting...",FONT_6x8,1,1);
    Serial.println("initialized, waiting...");
    dispInit = true;
  } else {
    Serial.println("failed to initialize display!");
  }
  pinMode(BTN_PIN,INPUT_PULLUP);
#endif
  pinMode(YM_CS,OUTPUT);
  pinMode(YM_IC,OUTPUT);
  pinMode(YM_AO,OUTPUT);

  pinMode(SN_SER,OUTPUT);
  pinMode(SN_OE,OUTPUT);
  pinMode(SN_RCLK,OUTPUT);
  pinMode(SN_SRCLK,OUTPUT);

  resetYM();
  bufIdx=0;
  memset(regPool,0,0x3f);

}

void loop() {
  if (Serial.available()>0) {
    avail=Serial.available();
    buffer = Serial.readString();
    if ((uint8_t)buffer[0] == 0xff) {
      Serial.println("packet detected");
      switch ((uint8_t)buffer[1]) {
        case 0xf0: // RTHPPacketShort
          Serial.print("short packet: ");
          for (int i=2;i<buffer.length();i+=4) {
            addr = buffer[i];
            data = buffer[i+1];
            Serial.print(addr);
            Serial.print(":");
            Serial.println(data);
            writeYM(addr,data);
          }
          break;
        case 0xf7: // RTHPPacketInfo
          Serial.print("info packet: ");
          if (displayMode != 0) break;
          obdFill(&disp,0,1);
          String a;
          uint8_t y=0;
          for (int i=2; i<buffer.length();i++) {
            if ((uint8_t)buffer[i] == 0xff) {
              obdWriteString(&disp,0,0,y,(char*)a.c_str(),FONT_6x8,1,1);
              Serial.print(a);
              y+=8;
              a="";
            } else {
              a+=buffer[i];
            }
          }
          Serial.println();
      }
    } else {
      Serial.println(buffer);
    }

  }

}
