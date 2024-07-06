#include "../rthp.h"

RTHPImplInfo RTHPImpl::getInfo() {
  return RTHPImplInfo(NULL,NULL,0);
}

int RTHPImpl::listDevices() {
  return 0;
}

std::vector<RTHPDevice> RTHPImpl::getDeviceList() {
  return {};
}

int RTHPImpl::init(int dev, unsigned int _rate, unsigned int tout) {
  return RTHP_SUCCESS;
}

bool RTHPImpl::isRunning() {
  return false;
}

void RTHPImpl::setChip(int _chip) {

}

int RTHPImpl::sendRegWrite(uint16_t addr, uint16_t value, RTHPPacketTypes packetType) {
  return RTHP_SUCCESS;
}

int RTHPImpl::sendRaw(char* data, size_t len) {
  return RTHP_SUCCESS;
}

int RTHPImpl::deinit() {
  return RTHP_SUCCESS;
}

RTHPImpl::~RTHPImpl() {

}