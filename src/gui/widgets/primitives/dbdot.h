#ifndef _GUI_PR_DBDOT_H_
#define _GUI_PR_DBDOT_H_

#include <QObject>
#include <QGraphicsItem>

namespace prim{


class DBDot: public QGraphicsItem
{
  Q_OBJECT

public:

private:

  int n, m; // lattice vector decomp

};


} // end prim namespace


#endif
