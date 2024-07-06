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
  running = false;
  rate = 0;
  timeout = 0;

  // port
}

int ERTHP::listDevices() {
  ports=serial::list_ports();
  devs.clear();
  for (unsigned long int i = 0; i < ports.size(); i++) {
    devs.push_back(RTHPDevice(i,ports[i].port.c_str()));
  }
  return devs.size();
}

bool ERTHP::isRunning() {
  return running;
}

void ERTHP::setChip(int _chip) {
  (void)_chip;
  chip = 0;
}

int ERTHP::init(int dev, unsigned int _rate, unsigned int tout) {
  if (running) return RTHP_SUCCESS;
  rate = _rate;
  timeout = tout;
  if (devs.size() < 1) return RTHP_ERROR;
  try {
    port.setPort(devs[currectDev].name);
    port.setBaudrate(rate);
    port.setTimeout(serial::Timeout::max(),timeout,0,timeout,0);
  } catch (std::exception& xc) {
    return RTHP_ERROR;
  }
  port.open();
  if (!port.isOpen()) return RTHP_PORT_CLOSED;
  running = true;
  
  return RTHP_SUCCESS;
}

int ERTHP::sendRegWrite(uint16_t addr, uint16_t value, RTHPPacketTypes packetType) {
  switch (packetType) {
    case RTHP_PACKET_SHORT:
      port.write(fmt::sprintf("%c%c%c%c",RTHPPACKETSHORT_KEY,value&0xff,addr&0xff,addr>>8));
      break;
    default: return RTHP_ERROR;
  }
  return RTHP_SUCCESS;
}

int ERTHP::sendRaw(char* data, size_t len) {
  port.write((uint8_t*)data,len);
  return RTHP_SUCCESS;
}

int ERTHP::deinit() {
  port.close();
  running = false;
  return RTHP_SUCCESS;
}

ERTHP::~ERTHP() {
  if (running) port.close();
  devs.clear();
  ports.clear();
}