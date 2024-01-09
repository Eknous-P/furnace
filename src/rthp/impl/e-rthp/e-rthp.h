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