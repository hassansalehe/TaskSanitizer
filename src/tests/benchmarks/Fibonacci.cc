/////////////////////////////////////
 /* File  : fibonacci.c
 * Computes fibonacci numbers
 * Last modified  : 2011-04-25
 */

/////////////////////////////////////

#include <stdio.h>
#include <omp.h>

long comp_fib_numbers(int n) {
  if ( n == 0 || n == 1 ) return(n);

  // In case the sequence gets too short, execute the serial version
  if ( n < 20 ) {
    return(comp_fib_numbers(n-1)+comp_fib_numbers(n-2));
  } else {
    // Basic algorithm: f(n) = f(n-1) + f(n-2)
    long fnm1, fnm2, fn;

#pragma omp task shared(fnm1)
      fnm1 = comp_fib_numbers(n-1);

#pragma omp task shared(fnm2)
      fnm2 = comp_fib_numbers(n-2);

#pragma omp taskwait
      fn = fnm1 + fnm2;

    return (fn);
 }
}

int main(int argc, char **argv) {
  int n;
  long result;
  if (argc<2) {
    printf("usage ./fibonacci <number>\n");
    exit(1);
  }
  n = atoi(argv[1]);

//#pragma omp parallel num_threads(NUM_THREADS)
#pragma omp parallel
  {
#pragma omp single nowait
    {
      result = comp_fib_numbers(n);
    } // end of single region
  } // end of parallel region

  printf("finonacci(%d) = %ld\n", n, result);
  return 0;
}
