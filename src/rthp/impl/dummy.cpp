#include "dummy.h"

RTHPImplInfo RTHPDummy::getInfo() {
  return RTHPImplInfo("RTHP Dummy Implementation","does absolutely nothing",0);
}

int RTHPDummy::listDevices() {
  return 0;
}

std::vector<RTHPDevice> RTHPDummy::getDeviceList() {
  return devs;
}

int RTHPDummy::init(int dev, unsigned int _rate, unsigned int tout) {
  running=true;
  return RTHP_SUCCESS;
}

bool RTHPDummy::isRunning() {
  return running;
}

void RTHPDummy::setChip(int _chip) {
  chip=_chip;
}

int RTHPDummy::sendRegWrite(uint16_t addr, uint16_t value, RTHPPacketTypes packetType) {
  return RTHP_SUCCESS;
}

int RTHPDummy::sendRaw(char* data, size_t len) {
  return RTHP_SUCCESS;
}

int RTHPDummy::deinit() {
  running=false;
  return RTHP_SUCCESS;
}

RTHPDummy::RTHPDummy() {
  devs.clear();
  currectDev=0;
  chip=0;
  rate=0;
  timeout=0;
  running=false;
}

RTHPDummy::~RTHPDummy() {

}