// @file:     global.h
// @author:   Jake
// @created:  2017.05.15
// @editted:  2017.05.15  - Jake
// @license:  GNU LGPL v3
//
// @desc: Useful functions

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <QTextStream>
#include <QDebug>
#include <QMetaEnum>
#include <QStringList>

namespace gui{

  // Forward declarations
  class Unit;

  // Globally relevant enums
  enum ToolType{NoneTool, SelectTool, DragTool, DBGenTool, MeasureTool, ElectrodeTool,
                AFMAreaTool, AFMPathTool, ScreenshotAreaTool, ScaleBarAnchorTool,
                LabelTool, ElectrodePolyTool};
  enum DisplayMode{DesignMode, SimDisplayMode, ScreenshotMode};

  // Handy unit functions
  class Unit : public QObject
  {
    Q_OBJECT

  public:

    //! Common metric distance units in ascending order.
    enum DistanceUnit{pm, ang, nm, um, mm, m};
    Q_ENUM(DistanceUnit);

    //! Return the corresponding float value for the given distance unit. Upon
    //! error, return -1.
    static float distanceUnitValue(DistanceUnit);

    //! Convert the value from one distance unit to another distance unit.
    static float valueConvertDistanceUnit(float val, DistanceUnit from_unit, DistanceUnit to_unit);

    //! Return the QString of a unit.
    static QString distanceUnitString(DistanceUnit du);

    //! Return a QString QList of available units from start unit to end unit.
    static QStringList distanceUnitStringList(DistanceUnit start, DistanceUnit end);

    //! Return the corresponding DistanceUnit enum of a QString.
    static DistanceUnit stringToDistanceUnit(QString unit);
  };

  // Global variables
  extern QString python_path;

} // end gui namespace

struct QPairFirstComparer
{
  template<typename T1, typename T2>
  bool operator()(const QPair<T1,T2> &a, const QPair<T1,T2> &b) const
  {
    return a.first < b.first;
  }
};

struct QPairSecondComparer
{
  template<typename T1, typename T2>
  bool operator()(const QPair<T1,T2> &a, const QPair<T1,T2> &b) const
  {
    return a.second < b.second;
  }
};

struct QPairFirstReverseComparer
{
  template<typename T1, typename T2>
  bool operator()(const QPair<T1,T2> &a, const QPair<T1,T2> &b) const
  {
    return a.first > b.first;
  }
};

struct QPairSecondReverseComparer
{
  template<typename T1, typename T2>
  bool operator()(const QPair<T1,T2> &a, const QPair<T1,T2> &b) const
  {
    return a.second > b.second;
  }
};
#endif
