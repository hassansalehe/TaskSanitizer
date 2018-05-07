//////////////////////////////////////////////////////////////
//
// critical.cc
//
// Copyright (c) 2017, Hassan Salehe Matar
// All rights reserved.
//
// This file is part of TaskSanitizer. For details, see
// https://github.com/hassansalehe/TaskSanitizer. Please also
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
//   The critical construct restricts execution of the
//   associated structured block to a single thread at a time.
//
// References:
//  1. http://www.openmp.org/wp-content/uploads/openmp-examples-4.5.0.pdf
//  2. http://www.openmp.org/wp-content/uploads/openmp-4.5.pdf

#include <stdio.h>
#include <omp.h>

int main() {

  int count = 0;
  #pragma omp parallel num_threads(4) shared(count)
  {
    #pragma omp critical
    {
      count++;
    }
  }

  printf("Value of count: %d, construct: <critical>\n", count);
  return 0;
}
