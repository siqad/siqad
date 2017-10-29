#include <iostream>
#include <ostream>
#include <streambuf>

// null buffer for the null stream, does nothing on overflow request
class NullBuff: public std::streambuf{
public:
  int overflow(int c){return c;}
};

// ostream type object which does nothing
class nostream : public std::ostream{
public:
  nostream() : std::ostream(&nobuff){}
private:
  NullBuff nobuff;
};

// verbosity controlled output to arbitrary std::ostream (std::cout by default).
// usage:
//        Logger log(Logger::CRT);
//        log.echo() << 42 << "some text" << std::endl;   // outputs to std:cout
//        log.debug() << foo;                             // does nothing
//
//        std::ofstream ofs("foo.log", std::ofstream::out);
//        Logger log(Logger::MSG, ofs);
//        log.echo() << 42 << ...;      // outputs to log file
//        log.warning() << ...;         // does nothing
//        ofs.close();                  // !! dont forget

class Logger{
public:

  enum {MSG, CRT, WRN, DBG};

  Logger(int verbosity=0, std::ostream &os = std::cout): verbosity(verbosity), os(os){}

  std::ostream& debug() {return verbosity >= DBG ? os : nos;}
  std::ostream& warning() {return verbosity >= WRN ? os : nos;}
  std::ostream& critical() {return verbosity >= CRT ? os : nos;}
  std::ostream& echo() {return verbosity >= MSG ? os : nos;}

private:

  int verbosity;
  std::ostream &os;
  nostream nos;
};
