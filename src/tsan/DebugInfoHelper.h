//===-- Extension to ThreadSanitizer.cpp - detecting races, Embeded ARM --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// (c) 2017 - 2018 Hassan Salehe Matar
//            Email: hmatar@ku.edu.tr
//
//===----------------------------------------------------------------------===//

#include "Libs.h"

#ifndef TSAN_DebugInfoHelper_h
#define TSAN_DebugInfoHelper_h

using namespace llvm;

namespace tasan {

// Implements helper functions for manipulating debugging
// information that is sent to the runtime nondeterminism
// reporting of TaskSanitizer.
namespace debug {

/**
 * Demangles names of attributes, functions, etc
 */
StringRef demangleName(StringRef name) {
  int status = -1;
  std::string name_(name.str());
  char* d =abi::__cxa_demangle(name_.c_str(), nullptr, nullptr, &status);
  if (! status) {
    StringRef ab(d);
    return ab;
  }
  return name;
}

/*
 * Returns file name with absolute path
 */
std::string createAbsoluteFileName(
  std::string & dir_name,
  std::string &file_name) {

  if (dir_name.size() <= 0 || file_name.size() <= 0) {
    return file_name;
  }

  // has full path already
  if (file_name[0] == '/') return file_name;

  if (file_name[0] == '.' && file_name[1] == '/') {
    if (dir_name[dir_name.length()-1] == '/') {
      return dir_name + file_name.substr(2);
    } else {
      return dir_name + file_name.substr(1);
    }
  }

  // file name has full path already
  if (file_name.find(dir_name) != std::string::npos)
    return file_name;

  if (dir_name[dir_name.length()-1] != '/')
    return dir_name + "/" + file_name;

  return dir_name + file_name;
}

/**
 * Returns absolute file name of which the function belongs to.
 */
std::string getFullFilename(llvm::Module & M) {

  std::string name      =  "Unknown";
  std::string dirName   =  "";
  llvm::DebugInfoFinder dFinder;
  dFinder.processModule(M);
  for (auto aScope : dFinder.scopes() ) {
    dirName = aScope->getDirectory().str();
    name    = aScope->getFilename().str();
    name    = createAbsoluteFileName(dirName, name);

    if (name.find("Unknown.iir") == std::string::npos) {
      return name;
    }
  }
  return name;
}

/**
 * Returns name of the function under instrumentation
 * as a value. The name is retrieved later at runtime.
 */
Value* getFuncName(Function & F) {
  StringRef name = demangleName(F.getName());
  auto idx = name.find('(');
  if (idx != StringRef::npos) {
    name = name.substr(0, idx);
  }

  IRBuilder<> IRB(F.getEntryBlock().getFirstNonPHI());
  return IRB.CreateGlobalStringPtr(name, "func_name");
}

/**
 * Return function name as a std::string
 */
llvm::StringRef getFuncNameStr(llvm::Function & F) {
  llvm::StringRef name = demangleName(F.getName());
  auto idx = name.find('(');
  if (idx != StringRef::npos) {
    name = name.substr(0, idx);
  }

  return name;
}

bool hasMainFunction(llvm::Module & M) {
  DebugInfoFinder dFinder;
  dFinder.processModule(M);
  auto subprograms = dFinder.subprograms();
  for (auto sp : dFinder.subprograms()) {
    if ( "main" == sp->getName() ) {
      return true;
    }
  }
  return false;
}

  /**
   * Returns the name of the memory location involved.
   * By object, this refers to the name of the variable.
   */
  Value * getObjectName(Value *V, llvm::Instruction* I, const DataLayout &DL) {
    Value* obj = GetUnderlyingObject(V, DL);

    Function* F = I->getFunction();
    IRBuilder<> IRB(F->getEntryBlock().getFirstNonPHI());

    if ( !obj ) {
      return IRB.CreateGlobalStringPtr("unknown", "variable");
    } else {
      return IRB.CreateGlobalStringPtr(obj->getName(), "variable");
    }
  }

  /**
   * Retrieves the line number of the instruction
   * being instrumented.
   */
  Value* getLineNumber(llvm::Instruction* I) {

    if (auto Loc = I->getDebugLoc()) { // Here I is an LLVM instruction
      IRBuilder<> IRB(I);
      unsigned Line = Loc->getLine();
      return ConstantInt::get(Type::getInt32Ty(I->getContext()), Line);
    }
    else {
      Constant* zero = ConstantInt::get(Type::getInt32Ty(I->getContext()), 0);
      return zero;
    }
  }

  /**
   * Retrieves the line number of the instruction
   * being instrumented.
   */
  unsigned int getLineNo(llvm::Instruction* I) {

    if (auto Loc = I->getDebugLoc()) { // Here I is an LLVM instruction
      return Loc->getLine();
    } else {
      return 0;
    }
  }

} // namespace debug

} // tasan

#endif
