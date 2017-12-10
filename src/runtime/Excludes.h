/////////////////////////////////////////////////////////////////
//  ADFinspec: a lightweight non-determinism checking
//          tool for ADF applications
//
//    Copyright (c) 2015 - 2017 Hassan Salehe Matar & MSRC at Koc University
//      Copying or using this code by any means whatsoever
//      without consent of the owner is strictly prohibited.
//
//   Contact: hmatar-at-ku-dot-edu-dot-tr
//
/////////////////////////////////////////////////////////////////

// Include for the instrumentation passes.

#ifndef EXCLUDES_H_P_P
#define EXCLUDES_H_P_P

#include "Libs.h" // the LLVM includes put there
#include "ADFSchedulerSignatures.h"

using namespace llvm;
using namespace std;

namespace INS {

   // function signature for terminating the scheduler
   StringRef schedTerminFunc;

   // function signature for initializing the scheduler
   StringRef schedInitiFunc;

   // function singature for passing tokens to the succeeding tasks
   StringRef tokenPassingFunc;

   // function signature for receiving tokens in a task
   StringRef tokenReceivingFunc;

   // function signature of a task body
   StringRef taskSignature;

   // This method parses the signature file and initializes
   // the signatures accordingly.
   void InitializeSignatures() {
     dfinspec::ADFSchedulerSignatures signatureFile;

     // check if file does not exist
     if(!signatureFile.good()) {
       printf( "                                           \n");
       printf( "===========================================\n");
       printf( " ERROR!                                    \n");
       printf( "Error opening signature file in Excludes.h \n");
       printf( "                                           \n");
       printf( "===========================================\n");
       exit ( EXIT_FAILURE );
     }

     if(signatureFile.is_open()) {

       for (std::string line; signatureFile.getline(line); ) {
         StringRef key(line);

         if(key.startswith("TASK:")){ // task body name

           std::string ts = std::get<1>(key.split(':')).str();
           char * holder = new char[ts.length()];
           ts.copy(holder, ts.length());
           taskSignature = StringRef(holder, ts.length());
         }
         else if(key.startswith("PASS:")) { // for passing token

           std::string ts = std::get<1>(key.split(':')).str();
           char * holder = new char[ts.length()];
           ts.copy(holder, ts.length());
           tokenPassingFunc = StringRef(holder, ts.length());
         }
         else if(key.startswith("RECV:")) { // for receiving token

           std::string ts = std::get<1>(key.split(':')).str();
           char * holder = new char[ts.length()];
           ts.copy(holder, ts.length());
           tokenReceivingFunc = StringRef(holder, ts.length());
         }
         else if(key.startswith("CREA:")) { // for creating task
           //adf_create_task
           continue;
         }
         else if(key.startswith("INIT:")) { // for scheduler initialization

           std::string ts = std::get<1>(key.split(':')).str();
           char * holder = new char[ts.length()];
           ts.copy(holder, ts.length());
           schedInitiFunc = StringRef(holder, ts.length());
         }
         else if(key.startswith("TERM")) { // for scheduler rermination

           std::string ts = std::get<1>(key.split(':')).str();
           char * holder = new char[ts.length()];
           ts.copy(holder, ts.length());
           schedTerminFunc = StringRef(holder, ts.length());
         }
         line.clear();
       } // end loop
       signatureFile.close(); // close file
     }
   }

   bool DontInstrument(StringRef name) {
     int status = -1;
     string name_(name.str());
     char* d =abi::__cxa_demangle(name_.c_str(), nullptr, nullptr, &status);
      if(! status) {
       StringRef ab(d);
       string dname(d);
       //errs() << ab << "\n";
       if(dname.find("genmat") != string::npos) {
         return true;
       }
     }
      return false;
     //return true;
   }

   StringRef demangleName(StringRef name)
   {
      int status = -1;
      string name_(name.str());
      char* d =abi::__cxa_demangle(name_.c_str(), nullptr, nullptr, &status);
      if(! status) {
        StringRef ab(d);
        return ab;
      }
      return name;
   }

   string Demangle(StringRef name)
   {
      int status = -1;
      char* d =abi::__cxa_demangle(name.str().c_str(), nullptr, nullptr, &status);
      if(! status) {
        string dname(d);
        return dname;
      }
      return name;
   }

   bool isTaskBodyFunction(StringRef name) {

     int status = -1;
     char* d =abi::__cxa_demangle(name.str().c_str(), nullptr, nullptr, &status);
     if(! status) {
       string dname(d);
       //if(dname.find("std::function<void (token_s*)>::function") != string::npos
       //            || dname.find("create_task") != string::npos || dname.find("luTask") != string::npos)
       if(dname.find(taskSignature) != string::npos)
         return true;
     }
     return false;
   }

   bool isLLVMCall(StringRef name) {
      if(name.find("llvm") != StringRef::npos)
        return true;
      return false;
   }

   bool isPassTokenFunc(StringRef name) {

     return name.find(tokenPassingFunc) !=StringRef::npos;
   }

   bool isTaskCreationFunc(StringRef name) {

     return name.find("adf_create_task") != StringRef::npos;
   }

   bool isRuntimeInitializer(StringRef name){
     return name.find(schedInitiFunc)!= StringRef::npos;
   }

  bool isRuntimeTerminator(StringRef name) {
     return name.find(schedTerminFunc) != StringRef::npos;
  }

  void ClearSignatures() {
    delete [] schedTerminFunc.data();
    delete [] schedInitiFunc.data();
    delete [] tokenPassingFunc.data();
    delete [] tokenReceivingFunc.data();
    delete [] taskSignature.data();
  }
}

#endif
