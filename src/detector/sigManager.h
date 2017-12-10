/////////////////////////////////////////////////////////////////
//  ADFinspec: a lightweight non-determinism checking
//          tool for ADF applications
//
//    (c) 2015, 2016, 2017 - Hassan Salehe Matar & MSRC at Koc University
//      Copying or using this code by any means whatsoever
//      without consent of the owner is strictly prohibited.
//
//   Contact: hmatar-at-ku-dot-edu-dot-tr
//
/////////////////////////////////////////////////////////////////

// Defines class for managing function names

#ifndef SigManager_HPP_
#define SigManager_HPP_

#include <unordered_map>
#include <cassert>

using namespace std;



class SigManager {

private:
  unordered_map<INTEGER, string> functions;

public:
  /**
   * Stores function "name" with id "id"
   */
  void addFuncName(string name, INTEGER id) {

    assert(functions.find(id) == functions.end());
    functions[id] = name;
  }

  /**
   * Returns the function signature given the function id
   */
  string getFuncName(INTEGER id) {

    auto fIdptr = functions.find( id );
    if( fIdptr == functions.end() ) {
      cout << "This function Id never exists: " << id << endl;
    }
    assert(fIdptr != functions.end());
    return fIdptr->second;
  }

  /**
   * Returns the function name identifier
   */
  INTEGER getFuncId(string name) {
    for(auto i = functions.begin(); i != functions.end(); i++)
       if(i->second == name)
         return i->first;

    return 0; // FIXME
  }

};

#endif // SigManager_HPP_
