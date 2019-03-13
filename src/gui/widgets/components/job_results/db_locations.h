/** @file:     db_locations.h
 *  @author:   Samuel
 *  @created:  2019.03.17
 *  @license:  GNU LGPL v3
 *
 *  @desc:     Stores DB locations
 */

#ifndef _COMP_DB_LOCATIONS_H_
#define _COMP_DB_LOCATIONS_H_

#include <QtWidgets>

#include "job_result.h"

namespace comp{

  //! Stores electron configurations of DB layouts.
  class DBLocations : public JobResult
  {
    Q_OBJECT

  public:

    //! Constructor taking a QXmlStreamReader to read the results directly. The 
    //! results are internally sorted in ascending order of electron count.
    DBLocations(QXmlStreamReader *rs);

    // TODO alternative constructor taking relevant information
    
    //! Destructor.
    ~DBLocations() {};

    //! Return the DB locations.
    QList<QPointF> locations() const {return db_locs;}


  private:

    QList<QPointF> db_locs;

  };

} // end of comp namespace

#endif
