#include "erthp.h"

RTHPImplInfo ERTHP::getInfo() {
  return RTHPImplInfo(
    "E-RTHP",
    "send shite through serial port aaa",
    RTHPIMPLFLAGS_VARIABLESPEED|RTHPIMPLFLAGS_USESHORTPACKET|RTHPIMPLFLAGS_USERAWPACKET|RTHPIMPLFLAGS_PLUGNPLAY
  );
}

ERTHP::ERTHP() {
  ports.clear();
  devs.clear();
  currectDev = 0;

  // port
}

int ERTHP::listDevices() {
  return 0;
}

void ERTHP::setParams(unsigned int rate, unsigned int tout) {
  baudrate = rate;
  timeout = tout;
}

int ERTHP::init(int dev) {
  if (devs.size() < 1) return RTHP_ERROR;
  try {
    port.setPort(devs[currectDev].name);
    port.setBaudrate(baudrate);
    port.setTimeout(serial::Timeout::max(),timeout,0,timeout,0);
  } catch (std::exception& xc) {
    return RTHP_ERROR;
  }
  port.open();
  if (!port.isOpen()) return RTHP_PORT_CLOSED;
  return RTHP_SUCCESS;
}

int ERTHP::sendRegWrite(uint16_t addr, uint16_t value) {
  port.write("");
  return RTHP_SUCCESS;
}

int ERTHP::sendRaw(char* data, size_t len) {
  return RTHP_SUCCESS;
}

int ERTHP::deinit() {
  return RTHP_SUCCESS;
}

ERTHP::~ERTHP() {

}