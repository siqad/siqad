// @file:     afmarea.h
// @author:   Samuel
// @created:  2018.03.15
// @editted:  2018.03.15 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Class that contains information for 2D AFM scans

#ifndef _PRIM_AFMAREA_H_
#define _PRIM_AFMAREA_H_

#include "resizablerect.h"

namespace prim{

  class AFMArea : public ResizableRect
  {
  public:

    //! Constructor that takes the top left and bottom right points of the area.
    //! The order of points 1 and 2 doesn't matter.
    AFMArea(int lay_id, const QRectF &scene_rect, bool orientation=true,
        float z_spd=-1, float h_spd=-1, float v_spd=-1, float v_disp=-1);

    //! The order of points 1 and 2 doesn't matter.
    AFMArea(int lay_id, QStringList points, bool orientation=true,
        float z_spd=-1, float h_spd=-1, float v_spd=-1, float v_disp=-1);

    //! Constructor that creates an AFMArea from a loaded design file.
    AFMArea(QXmlStreamReader *, QGraphicsScene *);

    //! Destructor.
    ~AFMArea() {}

    //! Save to XML.
    virtual void saveItems(QXmlStreamWriter *) const override;


    // Accessors

    //! Set the orientation of the scan. "Horizontal" refers to the x-axis if
    //! orientation is set to true, y-axis if false.
    void setOrientation(bool x_oriented) {h_orientation = x_oriented;}
    bool horizontalOrientation() const {return h_orientation;}

    //! Set the z-axis (out-of-plane) tip movement speed.
    void setZSpeed(float speed) {z_speed = speed;}
    float zSpeed() const {return z_speed;}

    //! Set the horizontal (in-plane) tip movement speed.
    void setHorizontalSpeed(float speed) {h_speed = speed;}
    float horizontalSpeed() const {return h_speed;}

    //! Set the vertical (in-plane) tip movement speed.
    void setVerticalSpeed(float speed) {v_speed = speed;}
    float verticalSpeed() const {return v_speed;}

    //! Set the vertical (in-plane) displacement after each horizontal scan.
    void setVerticalDisplacementBetweenScans(float d) {v_displacement = d;}
    float verticalDisplacementBetweenScans() const {return v_displacement;}

    //! Generate the path used for simulation and return a QList containing
    //! locations and timestamps.
    //QList<global::AFMPathTimed> generateSimulationPath();

    //! Return the class default property map
    virtual gui::PropertyMap *classPropertyMap() override {return &default_class_properties;}

    virtual QList<QAction*> contextMenuActions() override {return actions_list;}
    virtual void performAction(QAction *action) override;

    // Graphics

    //! Bounding rect for graphics calculations.
    virtual QRectF boundingRect() const override;

    //! Paint function.
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;

    //! Behavior when this object is copied.
    virtual Item *deepCopy() const override;


  protected:

    //! Show AFM area config dialog when selected.
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *) Q_DECL_OVERRIDE;

    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *) Q_DECL_OVERRIDE;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *) Q_DECL_OVERRIDE;


  private:

    //! Initialize AFMArea, commonly called by all constructors. Point order
    //! doesn't matter.
    void initAFMArea(int lay_id, const QRectF &scene_rect,
        bool orientation, float z_spd, float h_spd, float v_spd, float v_disp);

    //! Initialize static class variables.
    void prepareStatics();
    void createActions();
    void showProps();

    // properties of this item class
    static gui::PropertyMap default_class_properties; //! Default properties for this class

    // AFM tip movement parameters
    bool h_orientation; //! "horizontal" direction = orientation ? x, y
    float z_speed;    //! Z-axis movement speed of the tip (out-of-plane).
    float h_speed;    //! Horizontal movement speed of the tip (in-plane).
    float v_speed;    //! Vertical movement speed of the tip (in-plane).
    float v_displacement; //! Vertical displaycement between horizontal scans.

    // Graphical variables
    static qreal area_border_width;     //! Border width of the AFM area.
    static prim::Item::StateColors area_border_col; //! Border color of the AFM area.
    static prim::Item::StateColors area_fill_col; //! Background fill color of the area.
    static qreal scan_path_width;       //! AFM scan path preview width.
    static prim::Item::StateColors scan_path_fill_col; //! AFM scan path preview fill color.

    // Resize
    QList<QAction*> actions_list;
    QAction* action_show_prop;
  };
}


#endif
