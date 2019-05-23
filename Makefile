
all: clean obj

obj: work_problem.c
	gcc -fopenmp -fno-stack-protector -I. work_problem.c -o work_problem

clean:
	rm -f work_problem
