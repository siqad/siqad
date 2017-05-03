// @file:     design_panel.h
// @author:   Jake
// @created:  2016.11.02
// @editted:  2017.05.01  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Top level widget for the design panel. Contains all layers and
//            functionality for reating, selecting, moving, etc. object in the
//            design.

#ifndef _GUI_DESIGN_PANEL_H_
#define _GUI_DESIGN_PANEL_H_

#include <QObject>
#include <QWidget>

// graphics
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsItemGroup>

// interrupts
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>

#include "primitives/layer.h"
#include "primitives/dbdot.h"
#include "primitives/ghost.h"
#include "primitives/items.h"

#include "primitives/emitter.h"

namespace gui{

/*
Highest level of the design window visualization. Contains all functionality
for creating and viewing dangling bonds.
*/
class DesignPanel : public QGraphicsView
{
  Q_OBJECT

public:

  enum ToolType{SelectTool, DragTool, DBGenTool};

  // constructor
  DesignPanel(QWidget *parent = 0);

  // deconstructor
  ~DesignPanel();

  // accessors

  // add a new layer with the given name. If no name is given, a default scheme
  // is used. Checks if the layer already exists.
  void addLayer();
  void addLayer(const QString &name);

  // attempt to remove a layer, either by name or by index
  void removeLayer(const QString &name);
  void removeLayer(int n);

  // returns a pointer to a requested layer if it exists.
  prim::Layer* getLayer(const QString &name);
  prim::Layer* getLayer(int n);

  // get a list of all the surface dangling bonds
  QList<prim::DBDot*> getSurfaceDBs();

  // change the top layer to the requested layer
  void setLayer(const QString &name);
  void setLayer(int n);

  // resets the drawing layer and build a lattice from the given file.
  // If no file is given, the default lattice is used
  void buildLattice(const QString fname=QString());

  // update the tool type
  void setTool(ToolType tool);

  // update the fill values for the surface dangling bonds
  void setFills(float *fills);


public slots:

  void selectClicked(QGraphicsItem *item);

protected:

  // interrupts

  void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
  void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
  void mouseDoubleClickEvent(QMouseEvent *e) Q_DECL_OVERRIDE;

  void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;

  void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
  void keyReleaseEvent(QKeyEvent *e) Q_DECL_OVERRIDE;


private:

  QGraphicsScene *scene;

  prim::Ghost *ghost;      // temporary item (moving and paste)
  QGraphicsItemGroup *clipboard;  // deep copy storage for copy/paste

  // functional layers: order {lattice, db-surface, electrode1, electrode2, ...}
  QList<prim::Layer*> layers;
  prim::Layer *top_layer;     // new items added to this layer

  // interrupt parameters
  bool clicked;
  bool ghosting;
  bool moving;

  qreal snap_diameter;
  prim::DBDot *snap_target;

  //QGraphicsItem *clicked_item;

  QPoint old_mouse_pos;
  QPoint wheel_deg;

  ToolType tool_type;

  // ASSIST METHODS

  void wheelZoom(QWheelEvent *e, bool boost);
  void wheelPan(bool boost);

  // assert bounds on zoom range
  void boundZoom(qreal *ds);

  void mousePress();

  // filter items selected by the scene depending on wthe tool_type. If true,
  // rejects all items in the lattice layer, otherwise includes only items in
  // the lattice layer.
  void filterSelection(bool select_flag);
  bool inLattice(QGraphicsItem *item);

  // create DBDots in the surface on top of all lattice dots in selected
  void createDBs();

  // create a dangling bond in the surface with the same physical location as
  // the given dot.
  void createDB(prim::DBDot *dot);
  void destroyDB(prim::DBDot *dot);


  void createAggregate();
  void destroyAggregates();

  void deleteSelected();
  void deleteItem(QGraphicsItem *item);


  // create a deep copy of given items and store in clipboard
  void saveToClipBoard(QList<QGraphicsItem*> items);
  void saveToClipBoard(QGraphicsItem *item);


  // GHOST METHODS

  // create a mobile image of either the selected item or the clipboard
  void createGhost(bool selected);

  // make a deep copy of the ghost source in place
  void plantGhost();

  // destroy the ghost
  void destroyGhost();

  // snap the ghost to nearest matching lattice site, returns true if the snap_target
  // was updated (need to change ghost location), *offset will be the offset
  // between the offset of the snap ghost location
  bool snapGhost(QPointF scene_pos, QPointF *offset);

  // moving routine
  void initMove();
  void completeMove();

};

} // end gui namespace


#endif
