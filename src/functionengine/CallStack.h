//////////////////////////////////////////////////////////////
//
// CallStack.h
//
// Copyright (c) 2015 - 2018 Hassan Salehe Matar
// All rights reserved.
//
// This file is part of TaskSanitizer. For details, see
// https://github.com/hassansalehe/TaskSanitizer. This code is
// distributed under BSD license. Please see the LICENSE file for
// additional BSD notice
//
//////////////////////////////////////////////////////////////
//
//  This implements a class which manages a stack of function calls
//  as performed by a task
//

#ifndef _FUNCTIONENGINE_CALLSTACK_H_
#define _FUNCTIONENGINE_CALLSTACK_H_

#include <stack>
#include <string>

class CallStack {
  private:
    std::stack funcStack;

  public:
    void printStack();
    void getTopUserCode();
    void pushFunction(std::string name);
    void popFunction(std::string name);
};

#endif
