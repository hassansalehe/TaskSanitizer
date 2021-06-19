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

// This header defines the Action class. Action represetions a single
// action of an OpenMP task program. Example actions are the write,
// read, token receive, token send, task begin, and task end

#ifndef _COMMON_EVENTLOGGER_ACTION_H__
#define _COMMON_EVENTLOGGER_ACTION_H__

// includes and definitions
#include "common/defs.h"

class Action {
 public:
  INTEGER taskId;        // task id of writter
  ADDRESS addr;          // destination address
  VALUE value;           // value written
  VALUE lineNo;          // source-line number
  INTEGER funcId;        // the identifier of corresponding function
  std::string funcName;  // source-function name
  bool isWrite;          // true if this action is "write"

  Action(INTEGER tskId, VALUE val, VALUE ln, INTEGER fuId):
    taskId(tskId), value(val), lineNo(ln), funcId(fuId) {}

  Action(INTEGER tskId, ADDRESS adr, VALUE val, VALUE ln, INTEGER fuId):
    taskId(tskId), addr(adr), value(val), lineNo(ln), funcId(fuId) { }

  Action(INTEGER tskId, address adr, lint val, int ln, INTEGER fuId ):
    taskId(tskId), addr(adr), value(val), lineNo(ln), funcId(fuId) { }

  Action() { }

  // Generates std::string representation of the action and stores
  // in "buff". It appends '\n' at the end of the std::string
  void printAction(std::ostringstream & buff) {
    printActionNN( buff );
    buff << std::endl;
  }

  // Generates std::string representation of the action and stores in "buff".
  // It does not append '\n' at the end of. the std::string
  void printActionNN(std::ostringstream & buff) {
    std::string type = " R ";
    if ( isWrite ) type = " W ";
    buff << taskId << type <<  addr << " " << value
         << " " << lineNo << " " << funcId;
  }

}; // end Action

#endif // end action.h
