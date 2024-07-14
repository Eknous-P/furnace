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

// the display type (see OneBitDisplay.h, line 76)
#define DISP_TYPE OLED_128x32

OBDISP disp;
bool dispInit;
String info[2];

uint8_t displayMode=0;

void volumeBar(uint8_t x, uint8_t v) {
  obdRectangle(&disp,1+x,16-v,6+x,16,0,1);
}

#endif

// serial baud rate
#define SERIAL_BAUD 1000000
// YM2413 pins
#define YM_CS 7
#define YM_IC 8
#define YM_AO 9
// SN74HC595N pins
#define SN_SER 3
#define SN_OE 4
#define SN_RCLK 5
#define SN_SRCLK 6

// serial available data
int avail=0;
// serial data buffer
String buffer;
int bufIdx;
// YM2413 registers
uint8_t regPool[0x40];

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

uint8_t key2;
uint8_t addr, data, param;
bool running;

void setup() {
  running=false;
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
  memset(regPool,0,0x40);
  param=0;
  key2=0;

}

void loop() {
  if (Serial.available()>0) {
    running=true;
    avail=Serial.available();
    buffer = Serial.readString();
    if ((uint8_t)buffer[0] == 0xff) {
      Serial.print("packet detected: ");
      key2=(uint8_t)buffer[1];
      Serial.println(key2);
      if (key2 == 0xf0) {
        Serial.print("short packet: ");
        for (int i=2;i<buffer.length();i+=4) {
          addr = buffer[i];
          data = buffer[i+1];
          Serial.print(addr);
          Serial.print(":");
          Serial.println(data);
          writeYM(addr,data);
          regPool[addr&0x3f] = data;
        }
      } else if (key2 == 0xf7) {
        Serial.print("info packet: ");
#ifdef USE_DISPLAY
        String a;
        uint8_t y=0;
        uint8_t iter=0;
        for (int i=2; i<buffer.length();i++) {
          if ((uint8_t)buffer[i] == 0xff) {
            info[iter]=a;
            Serial.print(a);
            y+=8;
            a="";
            iter++;
          } else {
            a+=buffer[i];
          }
        }
        if (displayMode == 0) refreshDisplay();
#endif
        Serial.println();
      } else if (key2 == 245) {
        Serial.print("param packet: ");
        param = (uint8_t)buffer[2];
        Serial.println(param);
        switch (param) {
#ifdef USE_DISPLAY
          case 0x7f: //display mode
            Serial.print("display mode: ");
            displayMode = (uint8_t)buffer[3];
            Serial.print(displayMode);
            refreshDisplay();
            break;
#endif
          default: break;
        }
        Serial.println();
      } else {
        Serial.print(buffer);
      }
    }
  }
}
#ifdef USE_DISPLAY
void refreshDisplay() {
  if (running) {
    // Serial.println(displayMode);
    obdFill(&disp,0,1);
    switch (displayMode) {
      case 0:
        obdWriteString(&disp,0,0,0,(char*)info[0].c_str(),FONT_6x8,1,1);
        obdWriteString(&disp,0,0,8,(char*)info[1].c_str(),FONT_6x8,1,1);
      case 1:
        uint8_t vol;
        for (uint8_t i=0; i<9;i++) {
          vol=regPool[0x30+i]&0xf;
          volumeBar(i*8,vol);
        }
        break;
      default: break;
    }
  }
}
#endif

