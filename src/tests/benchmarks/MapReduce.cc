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

#define NTHREADS 4

void prepareInputs(std::vector<std::string> &words, char *file) {

  std::ifstream text( file );
  std::string line;

  while ( getline( text, line ) ) {
    std::istringstream splits( line );
    copy( std::istream_iterator< std::string >( splits ),
          std::istream_iterator< std::string >(),
          std::back_inserter( words )
        );
  }
}

void splitInput(std::vector<std::string> &words,
                std::vector<std::string> &sub_words,
                int pos) {
  int chunk_size = words.size() / NTHREADS;
  int start      = chunk_size * pos;
  int end        = start + chunk_size;

  while (start < end) {
    sub_words.push_back( words[ start++ ] );
  }
}

void mapWords(std::vector<std::string> &words,
              std::map<std::string, int> &hist) {
  for (auto word = words.begin(); word != words.end(); word++) {
    hist[ *word ] = 1;
  }
}

void updateHistogram(std::map<std::string, int> &histogram,
                     std::map<std::string, int> &word_hist) {

  for (auto elem = word_hist.begin(); elem != word_hist.end(); elem++) {
    std::string word  =  elem->first;
    int         count =  elem->second;
    histogram[ word ] +=  elem->count;
  }
}

void printHistogram(const std::map<std::string, int> & histogram) {
  for (auto word = histogram.begin();
       word != histogram.end();
       word++) {
    std::cout << word->first << ": " << word->second << std::endl;
  }
}

int main(int argc, char* argv[]) {
  std::map<std::string, int>  histogram;
  std::map<std::string, int>  sub_hist[NTHREADS];
  std::vector<std::string>    sub_words[NTHREADS];
  std::vector<std::string>    words;

  prepareInputs(words, argv[1]);

  #pragma omp parallel num_threads(NTHREADS)  \
  shared(sub_hist) shared(sub_words) shared(histogram)
  {
    #pragma omp single
    {
      // split words into 4 buckets
      for (int i = 0; i < NTHREADS; i++) {
        #pragma omp task depend(out: sub_words[i])  \
        firstprivate(i)  shared(sub_words)
        {
          splitInput(words, sub_words[i], i);
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
        #pragma omp task depend(in: sub_hist[i]) depend(out: histogram)   \
        shared(sub_hist)
        //shared(histogram)
        { // Reduce task

          updateHistogram(histogram, sub_hist[i]);
          std::cout << "reduce thread: " << omp_get_thread_num() << "\n";
          //delete sub_hist;
        }
      }

      #pragma omp task depend(in: histogram) shared(histogram)
      { // Printer task
        std::cout << "printer thread no: " << omp_get_thread_num()
                  << "\n Total words: "    << histogram.size()
                  << std::endl;
                  printHistogram(histogram);
      }

      #pragma omp taskwait
    }
  } // end parallel omp

  return 0;
}
