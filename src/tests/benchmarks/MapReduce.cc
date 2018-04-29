///////////////////////////////////////////////////////////////////////
//
//                  (c) 2017, 2018 - Hassan Salehe Matar
//
//  Description:
//
//   * This program constructs word frequency histogram from a file
//
//   *  This program creates 3 tasks namely Map, Reduce,
//      and Printer.
//
//   *  The expected execution order of these tasks is
//                --> Map -->  Reduce ---
//               /                       \
//              /                         v
//      Initial  ---> Map -->  Reduce  ----> Printer.
//              \                         ^
//               ---> Map -->  Reduce ---'
//               \                       ^
//                --> Map -->  Reduce --'
//
//      This execution order is ensured by the data flow dependency
//      specified using the depend clause of OpenMP task pragma.
//
//   *  Archer does not report any race on this setting.
//
//////////////////////////////////////////////////////////////////////

#include <iostream>
#include <omp.h>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>

#include <string.h>
#include <assert.h>

#define NTHREADS 2
#define WORDS 2048
#define WORDSIZE 50

typedef struct vector {
  int capacity = 0;
  int size = 0;
  char words[WORDS][WORDSIZE];

  vector() {
    capacity = WORDS;
    size = 0;
    //words = (char**) malloc(capacity * sizeof(char *));
  }

  int push_back(const char * word) {
    if (size == capacity || strlen(word) >= WORDSIZE) return -1;

    int pos = size++;
    strcpy(words[pos], word);
    return pos;
  }

  // returns position of the pushed word
  int push_unique(const char * word) {

    if (size == capacity || strlen(word) >= WORDSIZE) return -1;
    if (word == NULL) return -1;

    for (int pos = 0; pos < size; ++pos) {
      if ( words[pos] && 0 == strcmp(words[pos], word) ) {
        return pos;
      }
    }

    int pos = size++;
    strcpy(words[pos], word);
    return pos;
  }

  bool isFull()           const { return size == capacity; }
  int getSize()           const { return size; }
  int getCapacity()       const { return capacity; }
  const char *at(int pos) const { return words[pos]; }

  void print() {
    for (int i = 0; i < size; i++) {
      printf("%s\n", words[i]);
    }
  }

} vector;


typedef struct map {
  vector words;
  int counts[WORDS];
  int capacity;

  map() {
    memset(counts, 0, sizeof(int) * words.getCapacity());
    capacity = words.getCapacity();
  }

  const char *atWord(int pos) const {
    assert(pos < words.getSize());
    return words.at(pos);
  }

  int atCount(int pos) const {
    assert(pos < words.getSize());
    return counts[pos];
  }

  int getSize() const { return words.getSize(); }

  void addCount(const char *word, int precount) {
    int pos = words.push_unique(word);
    if(pos != -1) {
      counts[pos] += precount;
    }
  }

  void print() {
    for (int i = 0; i < words.getSize(); ++i) {
      printf("%s: %d\n", words.at(i), counts[i]);
    }
  }

} map;
void prepareInputs(vector &words, char *file) {

  std::ifstream text( file );
  std::string line;

  while ( getline( text, line ) ) {
    std::istringstream splits( line );
    std::string word;
    while (splits >> word) {
      words.push_back( word.c_str() );
    }
  }

  //words.print();
}

void splitInput(vector &words, vector &sub_words, int pos) {
  int chunk_size = words.getSize() / NTHREADS;
  int start      = chunk_size * pos;
  int end        = start + chunk_size;

  while (start < end) {
    sub_words.push_back( words.at(start++) );
  }
}

void mapWords(vector &words, map &hist) {
  for (int pos = 0; pos < words.getSize(); ++pos) {
    hist.addCount( words.at(pos), 1 );
  }
  //hist.print();
}

void updateHistogram(map &histogram, const map &word_hist) {

  for (int pos = 0; pos != word_hist.getSize(); pos++) {
    const char *word   =  word_hist.atWord(pos);
    int     precount   =  word_hist.atCount(pos);
    histogram.addCount(word, precount);
  }
}

void printHistogram(const map &histogram) {
  for ( int pos = 0; pos < histogram.getSize(); ++pos) {
    std::cout << histogram.atWord(pos)  << ": "
              << histogram.atCount(pos) << std::endl;
  }
}

int main(int argc, char* argv[]) {
  map    histogram;
  map    sub_hist[NTHREADS];
  vector sub_words[NTHREADS];
  vector words;

  if (argc < 2 ) {
    std::cout << "ERROR!! No input file given" << std::endl;
    std::cout << "Exiting" << std::endl;
    exit(1);
  }
  prepareInputs(words, argv[1]);

  #pragma omp parallel num_threads(NTHREADS) shared(words)  \
  shared(sub_hist) shared(sub_words) shared(histogram)
  {
    #pragma omp single
    {
      // split words into 4 buckets
      for (int i = 0; i < NTHREADS; i++) {
        #pragma omp task depend(out: sub_words[i])  \
        firstprivate(i)
        {
           splitInput(words, sub_words[i], i);
           std::cout << "split thread: " << omp_get_thread_num() << "\n";
        }
      }

      for (int i = 0; i < NTHREADS; i++) {
        #pragma omp task depend(out:sub_hist[i]) depend(in:sub_words[i])  \
        firstprivate(i)
        { // Map task
          mapWords(sub_words[i], sub_hist[i]);
          std::cout << "map thread: " << omp_get_thread_num() << "\n";
        }
      }

      for (int i = 0; i < NTHREADS; i++) {
        #pragma omp task depend(in: sub_hist[i])
        { // Reduce task
          updateHistogram(histogram, sub_hist[i]);
          std::cout << "reduce thread: " << omp_get_thread_num() << "\n";
          //delete sub_hist;
        }
      }

      #pragma omp taskwait
      #pragma omp task //depend(in: histogram) shared(histogram)
      { // Printer task
        std::cout << "printer thread no: " << omp_get_thread_num()
                  << "\n Total words: "    << histogram.getSize()
                  << std::endl;
                  printHistogram(histogram);
      }
    }
  } // end parallel omp

  return 0;
}
