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
//      Map -> Reduce -> Printer.
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

void prepareInputs(std::vector<std::string> &words, char *file) {

  std::ifstream text( file );
  std::string line;

  while ( getline( text, line ) ) {
    std::istringstream splits( line );
    copy( std::istream_iterator< std::string >( splits ),
          std::istream_iterator< std::string >(),
          back_inserter(words)
        );
  }
}

void updateHistogram(std::map<std::string, int> &histogram,
                     std::string &word, int count ) {
  if ( histogram.find( word ) != histogram.end() ) {
    histogram[word] = histogram[word] + count;
  } else {
    histogram[word] = count;
  }
}

int main(int argc, char* argv[] ) {
  std::map<std::string, int> histogram;
  std::map<std::string, int> sub_hist;

  // Prepare inputs.
  std::vector<std::string> words;
  prepareInputs(words, argv[1] );

  #pragma omp parallel num_threads(4)
  {
    #pragma omp single
    {
      // split words into 4 buckets
      #pragma omp task depend(out: sub_hist) shared(sub_hist) shared(histogram)
      { // Map task
        for (auto word = words.begin(); word != words.end(); word++) {
          updateHistogram( sub_hist, *word, 1 );
        }
        std::cout << "map thread no: "
                  << omp_get_thread_num() << std::endl;
      }

      #pragma omp task depend(in: sub_hist) depend(out: histogram) shared(sub_hist)
      //shared(histogram)
      { // Reduce task
        for (auto elem = sub_hist.begin();
             elem != sub_hist.end();
             elem++) {

          std::string word = elem->first;
          int count = elem->second;
          updateHistogram(histogram, word, count );
        }
        std::cout << "reduce thread no: "
                  << omp_get_thread_num() << std::endl;
        //delete sub_hist;
      }

      #pragma omp task depend(in: histogram) shared(histogram)
      { // Printer task
        std::cout << "printer thread no: "
                  << omp_get_thread_num() << std::endl;

        for (auto word = histogram.begin();
             word != histogram.end();
             word++) {
          std::cout << word->first << ": " << word->second << std::endl;
        }
      }
      #pragma omp taskwait
    }
  } // end parallel omp

  return 0;
}
