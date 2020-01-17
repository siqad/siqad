// @file:     textlabel.cc
// @author:   Samuel
// @created:  2018.06.12  - And an early glimpse into TES6
// @editted:  2018.06.12  - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Text label implementation


#include "textlabel.h"
#include "settings/settings.h"

#include <QInputDialog>

namespace prim {

TextLabel::TextLabel(const QRectF &rect, int lay_id, const QString &text)
  : prim::ResizableRect(prim::Item::TextLabel, rect, lay_id)
{
  initTextLabel(rect, text);
}

TextLabel::TextLabel(const QRectF &rect, int lay_id)
  : prim::ResizableRect(prim::Item::TextLabel, rect, lay_id)
{
  initTextLabel(rect, textPrompt());
}

void TextLabel::setText(const QString &text)
{
  block_text = text;
  update();
}

QString TextLabel::textPrompt(const QString &default_text, bool *ok)
{
  return QInputDialog::getMultiLineText(0, QObject::tr("Change label text"),
                                        QObject::tr("Text"), default_text, ok);
}

QRectF TextLabel::boundingRect() const
{
  QPointF diag = sceneRect().bottomRight() - sceneRect().topLeft();
  return QRectF(0, 0, diag.x(), diag.y());
}

void TextLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  // draw the background rectangle
  painter->setPen(Qt::NoPen);
  painter->setPen(QPen(QColor(255,255,255), 10));
  painter->setBrush(QColor(255,255,255));
  painter->drawRect(boundingRect());

  // draw the text
  painter->setPen(QPen(QColor(0,0,0), 10));
  QFont font = painter->font();
  font.setPointSize(font.pointSize() * 5);
  painter->setFont(font);
  painter->drawText(boundingRect(), Qt::AlignCenter, text());
}

prim::Item *TextLabel::deepCopy() const
{
  return new TextLabel(sceneRect(), layer_id, block_text);
}

void TextLabel::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
{
  //setText(textPrompt(text()));
  bool ok;
  QString new_text = textPrompt(text(), &ok);
  if (ok)
    emit prim::Emitter::instance()->editTextLabel(this, new_text);
}

void TextLabel::initTextLabel(const QRectF &rect, const QString &text)
{
  block_text = text;
  block_rect = rect;
  setPos(block_rect.topLeft());
  setFlag(QGraphicsItem::ItemIsSelectable, true);
}

} // end of prim namespace
