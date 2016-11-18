#ifndef _GUI_DESIGN_WIDGET_H_
#define _GUI_DESIGN_WIDGET_H_

#include <QObject>
#include <QWidget>

// graphics
#include <QGraphicsScene>
#include <QGraphicsView>

// interrupts
#include <QMouseEvent>
#include <QWheelEvent>

#include "primitives/layer.h"
#include "primitives/dbdot.h"

namespace gui{

class DesignWidget : public QGraphicsView
{
  Q_OBJECT

public:

  // constructor
  DesignWidget(QWidget *parent = 0);

  // deconstructor
  ~DesignWidget();

  // accessors

  // add a new layer with the given name. If no name is given, a default scheme
  // is used. Checks if the layer already exists.
  void addLayer();
  void addLayer(const QString &name);

  // attempt to remove a layer, either by name or by index
  void removeLayer(const QString &name);
  void removeLayer(int n);

  // returns a pointer to a requested layer if it exists.
  prim::Layer *getLayer(const QString &name);
  prim::Layer *getLayer(int n);

  void setLayer(const QString &name);
  void setLayer(int n);

  // add a new dangling bond to the current layer
  void addDB(qreal x, qreal y);

protected:

  // interrupts

  void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
  void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;

  void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;
  void wheelZoom(QWheelEvent *e);
  void wheelPan(QWheelEvent *e);


private:

  QGraphicsScene *scene;

  QList<prim::Layer*> layers;
  prim::Layer *top_layer;

  bool clicked;
  QPoint old_mouse_pos;
  QPoint wheel_deg;

};

} // end gui namespace


#endif
