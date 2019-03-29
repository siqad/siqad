/** @file:     sqcommands.h
 *  @author:   Samuel
 *  @created:  2019.03.26
 *  @license:  GNU LGPL v3
 *
 *  @desc:     A job result set storing SQCommands
 */

#ifndef _COMP_SQ_COMMANDS_H_
#define _COMP_SQ_COMMANDS_H_

#include <QtWidgets>
#include "job_result.h"

namespace comp{

  //! A job result is a base class that holds plugin job step results that can 
  //! be used by other classes. Inherit this class to create implementations 
  //! for specific result types.
  class SQCommands : public JobResult
  {
    Q_OBJECT

  public:

    //! Empty constructor.
    SQCommands() : JobResult(SQCommandsResult) {};

    //! Constructor.
    SQCommands(QXmlStreamReader *rs);

    //! Destructor.
    ~SQCommands() {};

    //! Read from XML stream.
    void readFromXMLStream(QXmlStreamReader *rs);

    //! Return the SQCommands stored in this set.
    QStringList sqCommands() {return sq_commands;}

  private:

    QStringList sq_commands;


  };

} // end of comp namespace

#endif

