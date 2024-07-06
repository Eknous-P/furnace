#include "rthp.h"
#include "impl/erthp.h"
#include "impl/dummy.h"

RTHP::RTHP() {
  set=false;
  running=false;
  impl=0;
  i=NULL;
}

RTHP::~RTHP() {
  reset();
}

int RTHP::setup(int _impl) {
  if (set) return RTHP_ERROR;
  switch (_impl) {
    case RTHP_IMPL_DUMMY:
      i=new RTHPDummy;
      break;
    case RTHP_IMPL_ERTHP:
      i=new ERTHP;
      break;
    default: return RTHP_ERROR;
  }
  set=true;
  if (!i) return RTHP_ERROR;
  return RTHP_SUCCESS;
}

int RTHP::reset() {
  if (set) {
    i->deinit();
    delete i;
    i=NULL;
    set=false;
  }
  return RTHP_SUCCESS;
}

RTHPImplInfo RTHP::getImplInfo() {
  return i->getInfo();
}

bool RTHP::isSet() {
  return set;
}

bool RTHP::isRunning() {
  if (i) return i->isRunning();
  return false;
}

void RTHP::setPacketType(int type) {
  packetType = RTHPPacketTypes(type);
}

int RTHP::send(uint16_t addr, uint16_t value) {
  if (!i) return RTHP_ERROR;
  i->sendRegWrite(addr,value,packetType);
}
int RTHP::send(int chip, uint16_t addr, uint16_t value) {}
int RTHP::send(char* data, size_t len) {}