// @file:     ghost.cc
// @author:   Jake
// @created:  2016.11.24
// @editted:  2017.06.07  - Jake
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



//GHOSTBOX CLASS
prim::GhostBox::GhostBox(prim::Item *item, prim::Item *parent, QColor *pcol)
  : Item(prim::Item::GhostBox, 0, parent), pcol(pcol)
{
  setPos(item->pos());
  width = static_cast<prim::Electrode*>(item)->getwidth();
  height = static_cast<prim::Electrode*>(item)->getheight();
}

QRectF prim::GhostBox::boundingRect() const
{
  // QPoint point = QWidget::mapFromGlobal(QCursor::pos());
  // return QRectF(point.x(), point.y(), width, height);
  return QRectF(0, 0, width, height);
}

void prim::GhostBox::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
  painter->setPen(Qt::NoPen);
  painter->setBrush(*pcol);
  painter->drawRect(boundingRect());
  // qDebug() << QObject::tr("Drawing GhostBox!");
}

void prim::GhostBox::constructStatics()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  // diameter = gui_settings->get<qreal>("ghost/dot_diameter")*scale_factor;
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

  // qDebug() << QObject::tr("Deleting Ghost Box");
  box_sources.clear();
  for(prim::GhostBox *box : boxes)
    delete box;
  boxes.clear();

  setPos(0,0);
  setValid(true);

  anchor=0;
  valid_hash.clear();

  aggnode.reset();

  hide();
}


void prim::Ghost::prepare(const QList<prim::Item*> &items)
{
  cleanGhost();
  for(prim::Item *item : items)
    prepareItem(item, &aggnode);
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


QList<prim::Item*> prim::Ghost::getTopItems() const
{
  // each top item corresponds to one of the top level nodes in aggnode

  // qDebug() << QObject::tr("Ghost::getTopItems");
  QList<prim::Item*> items;
  for(prim::AggNode *node : aggnode.nodes)
    items.append(getNodeItem(node));
  return items;
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
        if(static_cast<prim::Item*>(cand)->item_type == prim::Item::LatticeDot &&
            cand->flags() & QGraphicsItem::ItemIsSelectable){
          ldot = static_cast<prim::LatticeDot*>(cand);
          break;
        }
      ldots.append(ldot);
    }
  }
  return ldots;
}

prim::LatticeDot *prim::Ghost::getLatticeDot(prim::DBDot *db)
{
  // get index of source
  int index = sources.indexOf(static_cast<prim::Item*>(db));
  if(index==-1)
    return 0;

  // search for LatticeDot under GhostDot
  for(QGraphicsItem *cand : scene()->items(dots.at(index)->scenePos()))
    if(static_cast<prim::Item*>(cand)->item_type == prim::Item::LatticeDot &&
      cand->flags() & QGraphicsItem::ItemIsSelectable){
        return static_cast<prim::LatticeDot*>(cand);
      }

  // no valid LatticeDot found, return 0
  return 0;
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

  // invalid if a dangling bond is associated with no selectable lattice dot or
  // an unselectable lattice dot
  for(int i=0; i<sources.count(); i++)
    if(sources.at(i)->item_type != prim::Item::DBDot)
      continue;
    else if(ldots.at(i)==0 || ldots.at(i)->flags()^QGraphicsItem::ItemIsSelectable)
      return false;

  return true;
}


QPointF prim::Ghost::moveOffset() const
{
  if(sources.count())
    return dots.first()->scenePos()-sources.first()->scenePos();
  else
    return QPointF();
}


QRectF prim::Ghost::boundingRect() const
{
  return QRectF();
}

void prim::Ghost::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*)
{}



void prim::Ghost::echoTopIndices()
{
  QString s;
  echoNode(s, &aggnode);
  qDebug() << s;
}

void prim::Ghost::echoNode(QString &s, prim::AggNode *node)
{
  qDebug() << QObject::tr("Entering node: ind=%1 :: count=%2").arg(node->index).arg((node->nodes).count());
  if(node->index<0){
    // new Aggregate
    s += '[';
    for(prim::AggNode *n : node->nodes)
      echoNode(s, n);
    s += ']';
  }
  else
    s += QString("%1,").arg(node->index);
}


// PRIVATE METHODS


prim::Ghost::Ghost()
 : Item(prim::Item::Ghost)
{
  valid=false;
  cleanGhost();
  setVisible(false);
}

void prim::Ghost::createGhostDot(prim::Item *item)
{
  if(item->item_type == prim::Item::Aggregate)
    qWarning() << QObject::tr("Creating a ghost dot for an aggregate...");

  prim::GhostDot *dot = new prim::GhostDot(item, this, &col);

  dots.append(dot);
  sources.append(item);
}


void prim::Ghost::createGhostBox(prim::Item *item)
{
  // qDebug() << QObject::tr("Creating Ghost Box");
  // prim::GhostDot *dot = new prim::GhostDot(item, this, &col);
  prim::GhostBox *box = new prim::GhostBox(item, this, &col);
  //
  boxes.append(box);
  box_sources.append(item);
}


void prim::Ghost::prepareItem(prim::Item *item, prim::AggNode *node)
{
  prim::AggNode *new_node;
  if(item->item_type == prim::Item::Aggregate){
    // add a new list-type IndexList
    new_node = new prim::AggNode();
    new_node->source_type = prim::AggNode::Aggregate;
    node->nodes.append(new_node);
    // add each item in the Aggregate to the new list
    prim::Aggregate *agg = static_cast<prim::Aggregate*>(item);
    for(prim::Item *it : agg->getChildren())
      prepareItem(it, new_node);
  }
  else if(item->item_type == prim::Item::DBDot){
    // add a new index-type IndexList
    new_node = new prim::AggNode(sources.count());
    new_node->source_type = prim::AggNode::DBDot;
    node->nodes.append(new_node);
    // create a GhostDot for the Item
    createGhostDot(item);
  }
  else if(item->item_type == prim::Item::Electrode){
    // add a new index-type IndexList
    new_node = new prim::AggNode(box_sources.count());
    new_node->source_type = prim::AggNode::Electrode;
    node->nodes.append(new_node);
    // create a GhostBox for the Item
    createGhostBox(item);
  }
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
}


prim::Item *prim::Ghost::getNodeItem(prim::AggNode *node) const
{

  // qDebug() << QObject::tr("node index: %1").arg(node->index);
  if(node->index<0){
    // node corresponds to an Aggregate, the parent of the first item in the Aggregate
    return static_cast<prim::Item*>(getNodeItem(node->nodes.first())->parentItem());
  }
  else if(node->source_type == prim::AggNode::DBDot)
    return sources.at(node->index);
  else if(node->source_type == prim::AggNode::Electrode)
    return box_sources.at(node->index);
  else
    return 0;

}
