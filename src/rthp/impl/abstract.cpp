#include "../rthp.h"

int RTHPImpl::listDevices() {
  return 0;
}

int RTHPImpl::init(int dev) {
  return RTHP_SUCCESS;
}

int RTHPImpl::sendRegWrite(uint16_t addr, uint16_t value) {
  return RTHP_SUCCESS;
}

int RTHPImpl::sendRaw(char* data) {
  return RTHP_SUCCESS;
}

int RTHPImpl::deinit() {
  return RTHP_SUCCESS;
}