///////////////////////////////////////////////////////////////////////
//
//                  (c) 2017, 2018 - Hassan Salehe Matar
//
//  Description:
//
//   This app simulates banking operations. The first task sets 1000
//   to "balance" and then three other concurrent tasks do:
//    - One task sets 200 to "balance"
//    - One more task withdraws 500 from "balance"
//    - Another task adds 20% commission
//
//   Determinacy races:
//    - There are determinacy races among the three concurrent tasks.
//
//////////////////////////////////////////////////////////////////////

#include <iostream>
#include <cstring>
#include <omp.h>

#if !defined(NTHREADS)
#define NTHREADS 4
#endif

// balance
float balance;

// tokens
int token1;
int token2;
int token3;


int main() {

  #pragma omp parallel num_threads(NTHREADS)
  {
    #pragma omp single
    {
      #pragma omp task depend(out:token1,token2,token3)
      {
        balance  = 1000; // set balance
        token1 = token2 = token3 = 1; // token values
      }

      #pragma omp task depend(in:token1)
      {
        float rate = 0.2; // commission rate
        balance += (balance * rate); // add commission
      }

      #pragma omp task depend(in:token2)
      {
        int amount = 200;
        balance += amount;
      }

      #pragma omp task depend(in:token3)
      {
        int amount = 500; // amount to withdraw
        balance -= amount; // deduce amount withdrawn
      }

      // wait all tasks to complete
      #pragma omp taskwait
      std::cout << "balance: " <<  balance << std::endl;
    }
  }
  return 0;
}
