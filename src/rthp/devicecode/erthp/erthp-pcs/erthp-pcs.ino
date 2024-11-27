/*
 * ERTHP PC Speaker thing

 * NOTE: only works with LEGACY packets!

 * yes, it uses boring tone()/notone() for sound
 * and """clocked""" the same as ibm pc
 * (sorry, cbt is too much for this)
 * (btw: acutally faster than furnace emulation; negative lag!!!)
 */

#define BYTE unsigned char

#define SERIAL_BAUD 1000000
#define PCS_CLK 105000000.0/88.0

// output pin
#define OUT 4

#define PW(p,v) digitalWrite(p,v);

#define KEYCHR '>'

BYTE i; // iterator
BYTE ADDR,DATA;

int avail=0;

// 0 - key
// 1 - data
// 2 - address low
// 3 - address high
unsigned char whichByte=0;
char *buffer;
unsigned short fnum;



void setup() {

  Serial.begin(SERIAL_BAUD);

  pinMode(OUT,OUTPUT);

  fnum=0;
  buffer = new char[256];
}

void loop(){
  if (Serial.available()>0) {
    avail=Serial.available();
    Serial.readBytes(buffer,avail);
    for (i=0; i<avail; i++) {
      switch (whichByte) {
        case 0:
          whichByte = (buffer[i] == KEYCHR)?0:255;
          break;
        case 1:
          DATA=buffer[i];
          break;
        case 2:
          ADDR=buffer[i];
          switch (ADDR) {
            case 0:
              fnum&=0xff00;
              fnum|=DATA;
              break;
            case 1:
              fnum&=0x00ff;
              fnum|=(DATA<<8)&0xff00;
              break;
            default: break;
          }
          break;
        case 3:
          break;
      }
      // debug info
      Serial.print((int)buffer[i]);
      Serial.print( " - whichByte: ");
      Serial.print((int)whichByte);
      Serial.print(" data: ");
      Serial.print((int)DATA);
      Serial.print(" addr: ");
      Serial.print((int)ADDR);
      Serial.print(" freq: ");
      Serial.println(fnum);

      whichByte++; whichByte&=0b11;
    }
  }
  if (fnum) {
    tone(OUT,PCS_CLK/fnum);
  } else {
    noTone(OUT);
  }
}
