#include <iostream>
#include <cstring>

#include <omp.h>

using namespace std;

// balance
float balance;

// tokens
int token1;
int token2;
int token3;


int main() {

  #pragma omp parallel num_threads(4)
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
      //cout << "balance: " << balance << std::endl;
      std::cout<< balance << std::endl;
    }
  }
  return 0;
}
