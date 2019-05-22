#ifndef SPINLIB_H
#define SPINLIB_H

typedef unsigned long long u64_t;
typedef unsigned long long cycles_t;
typedef unsigned long long microsec_t;

/*
 * this is probably the trickiest thing to configure and
 * the accuracy of the workgen depends very much on this.
 */
#define SPINLIB_ITERS_SPIN (51000)

extern unsigned int spinlib_cycs_per_us;

extern void spinlib_calib(unsigned int cycs_per_us);
extern void spinlib_usecs(cycles_t usecs);
extern void spinlib_cycles(cycles_t cycs);

#define rdtscll(val) __asm__ __volatile__("rdtsc" : "=A" (val))

#endif /* SPINLIB_H */
