//////////////////////////////////////////////////////////////
//
// RacyPointerChasing.cc
//
// Copyright (c) 2015 - 2018 Hassan Salehe Matar
// All rights reserved.
//
//  This code has been influenced by an OpenMP forum:
//       http://forum.openmp.org/forum/viewtopic.php?f=3&t=1231
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
//
//  This program generates a linked list whose nodes contain generated
//  random numbers. The memory locations for holding random numbers in
//  the nodes are dynamically allocated. Two randomly selected nodes
//  write random numbers they generated to the same memory location.
//
// There is a determinacy race between two randomly selected tasks
// which write random numbers to a common dynamically allocated memory.
//
// References for OpenMP tasks:
//  1. http://www.openmp.org/wp-content/uploads/openmp-examples-4.5.0.pdf
//  2. http://www.openmp.org/wp-content/uploads/openmp-4.5.pdf

#include <omp.h>
#include <iostream>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <set>

#if !defined(NTHREADS)
#define NTHREADS 2
#endif

typedef struct elem_t {
  int * data;
  elem_t * next;
} elem_t;

namespace pchase {

void generateAndStoreRandomNumber(elem_t * elem) {
  *(elem->data) = std::rand();
}

void process_list(elem_t *elem) {
  #pragma omp parallel num_threads(NTHREADS)
  {
    #pragma omp single
    {
      while ( elem != NULL ) {
        #pragma omp task firstprivate(elem)
        {
          // elem is firstprivate by default
          generateAndStoreRandomNumber( elem );
        }
        elem = elem->next;
      }
    }
  }
}

elem_t * createNode(elem_t *next) {
  elem_t *elem = new elem_t;
  elem->data = new int;
  elem->next = next;
  return elem;
}

elem_t * initialize(int n) {
  elem_t *head = NULL;

  // create duplicate node
  int *duplicate = new int;
  int pos1 = 0, pos2 = 0;
  while (pos1 == pos2) {
    pos1 = std::rand() % n;
    pos2 = std::rand() % n;
  }

  for (int i = 0 ; i < n; i++) {
    head = createNode(head);
    if (i == pos1 || i == pos2) {
      delete head->data;
      head->data = duplicate;
    }
  }
  return head;
}

void clean(elem_t *elem) {
  std::set<int *> pointers;

  while (elem) {
    elem_t * tmp = elem->next;
    if (pointers.find(elem->data) == pointers.end()) { // avoid memory leak
      pointers.insert(elem->data);
      delete elem->data;
    }
    delete elem;
    elem = tmp;
  }
}

}; // end namespace pchase

int main(int argc, char **argv) {

  int size;
  if (argc < 2) {
    size = 14;
  }
  else {
    size = atoi( argv[1] );
  }
  std::srand(std::time(nullptr));
  printf("List size %d\n", size);
  elem_t * elems_head = pchase::initialize(size);
  pchase::process_list(elems_head);
  pchase::clean(elems_head);
  return 0;
}
