/////////////////////////////////////////////////////////////////
//  TaskSanitizer: determinacy race detection tool
//          for OpenMP task applications
//
//    Copyright (c) 2017 - 2018 Hassan Salehe Matar
//      Copying or using this code by any means whatsoever
//      without consent of the owner is strictly prohibited.
//
//   Contact: hassansalehe-at-gmail-dot-com
//
/////////////////////////////////////////////////////////////////

#ifndef _INSTRUMENTOR_EVENTLOGGER_UTIL_H_
#define _INSTRUMENTOR_EVENTLOGGER_UTIL_H_

#include "instrumentor/eventlogger/Logger.h"
#include "instrumentor/eventlogger/TaskInfo.h"
#include "instrumentor/callbacks/InstrumentationCallbacks.h"
#include <ompt.h>
#include <cassert>

// Namespace which contains small helper/utility functions
// used in task action logging callbacks.
namespace UTIL {

/**
 * Creates and initializes action logging metadata */
void createNewTaskMetadata(ompt_data_t *task_data) {

  // Null if this task created before OMPT initialization
  if (task_data == nullptr) return;
  TaskInfo *newTaskInfo =  new TaskInfo;
  TaskInfo *oldTaskInfo = (TaskInfo *)task_data->ptr;

  newTaskInfo->threadID    = static_cast<uint>( pthread_self() );
  newTaskInfo->taskID      = INS::GenTaskID();
  newTaskInfo->active      = true;
  task_data->ptr           = (void *)newTaskInfo;

  if (oldTaskInfo) {
    newTaskInfo->childrenIDs = oldTaskInfo->childrenIDs;
  }

  INS::TaskBeginLog(*newTaskInfo);

  if (oldTaskInfo) {
    INTEGER value = reinterpret_cast<INTEGER>(oldTaskInfo);
    INS::TaskReceiveTokenLog(*newTaskInfo, oldTaskInfo, value);
    delete oldTaskInfo;
  }
  PRINT_DEBUG("Task_Began, (threadID: " +
      std::to_string(newTaskInfo->threadID) + ", taskID: " +
      std::to_string(newTaskInfo->taskID)   + ")"
  );
}

/**
 * Marks task metadata of completion of task.
 * It also logs relevant information to file. */
void markEndOfTask(ompt_data_t *task_data) {

  // Null if this task created before OMPT initialization
  if (task_data == nullptr) return;

  TaskInfo *taskInfo = (TaskInfo*)task_data->ptr;
  uint threadID      = (uint)pthread_self();
  // assert(taskInfo->threadID == threadID);
  taskInfo->active   = false;

  INS::TaskEndLog(*taskInfo);
  PRINT_DEBUG("Task_Ended: (threadID: " + std::to_string(threadID) +
      ") taskID: " + std::to_string(taskInfo->taskID));
}


/**
 * Marks this task as complete */
void endThisTask(ompt_data_t *task_data) {
  markEndOfTask(task_data);
}

/**
 * Changes identifer of the current task to
 * new ID and thus make it look like a new task. */
void disguiseToTewTask(ompt_data_t *task_data) {
  UTIL::createNewTaskMetadata(task_data);
}

} // namespace

#endif //RUNTIME_UTIL_H
