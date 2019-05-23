#include <assert.h>
#include <sched.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <omp.h>

#define GETTID() ((unsigned)syscall(__NR_gettid))
#define CYC_US 3200

#define rdtscll(val) __asm__ __volatile__("rdtsc" : "=A" (val))

#define ITERS 10

/*
 * From Chaos tests!
 * NOTE: number obtained by running composite instance with no interference..
 *       (validated with fiasco so far, it is 10us)
 */
#define ITERS_10US 5850
#define MULTIPLE 100

#define SPIN_ITERS (ITERS_10US*MULTIPLE)

static void __spin_fn(void) __attribute__((optimize("O0")));

static void
__spin_fn(void)
{
        unsigned int spin = 0;

        while (spin < SPIN_ITERS) {
                __asm__ __volatile__("nop": : :"memory");
                spin++;
        }
}


/* 
 * with 3 threads, 
 * if thd0 entered single, other parallel thds wait at the implicit barrier at the end of single.
 * by then, if thd0 pushed task1 on to the queue, thd1 or thd2 (lets say thd1) steals it.
 *
 * 2 control flows here:
 * if thd2 reaches the single barrier after thd0 created task2 and before thd1 created task11, 
 * perhaps thd2 will run that task.
 * with that, eventually thd1 will create task11 and run it itself on taskwait.
 * thd2 will also finish task2 and go back to the single barrier.
 * there is no work-conservation problem here!
 *
 * if thd2 reaches the single barrier after thd1 created task11 and before thd1 reached taskwait,
 * thd2 may steal the task11 and run that instead of running task2.
 * in this case, when thd1 reaches taskwait, it will not be able to run task2 as it is not a
 * descendant task of task1 and/or task11.
 * waiting eventually for thd2 to finish task11 and then resuming execution of the remaining 
 * task2 and going to the barrier. This is where it is non-work-conserving.
 *
 * That is the idea, to capture the non-work-conserving problem.
 *
 */

int main(void)
{
	unsigned long long max = 0;
	int i;
	unsigned long long x, y;

	rdtscll(x);
	__spin_fn();
	rdtscll(y);
	printf("%llu\n", (y - x)/CYC_US);

	for (i = 0; i < ITERS; i++) {
		volatile unsigned long long st = 0, en = 0;

		rdtscll(st);
		//fork parallel
		#pragma omp parallel
		{
			#pragma omp single
			{
				#pragma omp task
				{
					#pragma omp task
					{
						__spin_fn(); //do work, 1ms
					}
					#pragma omp taskwait
				}

				#pragma omp task
				{
					__spin_fn(); //do work, 1ms
				}
				__spin_fn(); //do work, 1ms
				#pragma omp taskwait
			}
		}
		//join parallel
		rdtscll(en);
		long diff = en - st;
		if (diff > 0) {
		       if (max < diff) max = diff;
			printf("%llu\n", (en - st) / CYC_US);
		}
	}

	printf("Max: %llu\n", max / CYC_US);

	return 0;
}
