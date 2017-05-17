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

// text output
namespace echo{

  class MFatal{
  public:
    void operator<<(const QString &rhs){
      qFatal(rhs.toLatin1().constData(), 0);
    }
  };

  MFatal& mFatal(){
    static MFatal MFtl;
    return MFtl;
  }

  // standard output
  // QTextStream& qEcho(){
  //   static QTextStream ts(stdout);
  //   return ts;
  // }

} // end echo namespace

#endif
