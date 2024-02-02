// primitive ERTHP client
// for Arduino Nano (and compatible) boards
// TODO: test
// no pinout yet btw

#define SBAUD 57600 // serial baudrate

#define RS 10 // chip reset
#define CS 12 // chip select
#define AD 13 // address write
#define CT 11 // chip type sense
              // HIGH/FLOATING - separate address/data buses (only allows an 8 bit long addr bus!)
              // LOW - unified address/data bus

#define DBI 2 // data bus index (1st bit)
#define ABI 14 // addr bus index

// function macros

#define PW(p,v) digitalWrite(p,v)
#define BUSW(i,d) for (int z=0; z<8; z++) PW(z+i, (d&(1<<z)) )


char *buf;
unsigned short a,v;
bool chipType;

void setup() {
  pinMode(RS, OUTPUT);
  PW(RS,0);

  Serial.begin(SBAUD);
  Serial.setTimeout(10); // 100Hz max?

  pinMode(CT, INPUT_PULLUP);
  chipType=digitalRead(CT);
                                            // chip type: separate | unified
  for (int i=ABI; i<ABI+8; i++) pinMode (i, OUTPUT); // addr range | bus range low byte
  for (int i=DBI; i<DBI+8; i++) pinMode(i, OUTPUT);  // data range | bus range high byte

  pinMode(AD, OUTPUT);
  pinMode(CS, OUTPUT);

  PW(RS,1);
  PW(CS,1);
}

void loop() {
  if (Serial.available()) {
    Serial.readBytes(buf,Serial.available());
    int i=0;
    while (i<sizeof(buf)) { // read "packets"
      Serial.write(buf[i]);
      Serial.write(buf[i+1]);
      Serial.write(buf[i+2]);
      Serial.write(buf[i+3]);
      if (buf[i]!='>') { // invalid / error in transmission
        i++;
        continue;
      }
      PW(AD,1);

      // if (buf[i]==">") Serial.print("!");
      v=buf[i+1];
      a=buf[i+2]|(buf[i+3]<<8);

      PW(CS,0);

      if (chipType) {
        BUSW(DBI,v);
        BUSW(ABI,a);
      } else {
        BUSW(DBI,v);
        PW(AD,0);
        BUSW(DBI,a&0xff);
        BUSW(ABI,(a&0xff00)>>8);
        PW(AD,1);
      }

      PW(CS,1);
      i+=4;
    }
    // Serial.println("#");
  }

}
