// E-RTHP VERSION 0
// REAL-TIME HARDWARE PLAYBACK HOST IMPLEMENTATION
// COMPLIES WITH E-RTHP REVIVIOSN 0

#include <string>
#include "serial/serial.h"

#define MSG_CONFIRM "^"
#define MSG_CANCEL "X"
#define MSG_BEACON "*"

class ERTHP {
  serial::Serial serialPort;
  struct ERTHP_Serial {
    unsigned long int serialBaudrate, serialTimeout;
    std::string portName;
    unsigned long int beaconInterval;
    ERTHP_Serial():
      serialBaudrate(9600),
      serialTimeout(1000),
      portName(""),
      beaconInterval(1000) {}
  } erthp_serial;
  struct ERTHP_Log {
    std::string logBuffer[64]; // cyclical buffer, not intended as a main log buffer
    std::string lastLog;
    unsigned char lastLogNum;
    ERTHP_Log():
      lastLog(""),
      lastLogNum(0) {}
  } erthp_log;

  public:
    // serial
    int initSerial(std::string port, unsigned int baudrate, unsigned int timeout);
    int sendSerial(std::string msg);
    void sendBeacon();

    // logging
    void clearLog();
    void writeLog(std::string log);
    std::string getLastLog();
};

void ERTHP::clearLog() {
  for (unsigned char i=0; i<64; i++) {
    erthp_log.logBuffer[i]="";
  }
}

void ERTHP::writeLog(std::string logm) {
  erthp_log.lastLog=logm;
  erthp_log.lastLogNum++;
  erthp_log.lastLogNum%=64;
  erthp_log.logBuffer[erthp_log.lastLogNum]=erthp_log.lastLog;
}

std::string ERTHP::getLastLog() {
  return erthp_log.lastLog;
}

int ERTHP::initSerial(std::string port, unsigned int baudrate, unsigned int timeout) {
  erthp_serial.portName=port;
  erthp_serial.serialBaudrate=baudrate;
  erthp_serial.serialTimeout=timeout;
  serial::Serial serialPort(erthp_serial.portName, erthp_serial.serialBaudrate, serial::Timeout::simpleTimeout(erthp_serial.serialTimeout));
  if (!serialPort.isOpen()) {
    ERTHP::writeLog("ERTHP: could not open serial port!");
    return 1;
  }
  serialPort.write("RTHP");
  return 0;
}

int ERTHP::sendSerial(std::string msg) {
  serialPort.write(msg);
  return 0;
}