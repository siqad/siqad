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

namespace gui{

  enum ToolType{NoneTool, SelectTool, DragTool, DBGenTool, MeasureTool, ElectrodeTool,
      AFMAreaTool, AFMPathTool, ScreenshotAreaTool};
  enum DisplayMode{DesignMode, SimDisplayMode, ScreenshotMode};

} // end global namespace

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
