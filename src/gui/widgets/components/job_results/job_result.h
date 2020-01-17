/** @file:     job_result.h
 *  @author:   Samuel
 *  @created:  2019.03.17
 *  @license:  GNU LGPL v3
 *
 *  @desc:     A job result set storing results read from completed plugin
 *             invocations.
 */

#ifndef _COMP_JOB_RESULT_H_
#define _COMP_JOB_RESULT_H_

#include <QtWidgets>

namespace comp{

  //! A job result is a base class that holds plugin job step results that can 
  //! be used by other classes. Inherit this class to create implementations 
  //! for specific result types.
  class JobResult : public QObject
  {
    Q_OBJECT

  public:

    //! The result type of this job result set.
    enum ResultType{UndefinedResult, DBLocationsResult, ChargeConfigsResult, 
      PotentialLandscapeResult, SQCommandsResult};
    
    //! Constructor.
    JobResult(ResultType result_type=UndefinedResult);
    
    //! Destructor.
    ~JobResult();

    //! Return the result type.
    ResultType resultType() {return result_type;}


  private:

    ResultType result_type;     // the type of this result


  };

} // end of comp namespace

#endif

