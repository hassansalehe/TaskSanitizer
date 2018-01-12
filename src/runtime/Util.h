/////////////////////////////////////////////////////////////////
//  FlowSanitizer: output nondeterminism detection tool
//          for OpenMP applications
//
//    Copyright (c) 2017 - 2018 Hassan Salehe Matar
//      Copying or using this code by any means whatsoever
//      without consent of the owner is strictly prohibited.
//
//   Contact: hmatar-at-ku-dot-edu-dot-tr
//
/////////////////////////////////////////////////////////////////

#ifndef RUNTIME_UTIL_H
#define RUNTIME_UTIL_H

#include <cassert>
#include <ompt.h>
#include "TaskInfo.h"
#include "Callbacks.h"

// Namespace which contains small helper/utility functions
// used in task action logging callbacks.
namespace UTIL {

/**
 * Creates and initializes action logging metadata
 */
void createNewTaskMetadata(ompt_data_t *task_data) {

  // Null if this task created before OMPT initialization
  if(task_data == nullptr) return;

  TaskInfo * taskInfo = (TaskInfo *)task_data->ptr;
  if(taskInfo == nullptr)
    taskInfo = new TaskInfo;

  taskInfo->threadID = static_cast<uint>( pthread_self() );
  taskInfo->taskID = INS::GenTaskID();
  taskInfo->active = true;

  task_data->ptr = (void *)taskInfo;

  INS::TaskBeginLog(*taskInfo);
#ifdef DEBUG
  PRINT_DEBUG("Task_Began, (threadID: " +
      to_string(taskInfo->threadID) + ", taskID: " +
      to_string(taskInfo->taskID)   + ")";
#endif
}

/**
 * Marks task metadata of completion of task.
 * It also logs relevant information to file.
 */
void markEndOfTask(ompt_data_t *task_data) {

  // Null if this task created before OMPT initialization
  if(task_data == nullptr) return;

  TaskInfo *taskInfo = (TaskInfo*)task_data->ptr;
  uint threadID = (uint)pthread_self();
  assert(taskInfo->threadID == threadID);
  taskInfo->active = false;

  INS::TaskEndLog(*taskInfo);
#ifdef DEBUG
  PRINT_DEBUG("Task_Ended: (threadID: " + to_string(threadID) +
      ") taskID: " + to_string(taskInfo->taskID));
#endif
}


/**
 * Marks this task as complete
 */
void endThisTask(ompt_data_t *task_data) {
  markEndOfTask(task_data);
}

/**
 * Changes identifer of the current task to new ID and
 * thus make it look like a new task.
 */
void disguiseToTewTask(ompt_data_t *task_data) {
  UTIL::createNewTaskMetadata(task_data);
}

} // namespace

#endif //RUNTIME_UTIL_H
