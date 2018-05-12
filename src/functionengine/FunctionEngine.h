//////////////////////////////////////////////////////////////
//
// FunctionEngine.h
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
//  This implements a class for managing function call stacks
//  of active tasks in the OpenMP tasking application.
//

#ifndef _FUNCTIONENGINE_FUNCTIONENGINE_H_
#define _FUNCTIONENGINE_FUNCTIONENGINE_H_

#include "function_engine/CallStack.h"

class FunctionEngine {
  public:
    void pushFunction(std::string name, unsigned taskID, int lineNo);
    void popFunction(std::string name, unsigned taskID);
    CallStack &getStack(unsigned taskID);
    void removeStack(unsigned taskID);
};

#endif
