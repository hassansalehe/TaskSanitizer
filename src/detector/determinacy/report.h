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

#ifndef _DETECTOR_DETERMINACY_REPORT_H_
#define _DETECTOR_DETERMINACY_REPORT_H_

// For storing names of conflicting tasks
// and the addresses they conflict at
class Report {
 public:
  int task1ID;
  int task2ID;
  std::set<Conflict> buggyAccesses;
}; // end Report

#endif // end report.h
