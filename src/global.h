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
      AFMAreaTool, AFMPathTool};
  enum DisplayMode{DesignMode, SimDisplayMode};

} // end global namespace

#endif
