// @file:     ghost.cpp
// @author:   Jake
// @created:  2016.11.24
// @editted:  2017.05.01  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Ghost and GhostDot definitions


#include "ghost.h"


// GHOSTDOT


prim::GhostDot::GhostDot(QGraphicsItemGroup *parent, QGraphicsItem *item, QColor *col)
  : prim::MyItem(prim::MyItem::GhostDotType, -1), pcol(col)
{
  settings::GUISettings gui_settings;

  qreal scale_fact = gui_settings.get<qreal>("dbdot/scale_fact");
  diameter = gui_settings.get<qreal>("ghost/dot_diameter")*scale_fact;

  setPos(item->pos());
  parent->addToGroup(this);
}

QRectF prim::GhostDot::boundingRect() const
{
  return QRectF(-.5*diameter, -.5*diameter, diameter, diameter);
}


void prim::GhostDot::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
  painter->setPen(Qt::NoPen);
  painter->setBrush(*pcol);
  painter->drawEllipse(boundingRect());
}







// GHOST CLASS

prim::Ghost::Ghost(QGraphicsScene *scene)
  : QGraphicsItemGroup()
{
  settings::GUISettings gui_settings;

  if(scene != 0)
    scene->addItem(this);

  // don't make any asusmption that initial location is valid
  col = gui_settings.get<QColor>("ghost/invalid_col");
  valid=false;
}


prim::Ghost::~Ghost()
{
  cleanGhost();
}


// INITIALIZERS

void prim::Ghost::prepare(const QList<QGraphicsItem*> &items)
{
  cleanGhost();

  for(int i=0; i<items.count(); i++)
    prepareItem(items.at(i));

  zeroGhost();

  // set valid to false until snapping begins
  valid = false;
}


void prim::Ghost::prepare(QGraphicsItem *item)
{
  cleanGhost();
  prepareItem(item);
  zeroGhost();

  // set valid to false until snapping begins
  valid = false;
}

QList<prim::DBDot*> prim::Ghost::getTargets()
{
  // set ghost as invisible
  setVisible(false);
  QList<prim::DBDot*> targets;

  QPointF pos;
  QList<QGraphicsItem*> cands;
  QGraphicsItem *target;

  for(int i=0; i<dots.count(); i++){
    pos = mapToScene(dots.at(i)->pos());
    cands = scene()->items(pos);
    target = 0;
    for(int j=0; j<cands.count(); j++){
      if(inLattice(cands.at(j)) && (cands.at(j)->flags() & QGraphicsItem::ItemIsSelectable)){
        target = cands.at(j);
        break;
      }
    }
    targets.append((prim::DBDot*)target);
  }

  setVisible(true);

  return targets;
}

QList<QGraphicsItem*> prim::Ghost::getSource() const
{
  return source;
}

void prim::Ghost::flip(bool horiz)
{
  QGraphicsItem *item = 0;

  for(int i=0; i<childItems().count(); i++){
    item = childItems().at(i);
    if(horiz)
      item->setX(-item->x());
    else
      item->setY(-item->y());
  }

  updateValid();
}

void prim::Ghost::rotate(bool cw)
{
  qreal deg = cw ? -90 : 90;
  setRotation(rotation() + deg);

  updateValid();
}


QPointF prim::Ghost::getAnchor() const
{
  return dots.at(0)->pos();
}


void prim::Ghost::updateValid()
{
  if(checkValid()!=valid)
    setValid(!valid);
}

void prim::Ghost::setValid(bool flag)
{
  settings::GUISettings gui_settings;

  if(valid != flag){
    valid = flag;
    if(valid)
      col = gui_settings.get<QColor>("ghost/valid_col");
    else
      col = gui_settings.get<QColor>("ghost/invalid_col");
  }
}

bool prim::Ghost::checkValid(QPointF offset)
{
  return true;
  // get lattice beneath ghost dots
  QList<prim::DBDot*> dbs = getLattice(offset);

  // ghost is invalid if there is a ghost dot which does not correspond to a
  // selectable lattice db.
  for(QList<prim::DBDot*>::iterator it=dbs.begin(); it!=dbs.end(); ++it){
    if(!(*it) || !((*it)->flags() & QGraphicsItem::ItemIsSelectable))
      return false;
  }

  return true;
}


// ACCESSORS



// HELPER METHODS


void prim::Ghost::createGhostDot(QGraphicsItem *item)
{
  settings::GUISettings gui_settings;
  prim::GhostDot *dot = new prim::GhostDot(this, item, &col);

  dots.append(dot);
  source.append(item);
}


void prim::Ghost::prepareItem(QGraphicsItem *item)
{
  if(item->childItems().count()==0)
    createGhostDot(item);
  else{
    for(int i=0; i<item->childItems().count(); i++)
      prepareItem(item->childItems().at(i));
  }
}


void prim::Ghost::cleanGhost()
{
  // delete all GhostDot children
  QList<QGraphicsItem *> children = childItems();
  for(int i=0; i<children.count(); i++){
    removeFromGroup(children.at(i));
    delete children.at(i);
  }

  // free dots and source lists... dots should have already been deleted
  source.clear();
  dots.clear();

  valid = false;
}

void prim::Ghost::zeroGhost()
{
  QPointF center = boundingRect().center();
  QGraphicsItem *item=0;
  for(int i=0; i<childItems().count(); i++){
    item = childItems().at(i);
    item->setPos((item->pos()-center));
  }

  setPos(center);

  // set transformation parameters
  //setTransformOriginPoint(boundingRect().center());
}


bool prim::Ghost::inLattice(QGraphicsItem *item)
{
  prim::MyItem *p;
  if(item->childItems().count()==0){
    p = (prim::MyItem *)item;
    return p->layer==0;
  }
  return false;
}


// for each ghost dot, check if there is a lattice dot which intersects the
// location of the ghost dot. If true, append that dot, else append a null pointer
QList<prim::DBDot*> prim::Ghost::getLattice(QPointF offset)
{
  QList<prim::DBDot*> lattice;
  prim::MyItem *p=0;

  // for each ghost dot
  for(int i=0; i<dots.count(); i++){

    // get list of items which intersect the ghost dot
    QPointF pos = mapToScene(dots.at(i)->pos());
    QList<QGraphicsItem*> cands = scene()->items(pos+offset);

    // check for lattice dbdot in candidates
    prim::DBDot *db=0;
    for(QList<QGraphicsItem*>::iterator it=cands.begin(); it != cands.end(); ++it){
      if((*it)->childItems().count()==0){
        p = (prim::MyItem*)*it;
        if(p->layer==0){
          db = (prim::DBDot*)*it;
          break;
        }
      }
    }
    lattice.append(db);
  }

  return lattice;
}
