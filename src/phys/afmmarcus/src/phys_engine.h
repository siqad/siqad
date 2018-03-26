// @file:     phys_engine.h
// @author:   Samuel
// @created:  2017.08.23
// @editted:  2017.08.23 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Base class for physics engines

#include "problem.h"

#include <string>
#include <vector>
#include <boost/circular_buffer.hpp>

namespace phys{

  namespace bpt = boost::property_tree;

  class PhysicsEngine
  {
  public:

    // constructor
    PhysicsEngine(const std::string &eng_nm, const std::string &i_path, const std::string &o_path, const std::string &script_path="", const std::string &temp_path="");

    // destructor
    ~PhysicsEngine() {};

    // read DB locations from problem and store them in db_locs vector
    void readDBLocFromProblem();

    // export results
    void writeResultsXml();
    bpt::ptree xmlNodeEngInfo();
    bpt::ptree xmlNodeSimParams();
    bpt::ptree xmlNodePhysloc();
    bpt::ptree xmlNodeElecDist();
    bpt::ptree xmlNodeLineScans();

    // TODO function that checks whether tempPath is writable

    // Boost local time with specified format
    std::string formattedTime(const std::string &time_format=std::string("%Y%m%d-%H%M%S")) const;
    
    // ACCESSORS
    std::string name() {return eng_name;}
    std::string scriptPath() {return script_path;}
    std::string inPath() {return in_path;}
    std::string outPath() {return out_path;}
    std::string tempPath() {return temp_path;}

    // variables
    Problem problem;

    // structs to store exportable variables
    struct LineScanPath {
      int path_id;  // the scan_id in case there were multiple scans
      // TODO offset of db_locs from physloc
      std::vector<std::pair<float,float>> db_locs_enc;   // DB locations encountered in angstrom
      std::vector<std::string> results;   // results of line scans
    };

    // exportable variables/lists
    std::vector<std::pair<float,float>> db_locs; // location of free dbs
    std::vector<std::tuple<float,float,float>> fixed_charges; // location of fixed charges
    boost::circular_buffer<std::vector<int>> db_charges;
    std::vector<LineScanPath> line_scan_paths;

    // handy functions
    std::string float2StringPrecision(float number, int decimal_places);

  private:
    std::string eng_name;     // name of this physics engine
    std::string script_path;  // for further invoking Python scripts
    std::string in_path;      // input file path containing the problem to run
    std::string out_path;     // output file path containing the simulation result
    std::string temp_path;    // path where temporary files will be generated

    int n_dbs=-1; // number of dbs TODO move to problem?

  };

}
