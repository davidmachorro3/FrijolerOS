#include "threads/fixed_point.h"
#include <inttypes.h>

#define Q 14
#define F (1<<Q)

typedef int fixpoint;

static fixpoint crear_fixp(int);
static fixpoint multi_fixp(fixpoint, fixpoint);
static fixpoint divi_fixp(fixpoint, fixpoint);
static int round_down_fixp(fixpoint);
static int round_nearest_fixp(fixpoint);

static fixpoint crear_fixp(int n){
  return n * F;
}

static fixpoint multi_fixp(fixpoint x, fixpoint y){
  return ((int64_t)x) * y / F;
}

static fixpoint divi_fixp(fixpoint x, fixpoint y){
  return ((int64_t)x) * F / y;
}

static int round_down_fixp(fixpoint x){
  return x / F;
}

static int round_nearest_fixp(fixpoint x){
  if (x >= 0){
    return(x+(F/2))/F;
  }else{
    return(x-(F/2))/F;
  }
}

