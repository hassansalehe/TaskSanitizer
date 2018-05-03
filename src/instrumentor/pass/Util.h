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

// Include for the instrumentation passes.

#ifndef _INSTRUMENTOR_PASS_UTIL_H_
#define _INSTRUMENTOR_PASS_UTIL_H_

#include "instrumentor/pass/LLVMLibs.h" // the LLVM includes put there

/// general namespace for TaskSanitizer tool
namespace tasan {

/// This namespace contains general utility/helper functions
/// used in various parts of the program.
namespace util {

// function signature for terminating the scheduler
llvm::StringRef schedTerminFunc;

// function signature for initializing the scheduler
llvm::StringRef schedInitiFunc;

// function singature for passing tokens to the succeeding tasks
llvm::StringRef tokenPassingFunc;

// function signature for receiving tokens in a task
llvm::StringRef tokenReceivingFunc;

// function signature of a task body
llvm::StringRef taskSignature = "task";

bool DontInstrument(llvm::StringRef name) {
  int status = -1;
  std::string name_(name.str());
  char* d =abi::__cxa_demangle(name_.c_str(), nullptr, nullptr, &status);
  if (! status) {
    llvm::StringRef ab(d);
    std::string dname(d);
    //errs() << ab << "\n";
    if (dname.find("genmat") != std::string::npos) {
      return true;
    }
  }
  return false;
}

llvm::StringRef demangleName(llvm::StringRef name) {
  int status = -1;
  std::string name_(name.str());
  char* d =abi::__cxa_demangle(name_.c_str(), nullptr, nullptr, &status);
  if (! status) {
    llvm::StringRef ab(d);
    return ab;
  }
  return name;
}

std::string Demangle(llvm::StringRef name) {
  int status = -1;
  char* d = abi::__cxa_demangle(name.str().c_str(),
      nullptr, nullptr, &status);
  if (! status) {
    std::string dname(d);
    return dname;
  }
  return name;
}

llvm::StringRef getPlainFuncName(llvm::Function & F) {
  llvm::StringRef name = tasan::util::demangleName(F.getName());
  auto idx = name.find('(');

  if (idx != llvm::StringRef::npos) {
    name = name.substr(0, idx);
  }
  return name;
}

/**
 * Checks if function is main function of the program
 * under instrumentation.
 */
bool isMainFunction(llvm::Function &F ) {
   return tasan::util::getPlainFuncName(F) == "main";
}

bool isTaskBodyFunction(llvm::StringRef name) {

  int status = -1;
  char* d = abi::__cxa_demangle(name.str().c_str(),
      nullptr, nullptr, &status);

  if (! status) {
    std::string dname(d);
    if (dname.find(taskSignature) != std::string::npos) {
      return true;
    }
  }

  if (status == -2) {
    return name.find(taskSignature) != llvm::StringRef::npos;
  }
  return false;
}

bool isLLVMCall(llvm::StringRef name) {
  if (name.find("llvm") != llvm::StringRef::npos) return true;
  else return false;
}

bool isPassTokenFunc(llvm::StringRef name) {
  return name.find(tokenPassingFunc) !=llvm::StringRef::npos;
}

bool isTaskCreationFunc(llvm::StringRef name) {
  return name.find("adf_create_task") != llvm::StringRef::npos;
}

bool isRuntimeInitializer(llvm::StringRef name) {
  return name.find(schedInitiFunc)!= llvm::StringRef::npos;
}

bool isRuntimeTerminator(llvm::StringRef name) {
  return name.find(schedTerminFunc) != llvm::StringRef::npos;
}
  } // end namespace util
} // end namespace tasan

#endif // end tasan_util.h
