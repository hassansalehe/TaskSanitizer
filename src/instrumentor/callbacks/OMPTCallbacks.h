/////////////////////////////////////////////////////////////////
//  TaskSanitizer: a lightweight determinacy race checking
//          tool for OpenMP task applications
//
//    Copyright (c) 2015 - 2021 Hassan Salehe Matar
//      Copying or using this code by any means whatsoever
//      without consent of the owner is strictly prohibited.
//
//   Contact: hassansalehe-at-gmail-dot-com
//
/////////////////////////////////////////////////////////////////
//

#ifndef _INSTRUMENTOR_CALLBACKS_OMPTCALLBACKS_H_
#define _INSTRUMENTOR_CALLBACKS_OMPTCALLBACKS_H_

#include "instrumentor/eventlogger/Util.h"
#include "instrumentor/eventlogger/Logger.h"
#include "instrumentor/eventlogger/TaskInfo.h"
#include "instrumentor/callbacks/InstrumentationCallbacks.h"
#include <omp.h>
#include <ompt.h>

//////////////////////////////////////////////////
//// Helper functions
//////////////////////////////////////////////////
void TaskSanitizer_TaskBeginFunc(ompt_data_t *task_data) {
  UTIL::createNewTaskMetadata(task_data);
}

void INS_TaskFinishFunc( ompt_data_t *task_data ) {
  UTIL::markEndOfTask(task_data);
}

//////////////////////////////////////////////////
//// OMPT Callback functions
//////////////////////////////////////////////////

static ompt_get_thread_data_t ompt_get_thread_data;
static ompt_get_unique_id_t ompt_get_unique_id;
static ompt_get_task_info_t ompt_get_task_info;

#define register_callback_t(name, type)                       \
do{                                                           \
  type f_##name = &on_##name;                                 \
  if (ompt_set_callback(name, (ompt_callback_t)f_##name) ==   \
      ompt_set_never) {                                       \
    printf("0: Could not register callback '" #name "'\n"); } \
}while(0)

#define register_callback(name) register_callback_t(name, name##_t)

static void
on_ompt_callback_implicit_task(
    ompt_scope_endpoint_t endpoint,
    ompt_data_t *parallel_data,
    ompt_data_t *task_data,
    unsigned int team_size,
    unsigned int thread_num) {
  switch( endpoint )
  {
    case ompt_scope_begin:
      if (task_data->ptr == NULL) {
        TaskSanitizer_TaskBeginFunc(task_data);
      }
      break;
    case ompt_scope_end:
      // this is called when the task has ended.
      INS_TaskFinishFunc(task_data);
      break;
  }
}

static void
on_ompt_callback_task_create( // called in the context of the creator
    ompt_data_t *parent_task_data,    // id of parent task
    const ompt_frame_t *parent_frame, // frame data for parent task
    ompt_data_t* new_task_data,       // id of created task
    int type,
    int has_dependences,
    const void *codeptr_ra) {         // pointer to outlined function
  int tid = ompt_get_thread_data()->value;
  switch ((int)type)
  {
    case ompt_task_initial:
      PRINT_DEBUG("TaskSanitizer: initial task created");
      break;
    case ompt_task_implicit:
    {
      PRINT_DEBUG("TaskSanitizer: implicit task created");
      break;
    }
    case ompt_task_explicit:
    {
      PRINT_DEBUG("TaskSanitizer: explicit task created");
      if (new_task_data->ptr == NULL) {
        TaskSanitizer_TaskBeginFunc(new_task_data);
      }
      void * depAddr = parent_task_data->ptr;
      if (depAddr) { // valid parent task
        // send and receive token
        INTEGER value = reinterpret_cast<INTEGER>(depAddr);
        INS::TaskSendTokenLog(
            *((TaskInfo *)parent_task_data->ptr), depAddr,  value);
        INS::TaskReceiveTokenLog(
            *((TaskInfo *)new_task_data->ptr), depAddr, value);

        // store child ID
        int childID = ((TaskInfo*)new_task_data->ptr)->taskID;
        ((TaskInfo*)parent_task_data->ptr)->addChild(childID);
        PRINT_DEBUG("Parent ID: "
            + std::to_string(((TaskInfo*)parent_task_data->ptr)->taskID)
            + ", Child ID: " + std::to_string(childID)
        );
      }
      break;
    }
    case ompt_task_target:
      PRINT_DEBUG("TaskSanitizer: target task created");
      break;
    case 5: //ompt_task_included:
      PRINT_DEBUG("TaskSanitizer: included task created");
      break;
    case ompt_task_untied:
    case 6:
      PRINT_DEBUG("TaskSanitizer: untied task created");
      break;
    default:
      ;
  }

  UTIL::endThisTask(parent_task_data);
  UTIL::disguiseToTewTask(parent_task_data);
}

static void
on_ompt_callback_task_schedule( // called in the context of the task
    ompt_data_t *prior_task_data,         // data of prior task
    ompt_task_status_t prior_task_status, // status of prior task
    ompt_data_t *next_task_data) {        // data of next task

  if (next_task_data->ptr == NULL) {
    TaskSanitizer_TaskBeginFunc(next_task_data);
  }
  PRINT_DEBUG("Task is being scheduled (p:" +
      std::to_string(next_task_data->value) + " t:" +
      std::to_string(prior_task_data->value) +  ")" );

  // int tid = ompt_get_thread_data()->value;
  // if (prior_task_status == ompt_task_complete)
}

// Callbacks for tokens
static void
on_ompt_callback_task_dependences(
    ompt_data_t *task_data,
    const ompt_task_dependence_t *deps,
    int ndeps) {

  TaskInfo * taskInfo = (TaskInfo*)task_data->ptr;
  std::cout << ("on_ompt_callback_task_dependences ")
            << std::to_string(task_data->value) << std::endl;

  for (int i = 0; i < ndeps; i++) {

    void * depAddr = deps[i].variable_addr;
    INTEGER value = reinterpret_cast<INTEGER>(depAddr);

    switch (deps[i].dependence_flags)
    {
      case ompt_task_dependence_type_out:
        INS::TaskSendTokenLog(*taskInfo, depAddr,  value);
        break;
      case ompt_task_dependence_type_in:
        INS::TaskReceiveTokenLog(*taskInfo, depAddr, value);
        break;
      case ompt_task_dependence_type_inout:
        INS::TaskReceiveTokenLog(*taskInfo, depAddr, value);
        INS::TaskSendTokenLog(*taskInfo, depAddr,  value);
        break;
      default:
        ;
    }
  }
}

// FIXME Hassan
static void
on_ompt_callback_task_dependence(
    ompt_data_t *first_task_data,
    ompt_data_t *second_task_data) {
  std::cout << ("One task dependence is going for registration " +
      std::to_string(first_task_data->value) + " --> " +
      std::to_string(second_task_data->value)) << std::endl;
}

// Executed when a task inters or leaves a barrier
// or a task region or a task group.
static void on_ompt_callback_sync_region(
    ompt_sync_region_kind_t kind,   // kind of sync region
    ompt_scope_endpoint_t endpoint, // endpoint of sync region
    ompt_data_t *parallel_data,     // data of parallel region
    ompt_data_t *task_data,         // data of task
    const void *codeptr_ra) {       // return address of runtime call
  switch (kind)
  {
    case ompt_sync_region_barrier:
    {
      break;
    }
    case ompt_sync_region_taskwait:
    {
      TaskInfo * taskInfo = (TaskInfo*)task_data->ptr;
      switch (endpoint) {
        case ompt_scope_begin:
        {
          PRINT_DEBUG("Taskwait begin scope, task id: "
              + std::to_string(taskInfo->taskID)
          //  + " parallel data: "
          //  + (parallel_data)
          );
          break;
        }
        case ompt_scope_end:
        {
          PRINT_DEBUG("Taskwait end scope, task id: "
              + std::to_string(taskInfo->taskID) );

          taskInfo->addChild(taskInfo->taskID);
          UTIL::endThisTask(task_data);
          UTIL::disguiseToTewTask(task_data);
          taskInfo = (TaskInfo*)task_data->ptr;
          INS::saveChildHBs(*taskInfo);
          PRINT_DEBUG("Taskwait (after) end scope, task id: "
              + std::to_string(taskInfo->taskID) );
          break;
        }
      }
      break;
    }
    case ompt_sync_region_taskgroup:
    {
      break;
    }
  }
}

/// Initialization and Termination callbacks

static int dfinspec_initialize(
    ompt_function_lookup_t lookup,
    ompt_data_t *tool_data) {

  // Register OMPT callbacks
  ompt_set_callback_t ompt_set_callback =
      (ompt_set_callback_t) lookup("ompt_set_callback");
  ompt_get_thread_data =
      (ompt_get_thread_data_t) lookup("ompt_get_thread_data");
  ompt_get_unique_id =
      (ompt_get_unique_id_t) lookup("ompt_get_unique_id");
  ompt_get_task_info =
      (ompt_get_task_info_t) lookup("ompt_get_task_info");

  register_callback(ompt_callback_implicit_task);
  register_callback(ompt_callback_task_create);
  register_callback(ompt_callback_task_schedule);
  register_callback(ompt_callback_task_dependences);
  register_callback(ompt_callback_task_dependence);
  register_callback(ompt_callback_sync_region);

  INS::InitTaskSanitizerRuntime();
  PRINT_DEBUG("TaskSanitizer: init");

  return 1;
}

static void dfinspec_finalize(ompt_data_t *tool_data) {
  PRINT_DEBUG("TaskSanitizer: Everything is finalizing");
}

ompt_start_tool_result_t* ompt_start_tool(
    unsigned int omp_version,
    const char *runtime_version) {

  static ompt_start_tool_result_t ompt_start_end = {
      &dfinspec_initialize,
      &dfinspec_finalize, 0};
  return &ompt_start_end;
}

#endif // RUNTIME_OMPTCALLBACKS_H
