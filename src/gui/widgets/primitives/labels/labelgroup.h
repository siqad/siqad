/** @file:     labelgroup.h
 *  @author:   Samuel
 *  @created:  2018.06.12
 *  @editted:  2018.06.12  - Samuel
 *  @license:  GNU LGPL v3
 *
 *  @brief:     Label widget for showing user-defined static or dynamic labels.
 */

#ifndef _GUI_PR_LABEL_H_
#define _GUI_PR_LABEL_H_

#include "../item.h"
#include "textlabel.h"

namespace prim{

  class LabelGroup: public Item
  {
  public:

    //! Constructor, creating a label that contains other labels.
    //LabelGroup(QList<Label> labels, int lay_id);
    LabelGroup(int lay_id);

    //! Destructor, does nothing for now.
    ~LabelGroup(){}


  };  // end of LabelGroup class


} // end of prim namespace


#endif
