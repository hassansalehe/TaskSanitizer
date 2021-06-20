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

//
// This is a logger for all events in an OpenMP application

#ifndef _INSTRUMENTOR_EVENLOGGER_LOGGER_H_
#define _INSTRUMENTOR_EVENLOGGER_LOGGER_H_

#include "common/defs.h"
#include "instrumentor/eventlogger/TaskInfo.h"
#include "detector/determinacy/checker.h"
#include "detector/commutativity/CommutativityChecker.h"
#include <atomic>

struct hash_function {
  size_t operator()( const std::pair<ADDRESS,INTEGER> &p ) const {
    auto seed = std::hash<ADDRESS>{}(p.first);
    auto tid = std::hash<INTEGER>{}(p.second);
    //seed ^= tid + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    //return seed;
    // presumably addr and tid are 32 bit
    return (seed << 32) + tid;
  }
};

class INS {

  private:
    // a function to tell whether there is a happens-before relation between two tasks
    static inline bool hasHB( INTEGER tID, INTEGER parentID );

    // a strictly increasing value, used as tasks unique id generator
    static std::atomic<INTEGER> taskIDSeed;

    // storing function name pointers
    static std::unordered_map<STRING, INTEGER>funcNames;
    static INTEGER funcIDSeed;

    // mapping buffer location, value with task id
    static std::unordered_map<std::pair<ADDRESS,INTEGER>, INTEGER, hash_function> idMap;

    // keeping the happens-before relation between tasks
    static std::unordered_map<INTEGER, INTSET> HB;

    // keeping track of the last writer to a memory location
    static std::unordered_map<ADDRESS, INTEGER> lastWriter;

    // keeping track of the last reader from a memory location
    static std::unordered_map<ADDRESS, INTEGER> lastReader;

    // checker instance for detecting determinacy race online
    static Checker onlineChecker;

  public:
    // global lock to protect metadata, use this lock
    // when you call any function of this class
    static std::mutex guardLock;

    // checks if OPMT is initialized
    static bool isOMPTinitialized;

    // open file for logging.
    static inline VOID InitTaskSanitizerRuntime() {

      // reset attributes used
      idMap.clear(); HB.clear();
      lastReader.clear(); lastWriter.clear();

      taskIDSeed = 0;
      isOMPTinitialized = true;
    }

    // Generates a unique ID for each new task
    static inline INTEGER GenTaskID() {
      INTEGER taskID = taskIDSeed.fetch_add(1);
      return taskID;
    }

    static inline void initCommutativityChecker(char *fname) {
       onlineChecker.initializeCommutativityChecker(fname);
    }

    // registers the function if not registered yet.
    // Also prints the function to standard output.
    static inline INTEGER RegisterFunction(const STRING funcName) {

      guardLock.lock();
      int funcID = 0;
      auto fd = funcNames.find(funcName);
      if ( fd == funcNames.end() ) { // new function
        funcID = funcIDSeed++;
        funcNames[funcName] = funcID;
        onlineChecker.registerFuncSignature(
            std::string(funcName), funcID);
      } else {
         funcID = fd->second;
      }

      guardLock.unlock();
      return funcID;
    }

    // close file used in logging
    static inline VOID Finalize() {
      guardLock.lock();

      idMap.clear(); HB.clear();
      lastReader.clear();
      lastWriter.clear();
      //DuplicateManager::removeDuplicates( onlineChecker.getConflicts() );
      onlineChecker.reportConflicts();
      guardLock.unlock();
    }

    // called when a task begins execution and retrieves parent task id
    static inline VOID TaskBeginLog(TaskInfo& task) {
      guardLock.lock();
      onlineChecker.onTaskCreate(task.taskID);
      guardLock.unlock();
    }

    // called when a task begins execution. retrieves parent task id
    static inline VOID TaskReceiveTokenLog( TaskInfo & task,
        ADDRESS bufLocAddr, INTEGER value ) {
      INTEGER parentID = -1;
      auto tid = task.taskID;

      guardLock.lock();
      // if streaming location address not null and there is a sender of the token
      auto key = std::make_pair( bufLocAddr, value );
      if ( bufLocAddr && idMap.count( key ) ) {
        // dependent through a streaming buffer
        parentID = idMap[key];

        if (parentID != tid) {
          // there was a bug where a task could send token to itself
          onlineChecker.saveHappensBeforeEdge(parentID, tid);

          // there is a happens before between taskID and parentID:
          //parentID ---happens-before---> taskID
          if (HB.find( tid ) == HB.end()) {
            HB[tid] = INTSET();
          }
          HB[tid].insert( parentID );

          // take the happens-before of the parentID
          if (HB.find( parentID ) != HB.end()) {
            HB[tid].insert(HB[parentID].begin(), HB[parentID].end());
          }
        }
      }
      guardLock.unlock();
    }

    // called before the task terminates.
    static inline VOID TaskEndLog( TaskInfo& task ) {
      //guardLock.lock(); // protect file descriptor
      // do something
      //guardLock.unlock();
    }

    // stores the buffer address of the token and the
    // value stored in the buffer for the succeeding task.
    static inline VOID TaskSendTokenLog(TaskInfo & task,
        ADDRESS bufLocAddr, INTEGER value) {

      auto key = std::make_pair(bufLocAddr, value );
      guardLock.lock(); //  protect idMap
      idMap[key] = task.taskID;
      guardLock.unlock();
    }

    // provides the address of memory a task reads from
    static inline VOID Read( TaskInfo & task,
        ADDRESS addr, INTEGER source_line_num, STRING funcName ) {
      INTEGER funcID = task.getFunctionId( funcName );

      // register function if not registered yet
      if (funcID == 0) {
        funcID = RegisterFunction( funcName );
        task.registerFunction( funcName, funcID );
      }

      //task.saveReadAction(addr, source_line_num, funcID);
      std::stringstream ssin(std::to_string((VALUE)addr) + " 0 " +
          std::to_string(source_line_num) + " " + std::to_string(funcID));

      guardLock.lock();
      onlineChecker.detectRaceOnMem(task.taskID, "R", ssin);
      guardLock.unlock();
    }

    // stores a write action
    static inline VOID Write(TaskInfo & task, ADDRESS addr,
        INTEGER value, INTEGER source_line_num, STRING funcName) {

      INTEGER funcID = task.getFunctionId( funcName );

      // register function if not registered yet
      if (funcID == 0) {
        funcID = RegisterFunction( funcName );
        task.registerFunction( funcName, funcID );
      }

      //task.saveWriteAction(addr, value, source_line_num, funcID);
      std::stringstream ssin(std::to_string((VALUE)addr) + " " +
          std::to_string(value) + " " + std::to_string(source_line_num) +
          " " + std::to_string(funcID));

      guardLock.lock();
      onlineChecker.detectRaceOnMem(task.taskID, "W", ssin);
      guardLock.unlock();
    }

    // Saves IDs of child tasks at a barrier
    static inline VOID saveChildHBs(TaskInfo & task) {
      guardLock.lock();
      for (int uncleID : task.childrenIDs) {
        onlineChecker.saveHappensBeforeEdge(uncleID, task.taskID);
      }
      task.childrenIDs.clear();
      guardLock.unlock();
    }
};
#endif
