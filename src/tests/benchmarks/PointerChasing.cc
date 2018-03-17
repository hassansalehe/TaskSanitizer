#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <iostream>

typedef struct elem_t {
  void * data;
  elem_t * next;
} elem_t;

namespace pchase {

void process(elem_t * elem) {
  printf("I am a task %p\n " ,elem);
}

void process_list(elem_t *elem) {
  #pragma omp parallel num_threads(4)
  {
    #pragma omp single
    {
      while ( elem != NULL ) {
        #pragma omp task firstprivate(elem)
        {
          // elem is firstprivate by default
          process( elem );
        }
        elem = elem->next;
        printf("Parent %p\n " ,elem);
      }
    }
  }
}

elem_t * initialize(int n) {
  elem_t *head = NULL;

  for (int i = 0 ; i < n; i++) {
    elem_t *elem = new elem_t;
    elem->next = head;
    head = elem;
  }

  return head;
}

void clean(elem_t *elem) {
  while (elem) {
    elem_t * tmp = elem->next;
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
  printf("List size %d\n", 4);
  elem_t * elems_head = pchase::initialize(size);
  pchase::process_list(elems_head);
  pchase::clean(elems_head);
  return 0;
}
