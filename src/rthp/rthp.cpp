#include "rthp.h"
#include "impl/erthp.h"

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
  switch (_impl) {
    case RTHP_IMPL_DUMMY:
      break;
    case RTHP_IMPL_ERTHP:
      i=new ERTHP;
      break;
    default: return RTHP_ERROR;
  }
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