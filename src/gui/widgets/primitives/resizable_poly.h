/** @file:     resiable_poly.h
 *  @author:   Nathan
 *  @created:  2018.08.08
 *  @editted:  2018.08.08 - Nathan
 *  @license:  GNU LGPL v3
 *
 *  @brief:     Function prototypes for the ResizablePoly object.
 */

#ifndef _GUI_PR_RESIZABLE_POLY_H_
#define _GUI_PR_RESIZABLE_POLY_H_


#include <QtWidgets>
#include "polygon_handle.h"
#include "item.h"

namespace prim{

  class ResizablePoly: public Item
  {
  public:
    ResizablePoly(ItemType type, const QPolygonF &poly, const QRectF &scene_rect, int lay_id);
    ~ResizablePoly();
    void setPolygon(QPolygonF poly_in){poly = poly_in;}
    QPolygonF getPolygon() const {return poly;}
    QPolygonF getTranslatedPolygon();
    void setSceneRect(const QRectF &scene_rect_in){scene_rect = scene_rect_in;}
    QRectF sceneRect() const {return scene_rect;}
    void createHandles(bool remake = false);
    virtual void moveItemBy(qreal dx, qreal dy) override;
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;
    void setRect(QRectF scene_rect_in, bool translate = false);
    void showStatus();
    QList<prim::PolygonHandle*> getHandles(){return poly_handles;}
  protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

  private:
    void initResizablePoly(int lay_id, QPolygonF poly_in, QRectF scene_rect_in);

    QRectF scene_rect;
    QPolygonF poly;
    QList<prim::PolygonHandle*> poly_handles;
  };

} //end prim namespace

#endif
