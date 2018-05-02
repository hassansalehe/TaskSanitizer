/////////////////////////////////////
 /* File  : fibonacci.c
 * Computes fibonacci numbers
 * Last modified  : 2011-04-25
 */

/////////////////////////////////////

#include <stdio.h>
#include <omp.h>

long * memoizer;
long comp_fib_numbers(int n) {
  if ( n == 0 || n == 1 ) return n;

  #pragma omp task firstprivate(n)
  {
    if (memoizer[n - 1] == 0) memoizer[n - 1] = comp_fib_numbers(n - 1);
  }

  #pragma omp task firstprivate(n)
  {
    if (memoizer[n - 2] == 0) memoizer[n - 2] = comp_fib_numbers(n - 2);
  }

  #pragma omp taskwait
  long fn = memoizer[n - 1] + memoizer[n - 2];

  return fn;
}

int main(int argc, char **argv) {
  int n;
  long result;
  if (argc < 2) {
    printf("usage ./fibonacci <number>\n");
    exit(1);
  }
  n = atoi(argv[1]);

  memoizer = new long[n+1]();
//#pragma omp parallel num_threads(NUM_THREADS)
#pragma omp parallel num_threads(2)
  {
#pragma omp single
    {
      result = comp_fib_numbers(n);
    } // end of single region
  } // end of parallel region

  printf("finonacci(%d) = %ld\n", n, result);
  return 0;
}
