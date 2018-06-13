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
    TextLabel(const QRectF &rect, int lay_id, const QString &text);

    //! Constructor that takes the position and layer id without text, prompts
    //! for text immediately.
    TextLabel(const QRectF &rect, int lay_id);

    //! Destructor
    ~TextLabel() {};

    //! Update contained text.
    void setText(const QString &text);

    //! Show a dialog that prompts user for text, and sets label with this text.
    //! The ok pointer indicates whether the user has applied the change or
    //! canceled.
    static QString textPrompt(const QString &default_text=QString(), bool *ok=0);

    //! Return current text.
    QString text() {return block_text;}

    //! Return current rectangle.
    QRectF rect() {return block_rect;}

    //! Bounding rectangle
    virtual QRectF boundingRect() const override;

    //! Draw text.
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;

    //! Return a copy of this text label.
    virtual Item *deepCopy() const;

  protected:

    //! show text editor on double click
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e) override;

  private:
    //! Initialize the text label
    void initTextLabel(const QRectF &rect, const QString &text);

    QString block_text;
    QRectF block_rect;

    //static QWidget *text_edit_diag; // dialog for editing text
  };  // end of TextLabel class

} // end of prim namespace

#endif
