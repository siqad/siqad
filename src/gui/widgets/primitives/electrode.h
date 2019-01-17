/** @file:     electrode.h
 *  @author:   Nathan
 *  @created:  2017.10.27
 *  @editted:  2018.01.17 - Nathan
 *  @license:  GNU LGPL v3
 *
 *  @brief:     Function prototypes for the Electrode object.
 */

#ifndef _GUI_PR_ELECTRODE_H_
#define _GUI_PR_ELECTRODE_H_

#include <QtWidgets>
#include <QTransform>
#include "resizerotaterect.h"

namespace prim{

  // forward declarations
  class Layer;

  class Electrode: public ResizeRotateRect
  {
  public:
    //! Clock types will vary over time. Fix types are static.
    enum ElectrodeType{Fix, Clock};

    //! constructor, creates an electrode given a QRectF.
    Electrode(int lay_id, const QRectF &scene_rect);

    //! constructor, creates an electrode given two x and two y values.
    Electrode(int lay_id, QStringList points);

    //! constructor, creates an electrode from the design file.
    Electrode(QXmlStreamReader *ls, QGraphicsScene *scene, int lay_id);

    //! destructor
    ~Electrode(){}

    // Initializer for initial attribute values.
    void initElectrode(int lay_id, const QRectF &scene_rect);

    // accessors
    // qreal getTopDepth(void){return top_depth;}
    // qreal getDepth(){return elec_depth;}

    // inherited abstract method implementations
    QPainterPath shape() const Q_DECL_OVERRIDE;
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;
    Item *deepCopy() const override;
    // QPolygonF getPolygon();
    // void hideHandles();

    // saving to design
    virtual void saveItems(QXmlStreamWriter *) const override;

    //! Return the class default property map
    virtual gui::PropertyMap *classPropertyMap() override {return &default_class_properties;}
    virtual gui::PropertyMap *classPropertyMap() const override {return &default_class_properties;}
    virtual QList<QAction*> contextMenuActions() override {return actions_list;}
    virtual void performAction(QAction *action) override;

  protected:

    // Mouse events
    /*virtual QVariant itemChange(GraphicsItemChange change,
        const QVariant &value) Q_DECL_OVERRIDE;*/
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;

    // virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;

  private:
    // construct static variables
    void constructStatics();
    void createActions();
    void showProps();
    void requestRotation();

    // properties of this item class
    static gui::PropertyMap default_class_properties; //! Default properties for this class

    // VARIABLES
    // qreal elec_depth;
    // qreal top_depth;

    static qreal edge_width;  // proportional width of dot boundary edge
    static QColor fill_col;   // dot fill color (same for all lattice dots)
    static QColor edge_col;     // edge colour, unselected
    static QColor selected_col; // edge colour, selected

    // Resize
    QList<QAction*> actions_list;
    QAction* action_rotate_prop;
    QAction* action_show_prop;
  };

} // end prim namespace

#endif
