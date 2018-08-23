/** @file:     electrode_poly.h
 *  @author:   Nathan
 *  @created:  2018.07.01
 *  @editted:  2018.08.08 - Nathan
 *  @license:  GNU LGPL v3
 *
 *  @brief:     Function prototypes for the ElectrodePoly object.
 */

#ifndef _GUI_PR_ELECTRODE_POLY_H_
#define _GUI_PR_ELECTRODE_POLY_H_

#include <QtWidgets>
#include "resizable_poly.h"

namespace prim{

  class ElectrodePoly: public ResizablePoly
  {
  public:
    ElectrodePoly(const QPolygonF, int lay_id);
    ElectrodePoly(QXmlStreamReader *ls, QGraphicsScene *scene);
    ~ElectrodePoly();
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;
    virtual Item *deepCopy() const override;
    void saveItems(QXmlStreamWriter *ss) const;
    virtual gui::PropertyMap *classPropertyMap() override {return &default_class_properties;}
    virtual gui::PropertyMap *classPropertyMap() const override {return &default_class_properties;}
    virtual QList<QAction*> contextMenuActions() override {return actions_list;}
    virtual void performAction(QAction *action) override;


  private:
    void constructStatics();
    void createActions();
    void showProps();

    void initElectrodePoly(int lay_id, QPolygonF poly_in);
    static gui::PropertyMap default_class_properties; //! Default properties for this class
    static qreal edge_width;  // proportional width of dot boundary edge
    static QColor fill_col;   // dot fill color (same for all lattice dots)
    static QColor edge_col;     // edge colour, unselected
    static QColor selected_col; // edge colour, selected
    QList<QAction*> actions_list;
    QAction* action_show_prop;

  };

} //end prim namespace

#endif
