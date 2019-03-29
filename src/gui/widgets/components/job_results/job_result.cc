/** @file:     job_result.cc
 *  @author:   Samuel
 *  @created:  2019.03.18
 *  @license:  GNU LGPL v3
 *
 *  @desc:     A job result set storing results read from completed plugin
 *             invocations.
 */

#include "job_result.h"

using namespace comp;

JobResult::JobResult(ResultType result_type)
  : result_type(result_type)
{}

JobResult::~JobResult()
{}
