#ifndef DETECTOR_COMMUTATIVITY_CRITICALSECTIONBODY_H
#define DETECTOR_COMMUTATIVITY_CRITICALSECTIONBODY_H

#include <vector>
#include <string>
#include "instruction.h"

namespace tasan {

namespace commute {

class CriticalSectionBody {
private:
  int                        startLineNo;
  int                        endLineNo;
  std::vector<Instruction>   body;

public:
  CriticalSectionBody() {
    setStartLineNo( 0 );
    setEndLineNo(   0  );
  }

  CriticalSectionBody(std::vector<Instruction> _body) {
    if( _body.size() > 0 ) {
      setStartLineNo( _body.front().lineNo );
      setEndLineNo  ( _body.back().lineNo  );
      body = _body;
    } else {
      throw "Critical section must contain at least one statement";
    }
  }
  void   setStartLineNo(int lineNo) { startLineNo = lineNo; }
  void   setEndLineNo(int lineNo) {
    endLineNo = lineNo;
    assert(endLineNo >= startLineNo);
  }
  int    getStartLineNo()           { return startLineNo;   }
  int    getEndLineNo()             { return endLineNo;     }

  void setCriticalSectionBody(std::vector<Instruction> cbody) {
    body = cbody;
  }
  std::vector<Instruction> getCriticalSectionBody() {
    return body;
  }

  std::vector<Instruction>::iterator begin() { return body.begin(); }
  std::vector<Instruction>::iterator   end() { return body.end(); }

  bool operator<(const CriticalSectionBody &RHSbody) {
   return endLineNo <= RHSbody.startLineNo;
  }

  std::string to_string() {
    return std::to_string(startLineNo)
        + " <-- ("
        + std::to_string( body.size() )
        + " statements) -->"
        + std::to_string(endLineNo);
  }
};

} // end commute

} // end tasan

#endif
