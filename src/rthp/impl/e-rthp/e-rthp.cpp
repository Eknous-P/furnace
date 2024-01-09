// E-RTHP VERSION 0
// REAL-TIME HARDWARE PLAYBACK HOST IMPLEMENTATION
// COMPLIES WITH E-RTHP REVIVIOSN 0

#include "e-rthp.h"

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