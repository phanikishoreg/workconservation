#include <assert.h>
#include <sched.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <omp.h>
#include "spinlib.h"

#define GETTID() ((unsigned)syscall(__NR_gettid))
#define CYC_US 2200

#define ITERS 10

int main(void)
{
	unsigned long long max = 0;
	int i;

	spinlib_calib(CYC_US);

	for (i = 0; i < ITERS; i++) {
		volatile unsigned long long st = 0, en = 0;

		rdtscll(st);
#pragma omp parallel
		{
			//printf("(a, %u:%u, %d)\n", sched_getcpu(), GETTID(), omp_get_thread_num());
#pragma omp single
			{
				//printf("(b, %u:%u, %d)\n", sched_getcpu(), GETTID(), omp_get_thread_num());
#pragma omp task
				{
					//printf("(c, %u:%u, %d)\n", sched_getcpu(), GETTID(), omp_get_thread_num());
#pragma omp task
					{
						//printf("(d, %u:%u, %d)\n", sched_getcpu(), GETTID(), omp_get_thread_num());
					}
#pragma omp taskwait
				}

#pragma omp task
				{
					//printf("(e, %u:%u, %d)\n", sched_getcpu(), GETTID(), omp_get_thread_num());
				}
				spinlib_usecs(1000);
#pragma omp taskwait
			}
			spinlib_usecs(1000);
			//printf("(f, %u:%u, %d)\n", sched_getcpu(), GETTID(), omp_get_thread_num());
		}
		rdtscll(en);
		long diff = en - st;
		if (diff > 0) {
		       if (max < diff) max = diff;
			printf("%llu\n", (en - st) / CYC_US);
		}
	}

	printf("Max: %llu\n", max / CYC_US);
//	printf("Time: %llu, %llu\n", en - st, (en -st) / CYC_US);

	return 0;
}
