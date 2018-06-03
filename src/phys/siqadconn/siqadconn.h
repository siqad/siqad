// @file:     siqadconn.h
// @author:   Samuel
// @created:  2017.08.23
// @editted:  2018.06.02 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Convenient functions for interacting with SiQAD including
//            setting expected problem parameters, parsing problem files,
//            writing result files, etc. Use of the class is recommended, but
//            ultimately optional as devs may want to implement their own
//            I/O with SiQAD

#ifndef _POIS_SOLVER_PHYS_PHYS_CONNECTOR_H_
#define _POIS_SOLVER_PHYS_PHYS_CONNECTOR_H_


#include <stack>
#include <memory>
#include <map>
#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <string>
#include <vector>
#include <boost/circular_buffer.hpp>
#include <ctime>
#include <chrono>

namespace phys{
  namespace bpt = boost::property_tree;

  // Layer
  struct Layer;

  // Electrode related
  struct Electrode;
  class ElectrodeCollection;

  // DBDot related
  struct DBDot;
  class DBCollection;

  // Aggregate
  struct Aggregate;

  // Iterators
  class ElecIterator;
  class DBIterator;

  // ITERATOR
  typedef std::vector<std::shared_ptr<DBDot>>::const_iterator DBIter;
  typedef std::vector<std::shared_ptr<Electrode>>::const_iterator ElecIter;
  typedef std::vector<std::shared_ptr<Aggregate>>::const_iterator AggIter;

  // layer struct
  struct Layer {
    Layer(std::string name, std::string type, float zoffset, float zheight)
      : name(name), type(type), zoffset(zoffset), zheight(zheight) {};
    Layer() {};
    std::string name;   // layer name
    std::string type;   // layer type
    float zoffset=0;    // layer offset from lattice surface
    float zheight=0;    // layer thickness
  };

  // dangling bond
  struct DBDot {
    float x,y;  // physical location in angstroms
    int n,m,l;  // location in lattice coordinates
    DBDot(float in_x, float in_y, int n, int m, int l)
      : x(in_x), y(in_y), n(n), m(m), l(l) {};
  };

  // a constant iterator that iterates through all dangling bonds in the problem
  class DBIterator
  {
  public:
    explicit DBIterator(std::shared_ptr<Aggregate> root, bool begin=true);

    DBIterator& operator++(); // recursive part here
    bool operator==(const DBIterator &other) {return other.db_iter == db_iter;}
    bool operator!=(const DBIterator &other) {return other.db_iter != db_iter;}
    std::shared_ptr<DBDot> operator*() const {return *db_iter;}

    void setCollection(DBCollection *coll) {collection = coll;}
    DBCollection *collection; // needed for python wrapper
  private:

    DBIter db_iter;                   // points to the current DB
    std::shared_ptr<Aggregate> curr;  // current working Aggregate
    std::stack<std::pair<std::shared_ptr<Aggregate>, AggIter>> agg_stack;

    // add a new aggregate pair to the stack
    void push(std::shared_ptr<Aggregate> agg);

    // pop the aggregate stack
    void pop();
  };

  class DBCollection
  {
  public:
    DBCollection(std::shared_ptr<Aggregate> db_tree_in)
      : db_tree_inner(db_tree_in) {};
    DBIterator begin() {return DBIterator(db_tree_inner);}
    DBIterator end() {return DBIterator(db_tree_inner, false);}
    std::shared_ptr<Aggregate> db_tree_inner;
  };

  // electrode
  struct Electrode {
    int layer_id;
    double x1,x2,y1,y2;      // pixel location of electrode.
    double potential;  // voltage that the electrode is set to
    double phase;
    int electrode_type;
    double pixel_per_angstrom;
    Electrode(int in_layer_id, double in_x1, double in_x2, double in_y1, double in_y2, \
              double in_potential, double in_phase, int in_electrode_type, double in_pixel_per_angstrom)
      : layer_id(in_layer_id), x1(in_x1), x2(in_x2), y1(in_y1), y2(in_y2), \
        potential(in_potential), phase(in_phase), electrode_type(in_electrode_type), \
        pixel_per_angstrom(in_pixel_per_angstrom) {};
  };

  class ElecIterator
  {
  public:
    explicit ElecIterator(std::shared_ptr<Aggregate> root, bool begin=true);
    ElecIterator& operator++(); // recursive part here
    bool operator==(const ElecIterator &other) {return other.elec_iter == elec_iter;}
    bool operator!=(const ElecIterator &other) {return other.elec_iter != elec_iter;}
    std::shared_ptr<Electrode> operator*() const {return *elec_iter;}

    void setCollection(ElectrodeCollection *coll) {collection = coll;}
    ElectrodeCollection *collection; // needed for python wrapper
  private:
    ElecIter elec_iter;               // points to the current electrode
    std::shared_ptr<Aggregate> curr;  // current working Aggregate
    std::stack<std::pair<std::shared_ptr<Aggregate>, AggIter>> agg_stack;
    // add a new aggregate pair to the stack
    void push(std::shared_ptr<Aggregate> agg);
    // pop the aggregate stack
    void pop();
  };

  class ElectrodeCollection
  {
  public:
    ElectrodeCollection(std::shared_ptr<Aggregate> elec_tree_in)
      : elec_tree_inner(elec_tree_in) {};
    ElecIterator begin() {return ElecIterator(elec_tree_inner);}
    ElecIterator end() {return ElecIterator(elec_tree_inner, false);}
    std::shared_ptr<Aggregate> elec_tree_inner;
  };

  // aggregate
  struct Aggregate
  {
  public:
    std::vector<std::shared_ptr<Aggregate>> aggs;
    std::vector<std::shared_ptr<DBDot>> dbs;
    std::vector<std::shared_ptr<Electrode>> elecs;

    // Properties
    int size(); // returns the number of contained elecs, including those in children aggs
  };

  class SiQADConnector
  {
  public:
    // CONSTRUCTOR
    SiQADConnector(const std::string &eng_name_in, const std::string &input_path_in, const std::string &output_path_in);
    // DESTRUCTOR
    ~SiQADConnector(){writeResultsXml();}

    void writeResultsXml();

    // Initializers
    void initProblem();
    void initCollections();
    // File Handling
    void readProblem();

    // Accessors
    //Input flags
    void setExpectElectrode(bool set_val){expect_electrode = set_val;}
    void setExpectDB(bool set_val){expect_db = set_val;}
    void setExpectAFMPath(bool set_val){expect_afm_path = set_val;}

    //Set output flags
    void setExportElecPotential(bool set_val){export_elec_potential = set_val;}
    void setExportDBChargeConfig(bool set_val){export_db_charge_config = set_val;}
    void setExportElectrode(bool set_val){export_electrode = set_val;}
    void setExportDBLoc(bool set_val){export_db_loc = set_val;}
    void setExportDBPot(bool set_val){export_db_pot = set_val;}

    //generalosed setExport
    void setExport(std::string key, std::vector< std::pair< std::string, std::string > > &data_in);
    void setExport(std::string key, std::vector< std::vector< std::string > > &data_in);

    //set vector of strings as potential data
    void setElecPotentialData(std::vector<std::vector<std::string>> &data_in);
    //set vector of strings as electrode data
    void setElectrodeData(std::vector<std::vector<std::string>> &data_in);
    //set vector of strings as db data
    void setDBLocData(std::vector< std::pair< std::string, std::string > > &data_in);
    //set vector of strings as db data
    void setDBPotData(std::vector< std::vector< std::string > > &data_in);
    //set vector of strings as db data
    void setDBChargeData(std::vector<std::pair<std::string, std::string> > &data_in);

    //! Checks if a parameter exists given the parameter key.
    bool parameterExists(const std::string &key) {return sim_params.find(key) != sim_params.end();}

    //! Getter for a parameter, given a parameter key.
    std::string getParameter(const std::string &key) {return sim_params.find(key) != sim_params.end() ? sim_params.at(key) : "";}
    void setOutputPath(std::string path){output_path = path;}
    std::string getOutputPath(void){return output_path;}
    std::string getInputPath(void){return input_path;}

    //! Return pointer to DB collection, which allows iteration through DBs
    //! across all aggregate levels.
    DBCollection* dbCollection() {return db_col;}

    //! Return pointer to Electrode collection, which allows iteration through
    //! electrodes across all electrode layers.

    //simulation inputs and outputs
    std::vector<std::vector<std::string>> pot_data;
    std::vector<std::vector<std::string>> db_pot_data;
    std::vector<std::vector<std::string>> elec_data;
    std::vector<std::pair<std::string, std::string>> dbl_data;
    std::vector<std::pair<std::string, std::string>> db_charge_data;
    std::vector<std::pair<float,float>> db_locs;
    boost::circular_buffer<std::vector<int>> db_charges;



  private:

    // Read program properties
    void readProgramProp(const bpt::ptree &);

    // Read layer properties
    void readLayers(const bpt::ptree &);
    void readLayerProp(const bpt::ptree &);

    // Read simulation parameters
    void readSimulationParam(const bpt::ptree &);

    // Read design
    void readDesign(const bpt::ptree &subtree, const std::shared_ptr<Aggregate> &agg_parent);
    void readItemTree(const bpt::ptree &subtree, const std::shared_ptr<Aggregate> &agg_parent);
    void readElectrode(const bpt::ptree &subtree, const std::shared_ptr<Aggregate> &agg_parent);
    void readDBDot(const bpt::ptree &subtree, const std::shared_ptr<Aggregate> &agg_parent);

    // Iterable collections
    ElectrodeCollection* elec_col;
    DBCollection* db_col;

    // Retrieved items and properties
    std::map<std::string, std::string> program_props; // SiQAD properties
    std::shared_ptr<Aggregate> item_tree;             // all physical items
    std::vector<Layer> layers;                        // layers
    std::map<std::string, std::string> sim_params;    // simulation parameters

    // Engine properties
    std::string eng_name;                 // name of simulation engine
    std::string input_path;               // path to problem file
    std::string output_path;              // path to result export

    bool expect_electrode;
    bool expect_db;
    bool expect_afm_path;
    bool export_elec_potential;
    bool export_db_charge_config;
    bool export_electrode;
    bool export_db_loc;
    bool export_db_pot;
    int return_code;
    std::chrono::time_point<std::chrono::system_clock> start_time;
    std::chrono::time_point<std::chrono::system_clock> end_time;
  };

}//end namespace phys

#endif
