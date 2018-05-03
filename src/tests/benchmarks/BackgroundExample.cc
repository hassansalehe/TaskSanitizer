///////////////////////////////////////////////////////////////////////
//
//                  (c) 2017, 2018 - Hassan Salehe Matar
//
//  Description:
//
//   This app creates two concurrent tasks with critical sections each.
//
//   Determinacy races:
//    - There is a determinacy race between the two concurrent
//      tasks because they can execute in any order and thus final
//      result of "i" can be 1 or 2
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>

#if !defined(NTHREADS)
#define NTHREADS 4
#endif

int main() {
 int i=0;
#pragma omp parallel num_threads(NTHREADS)
#pragma omp single
 {
   #pragma omp task shared(i)
   #pragma omp critical(lock_i)
   {  i = 1; }

   #pragma omp task shared(i)
   #pragma omp critical(lock_i)
   { i = 2; }
 }

 printf ("i=%d\n",i);
 return 0;
}
