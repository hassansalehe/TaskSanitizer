//////////////////////////////////////////////////////////////
//
// flush.cc
//
// Copyright (c) 2017, Hassan Salehe Matar
// All rights reserved.
//
// This file is part of FlowSanitizer. For details, see
// https://github.com/hassansalehe/FlowSanitizer. Please also
// see the LICENSE file for additional BSD notice
//
// Redistribution and use in source and binary forms, with
// or without modification, are permitted provided that
// the following conditions are met:
//
// * Redistributions of source code must retain the above
//   copyright notice, this list of conditions and the
//   following disclaimer.
//
// * Redistributions in binary form must reproduce the
//   above copyright notice, this list of conditions and
//   the following disclaimer in the documentation and/or
//   other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names
//   of its contributors may be used to endorse or promote
//   products derived from this software without specific
//   prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////

// From the OpenMP specification:
//
// * Makes a thread’s temporary view of memory consistent with
//   memory and enforces an order on the memory operations of
//   the variables explicitly specified or implied
//
// * The binding thread set for a flush region is the encountering
//   thread. Execution of a flush region affects the memory and
//   the temporary view of memory of only the thread that executes
//   the region. It does not affect the temporary view of other
//   threads. Other threads must themselves execute a flush operation
//   in order to be guaranteed to observe the effects of the
//   encountering thread’s flush operation
//
//  * A barrier also implies a flush
//
// References:
//  1. http://www.openmp.org/wp-content/uploads/openmp-examples-4.5.0.pdf
//  2. http://www.openmp.org/wp-content/uploads/openmp-4.5.pdf

#include <stdio.h>
#include <omp.h>

int main() {

  int count = 0;
  #pragma omp parallel shared(count)
  {
    #pragma omp flush(count)
    count++;
    #pragma omp flush(count)
  }

  printf("Value of count: %d, construct: <flush>\n", count);
  return 0;
}
