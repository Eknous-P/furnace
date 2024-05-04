// primitive ERTHP client V0.1
// for Arduino Nano (and compatible) boards
// TODO: test

// PINOUT(S?)

/* YM2413
             +----o----+
         GND-|GND    D1|-D3
          D4-|D2     D0|-D2
          D5-|D3    VCC|-5V   R0,R1
          D6-|D4     RO|------[10K]-+
          D7-|D5     MO|------[10K]-+--> audio out
          D8-|D6    /IC|-D10
          D9-|D7    /CS|-D12
     XTAL ||-|XIN   /WE|-GND
3.579545M ||-|XOUT   AO|-D13
             +---------+
*/

// defines

#define RS 10 // chip reset
#define CS 12 // chip select
#define AD 13 // address write
#define CT 11 // chip / bus type sense
              // HIGH/FLOATING - separate address/data buses (only allows an 8 bit long addr bus!)
              // LOW - unified address/data bus

#define DBI 2 // data bus index (1st bit)
#define ABI 14 // addr bus index

#define KEYCHR '>'

// function macros

#define PW(p,v) digitalWrite(p,v)
#define BUSW(i,d) for (int z=0; z<8; z++) PW(z+i, (d&(1<<z)) )

// variables

int avail=0;
bool busType=false;

// 0 - key
// 1 - data
// 2 - address low
// 3 - address high
unsigned char whichByte=0;

void setup() {
  pinMode(RS, OUTPUT); // reset the chip until inited
  PW(RS,0);

  Serial.begin(57600);   // init serial
  Serial.setTimeout(10); // 100Hz max?

  pinMode(CT, INPUT_PULLUP); // chip type sense
  busType=digitalRead(CT);  // sense
                                            // chip type: separate | unified
  for (int i=ABI; i<ABI+8; i++) pinMode (i, OUTPUT); // addr range | bus range high byte
  for (int i=DBI; i<DBI+8; i++) pinMode(i, OUTPUT);  // data range | bus range low byte

  pinMode(AD, OUTPUT); // address write pin
  pinMode(CS, OUTPUT); // chip select pin
}

char buffer[256];

void loop() {
  if (Serial.available()>0) {
    PW(RS,1); // unreset
    avail=Serial.available();
    Serial.readBytes(buffer,avail);
    for (int i=0; i<avail; i++) {
      if (buffer[i]==KEYCHR && whichByte==0) { // pls work correctly
        whichByte=1;
        continue;
      }
      PW(CS,0);
      switch (whichByte) {
        case 1:
          BUSW(DBI,buffer[i]);
          break;
        case 2:
          if (busType) { // separate
            BUSW(ABI,buffer[i]);
          } else {       // unified
            PW(AD,0);
            BUSW(DBI,buffer[i]);
          }
          break;
        case 3:
          if (busType) { // separate
            continue;    // separate 8 bits only!
          } else {       // unified
            PW(AD,0);
            BUSW(ABI,buffer[i]);
          }
          break;
      }
      PW(CS,1);
      whichByte++; whichByte&=0b11;
      PW(AD,1);
      Serial.write(buffer[i]);
    }
    free(buffer);
  }
}
