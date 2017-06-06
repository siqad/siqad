// @file:     ghost.cc
// @author:   Jake
// @created:  2016.11.24
// @editted:  2017.06.02  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Implementation of GhostDot and Ghost


#include "ghost.h"


// GHOSTDOT CLASS
qreal prim::GhostDot::diameter = -1;

prim::GhostDot::GhostDot(prim::Item *item, prim::Item *parent, QColor *pcol)
 : Item(prim::Item::GhostDot, 0, parent), pcol(pcol)
{
  //settings::GUISettings *gui_settings = settings::GUISettings::instance();

  if(diameter<0)
    constructStatics();

  // create dot at item center, assumes item local boundingRect centered at 0.
  setPos(item->pos());
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

void prim::GhostDot::constructStatics()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  diameter = gui_settings->get<qreal>("ghost/dot_diameter")*scale_factor;
}





// GHOST CLASS
prim::Ghost* prim::Ghost::inst = 0;

prim::Ghost* prim::Ghost::instance()
{
  if(inst==0)
    inst = new prim::Ghost();
  return inst;
}


void prim::Ghost::cleanGhost()
{
  // clear lists
  sources.clear();
  for(prim::GhostDot *dot : dots)
    delete dot;
  dots.clear();

  setPos(0,0);
  setValid(true);

  anchor=0;
  valid_hash.clear();

  hide();
}


void prim::Ghost::prepare(const QList<prim::Item*> &items)
{
  cleanGhost();
  for(prim::Item *item : items)
    prepareItem(item);
  zeroGhost();
  setAnchor();
  show();
}



void prim::Ghost::prepare(prim::Item *item)
{
  QList<prim::Item*> items;
  items.append(item);
  prepare(items);
}

void prim::Ghost::moveTo(QPointF pos)
{
  setPos(pos-zero_offset);
}

QList<prim::LatticeDot*> prim::Ghost::getLattice(const QPointF &offset) const
{
  QList<prim::LatticeDot*> ldots;
  for(int i=0; i<sources.count(); i++){
    if(sources.at(i)->item_type != prim::Item::DBDot)
      ldots.append(0);
    else{
      // get list of items which intersect the ghost dot
      QList<QGraphicsItem*> cands = scene()->items(dots.at(i)->scenePos()+offset);
      // use first LatticeDot candidate
      prim::LatticeDot *ldot=0;
      for(QGraphicsItem *cand : cands)
        if(static_cast<prim::Item*>(cand)->item_type == prim::Item::LatticeDot){
          ldot = static_cast<prim::LatticeDot*>(cand);
          break;
        }
      ldots.append(ldot);
    }
  }
  return ldots;
}


QPointF prim::Ghost::freeAnchor(QPointF scene_pos)
{
  return scene_pos+anchor_offset;
}


void prim::Ghost::setValid(bool val)
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  if(valid != val){
    valid = val;
    if(valid)
      col = gui_settings->get<QColor>("ghost/valid_col");
    else
      col = gui_settings->get<QColor>("ghost/invalid_col");
  }
}


bool prim::Ghost::checkValid(const QPointF &offset)
{
  QList<prim::LatticeDot*> ldots = getLattice(offset);

  // invalid if a dangling bond is associated with no lattice dot
  for(int i=0; i<sources.count(); i++)
    if(sources.at(i)->item_type == prim::Item::DBDot && ldots.at(i)==0)
      return false;

  return true;
}



QRectF prim::Ghost::boundingRect() const
{
  return QRectF();
}

void prim::Ghost::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*)
{}

prim::Ghost::Ghost()
 : Item(prim::Item::Ghost)
{
  cleanGhost();
  setVisible(false);
}

void prim::Ghost::createGhostDot(prim::Item *item)
{
  if(item->item_type == prim::Item::Aggregate)
    qWarning() << QObject::tr("Creating a ghost dot for an aggregate...");

  prim::GhostDot *dot = new prim::GhostDot(item, this,&col);

  dots.append(dot);
  sources.append(item);
}

void prim::Ghost::prepareItem(prim::Item *item)
{
  if(item->item_type == prim::Item::Aggregate){
    prim::Aggregate *agg = static_cast<prim::Aggregate*>(item);
    for(prim::Item *it : agg->getChildren())
      prepareItem(it);
  }
  else
    createGhostDot(item);
}


void prim::Ghost::updateValid()
{

}


void prim::Ghost::zeroGhost()
{
  // compute center of effective bounding rect for all items
  qreal xmin=0, xmax=0, ymin=0, ymax=0;
  bool unset=true;
  for(prim::GhostDot *dot : dots){
    QRectF rect = dot->boundingRect();
    rect.moveCenter(dot->pos());
    if(unset){
      unset=false;
      xmin = rect.left();
      xmax = rect.right();
      ymin = rect.top();
      ymax = rect.bottom();
    }
    else{
      xmin = qMin(xmin, rect.left());
      xmax = qMax(xmax, rect.right());
      ymin = qMin(ymin, rect.top());
      ymax = qMax(ymax, rect.bottom());
    }
  }

  zero_offset = QPointF(.5*(xmin+xmax), .5*(ymin+ymax));
}


void prim::Ghost::setAnchor()
{
  // find nearest GhostDot to the zero_offset, by Manhattan length
  prim::GhostDot *dot=anchor=0;
  qreal mdist=-1, dist;
  for(int i=0; i<dots.count(); i++){
    if(sources.at(i)->item_type != prim::Item::DBDot)
      continue;
    dot = dots.at(i);
    dist = (dot->scenePos()-zero_offset).manhattanLength();
    if(mdist < 0 || mdist > dist){
      anchor = dot;
      mdist=dist;
    }
  }

  anchor_offset = anchor==0 ? QPointF() : anchor->scenePos() - zero_offset - pos();
  qDebug() << QObject::tr("anchor_offset: %1 :: %2").arg(anchor_offset.x()).arg(anchor_offset.y());
}
