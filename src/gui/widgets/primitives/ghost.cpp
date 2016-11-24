#include "ghost.h"


// GHOSTDOT


prim::GhostDot::GhostDot(QGraphicsItemGroup *parent, QGraphicsItem *item)
  : QGraphicsItem()
{
  settings::GUISettings gui_settings;

  qreal scale_fact = gui_settings.get<qreal>("dbdot/scale_fact");
  diameter = gui_settings.get<qreal>("ghost/dot_diameter")*scale_fact;
  col = gui_settings.get<QColor>("ghost/col");

  setPos(item->pos());
  parent->addToGroup(this);
}

QRectF prim::GhostDot::boundingRect() const
{
  return QRectF(-.5*diameter, -.5*diameter, diameter, diameter);
}


void prim::GhostDot::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  painter->setPen(Qt::NoPen);
  painter->setBrush(col);
  painter->drawEllipse(boundingRect());

}







// GHOST CLASS

prim::Ghost::Ghost(QGraphicsScene *scene)
  : QGraphicsItemGroup()
{
  if(scene != 0)
    scene->addItem(this);
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
}


void prim::Ghost::prepare(QGraphicsItem *item)
{
  cleanGhost();
  prepareItem(item);
  zeroGhost();
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
}

void prim::Ghost::rotate(bool cw)
{
  qreal deg = cw ? -90 : 90;
  setRotation(rotation() + deg);
}


QPointF prim::Ghost::getAnchor() const
{
  return dots.at(0)->pos();
}


// ACCESSORS



// HELPER METHODS


void prim::Ghost::createGhostDot(QGraphicsItem *item)
{
  settings::GUISettings gui_settings;
  prim::GhostDot *dot = new prim::GhostDot(this, item);

  dots.append(dot);
  source.append(item);
}


void prim::Ghost::prepareItem(QGraphicsItem *item)
{
  QGraphicsItem *cp=0;
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
  prim::DBDot *db;
  if(item->childItems().count()==0){
    p = (prim::MyItem *)item;
    if(p->item_type == prim::MyItem::DotType){
      db = (prim::DBDot *)p;
      return db->inLattice();
    }
  }
  return false;
}
