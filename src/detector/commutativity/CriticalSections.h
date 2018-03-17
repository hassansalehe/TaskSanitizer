#ifndef DETECTOR_COMMUTATIVITY_CRITICALSECTIONS_H
#define DETECTOR_COMMUTATIVITY_CRITICALSECTIONS_H

#include <map>
#include <cassert>
#include "detector/commutativity/CriticalSectionBody.h"

namespace tasan {

namespace commute {

class CriticalSections {
private:
  std::map<int, CriticalSectionBody>   sections;

public:
  void insert(CriticalSectionBody cbody) {
    if (find( cbody.getStartLineNo() ) == this->end() &&
        find( cbody.getEndLineNo() ) == this->end()) {
      sections[ cbody.getStartLineNo() ] = cbody;
    }
  }

  void insert(std::vector<Instruction> body_structure) {
     insert( CriticalSectionBody(body_structure) );
  }

  size_t getSize() { return sections.size(); }

  CriticalSectionBody *find(int lineNo) {
    if (0 == sections.size()) return end();

    for (auto &cr : sections) {
      if (cr.second.getStartLineNo() <= lineNo
          && lineNo <= cr.second.getEndLineNo()) {
        return &cr.second;
      }
    }
    auto start = sections.lower_bound(lineNo);
    if (start == sections.end()) return end();
    if ( lineNo >= start->second.getStartLineNo() &&
        lineNo <= start->second.getEndLineNo() ) {
      return &start->second;
    }

    return nullptr;
  }

  CriticalSectionBody *end() { return nullptr; }

}; // class

} // end commute

} // end tasan

#endif // end critical sections
