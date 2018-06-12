/** @file:     textlabel.h
 *  @author:   Samuel
 *  @created:  2018.06.12  - Not long after Fallout 76 was announced
 *  @editted:  2018.06.12  - Samuel
 *  @license:  GNU LGPL v3
 *
 *  @brief:     Label widget for showing user-defined static or dynamic labels.
 */

#ifndef _GUI_PR_TEXTLABEL_H_
#define _GUI_PR_TEXTLABEL_H_

#include "../item.h"

namespace prim{

  class TextLabel: public Item
  {
  public:
    //! Constructor that takes the text, position, and layer id.
    TextLabel(const QString &text, QRectF rect, int lay_id);

    //! Destructor
    ~TextLabel() {};

    //! Update contained text.
    void setText(const QString &text) {block_text = text;}

    //! Return current text.
    QString text() {return block_text;}

    //! Return current rectangle.
    QRectF rect() {return block_rect;}

    //! Dialog for editing text of the given TextLabel.
    //static QWidget *EditDialog(TextLabel *text_label);

    //! Bounding rectangle
    virtual QRectF boundingRect() const override;

    //! Draw text.
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;

  private:
    QString block_text;
    QRectF block_rect;

    //static QWidget *text_edit_diag; // dialog for editing text
  };  // end of TextLabel class

} // end of prim namespace

#endif
