/////////////////////////////////////////////////////////////////
//  TaskSanitizer: a lightweight determinacy race checking
//          tool for OpenMP task applications
//
//    Copyright (c) 2015 - 2018 Hassan Salehe Matar
//      Copying or using this code by any means whatsoever
//      without consent of the owner is strictly prohibited.
//
//   Contact: hassansalehe-at-gmail-dot-com
//
/////////////////////////////////////////////////////////////////

#ifndef _DETECTOR_COMMUTATIVITY_CRITICALSECTIONS_H_
#define _DETECTOR_COMMUTATIVITY_CRITICALSECTIONS_H_

#include "detector/commutativity/CriticalSectionBody.h"
#include <map>
#include <cassert>

namespace tasksan {

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

  CriticalSectionBody *find(int source_line_num) {
    if (0 == sections.size()) return end();

    for (auto &cr : sections) {
      if (cr.second.getStartLineNo() <= source_line_num
          && source_line_num <= cr.second.getEndLineNo()) {
        return &cr.second;
      }
    }
    auto start = sections.lower_bound(source_line_num);
    if (start == sections.end()) return end();
    if ( source_line_num >= start->second.getStartLineNo() &&
        source_line_num <= start->second.getEndLineNo() ) {
      return &start->second;
    }

    return nullptr;
  }

  CriticalSectionBody *end() { return nullptr; }

}; // class

} // end commute

} // end tasksan

#endif // end critical sections
