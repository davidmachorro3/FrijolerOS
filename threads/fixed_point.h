#ifndef THREADS_FIXED_POINT_H
#define THREADS_FIXED_POINT_H

#define Q 14
#define F (1<<Q)

typedef int fixpoint;

static fixpoint crear_fixp(int);
static fixpoint multi_fixp(fixpoint, fixpoint);
static fixpoint divi_fixp(fixpoint, fixpoint);
static int round_down_fixp(fixpoint);
static int round_nearest_fixp(fixpoint);

#endif