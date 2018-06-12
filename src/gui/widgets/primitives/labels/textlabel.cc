// @file:     textlabel.cc
// @author:   Samuel
// @created:  2018.06.12  - And an early glimpse into TES6
// @editted:  2018.06.12  - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Text label implementation


#include "textlabel.h"
#include "src/settings/settings.h"

namespace prim {

TextLabel::TextLabel(const QString &text, QRectF rect, int lay_id)
  : prim::Item(prim::Item::TextLabel, lay_id), block_text(text), block_rect(rect)
{
  setPos(block_rect.topLeft());
}

  /*
QWidget *TextLabel::EditDialog(TextLabel *text_label)
{
  return 0;
}
  */

QRectF TextLabel::boundingRect() const
{
  QPointF diag = block_rect.bottomRight() - block_rect.topLeft();
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

} // end of prim namespace
