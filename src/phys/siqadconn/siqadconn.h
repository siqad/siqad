// @file:     siqadconn.h
// @author:   Samuel
// @created:  2017.08.23
// @editted:  2018.09.18 - Nathan
// @license:  Apache License 2.0
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
#include <ctime>
#include <chrono>

namespace phys{
  namespace bpt = boost::property_tree;

  // forward declaration
  struct Layer;
  struct DBDot;
  class DBIterator;
  class DBCollection;
  struct Electrode;
  class ElecIterator;
  class ElectrodeCollection;
  struct ElectrodePoly;
  class ElecPolyIterator;
  class ElectrodePolyCollection;
  struct Aggregate;

  typedef std::vector<std::shared_ptr<DBDot>>::const_iterator DBIter;
  typedef std::vector<std::shared_ptr<Electrode>>::const_iterator ElecIter;
  typedef std::vector<std::shared_ptr<ElectrodePoly>>::const_iterator ElecPolyIter;
  typedef std::vector<std::shared_ptr<Aggregate>>::const_iterator AggIter;

  // SiQAD connector class
  class SiQADConnector
  {
  public:
    // CONSTRUCTOR
    SiQADConnector(const std::string &eng_name, const std::string &input_path,
        const std::string &output_path);
    // DESTRUCTOR
    ~SiQADConnector(){writeResultsXml();}

    // Write results to the provided output_path
    void writeResultsXml();


    // EXPORTING

    // Set export type and variable
    void setExport(std::string type, std::vector< std::pair< std::string, std::string > > &data_in);
    void setExport(std::string type, std::vector< std::vector< std::string > > &data_in);


    // SIMULATION PARAMETERS

    // Checks if a parameter with the given key exists.
    bool parameterExists(const std::string &key) {return sim_params.find(key) != sim_params.end();}

    // Get the parameter with the given key.
    std::string getParameter(const std::string &key) {return sim_params.find(key) != sim_params.end() ? sim_params.at(key) : "";}

    std::vector<Layer> getLayers(void){return layers;}
    std::map<std::string, std::string> getAllParameters(void){return sim_params;}


    // ITERABLE COLLECTIONS

    // Return pointer to DB collection, which allows iteration through DBs
    // across all aggregate levels.
    DBCollection* dbCollection() {return db_col;}

    // Return pointer to Electrode collection, which allows iteration through
    // electrodes across all electrode layers.
    ElectrodeCollection* electrodeCollection() {return elec_col;}

    // Return pointer to Electrode collection, which allows iteration through
    // electrodes across all electrode layers.
    ElectrodePolyCollection* electrodePolyCollection() {return elec_poly_col;}


    // Misc Accessors
    std::string inputPath(){return input_path;}

    void setOutputPath(std::string path){output_path = path;}
    std::string outputPath(){return output_path;}

  private:

    // Read the problem file
    void readProblem(const std::string &path);

    // Read program properties
    void readProgramProp(const bpt::ptree &);

    // Read layer properties
    void readLayers(const bpt::ptree &);
    void readLayerProp(const bpt::ptree &);

    // Read simulation parameters
    void readSimulationParam(const bpt::ptree &);

    // Read design
    void readDesign(const bpt::ptree &, const std::shared_ptr<Aggregate> &);
    void readItemTree(const bpt::ptree &, const std::shared_ptr<Aggregate> &);
    void readElectrode(const bpt::ptree &, const std::shared_ptr<Aggregate> &);
    void readElectrodePoly(const bpt::ptree &, const std::shared_ptr<Aggregate> &);
    void readDBDot(const bpt::ptree &, const std::shared_ptr<Aggregate> &);

    // Generate property trees for writing
    bpt::ptree engInfoPropertyTree();
    bpt::ptree simParamsPropertyTree();
    bpt::ptree dbLocPropertyTree();
    bpt::ptree dbChargePropertyTree();
    bpt::ptree electrodePropertyTree();
    // bpt::ptree electrodePolyPropertyTree();
    bpt::ptree potentialPropertyTree();
    bpt::ptree dbPotentialPropertyTree(); // TODO fix up this function, a lot of redundant information

    // Engine properties
    std::string eng_name;                 // name of simulation engine
    std::string input_path;               // path to problem file
    std::string output_path;              // path to result export

    // Iterable collections
    ElectrodeCollection* elec_col;
    DBCollection* db_col;
    ElectrodePolyCollection* elec_poly_col;

    // Retrieved items and properties
    std::map<std::string, std::string> program_props; // SiQAD properties
    std::shared_ptr<Aggregate> item_tree;             // all physical items
    std::vector<Layer> layers;                        // layers
    std::map<std::string, std::string> sim_params;    // simulation parameters

    // Exportable data
    std::vector<std::vector<std::string>> pot_data;
    std::vector<std::vector<std::string>> db_pot_data;
    std::vector<std::vector<std::string>> elec_data;
    std::vector<std::pair<std::string, std::string>> dbl_data;        // pair of location x and y
    std::vector<std::vector<std::string>> db_charge_data;  // pair of elec dist and energy

    // Runtime information
    int return_code=0;
    std::chrono::time_point<std::chrono::system_clock> start_time;
    std::chrono::time_point<std::chrono::system_clock> end_time;
  };


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


  // electrode_poly
  struct ElectrodePoly {
    int layer_id;
    std::vector<std::pair<double, double>> vertices;  // vertex points
    double potential;
    double phase;
    int electrode_type;
    int net;
    double pixel_per_angstrom;
    ElectrodePoly(int in_layer_id, std::vector<std::pair<double, double>> in_vertices, \
              double in_potential, double in_phase, int in_electrode_type, double in_pixel_per_angstrom, int in_net)
      : layer_id(in_layer_id), vertices(in_vertices), \
        potential(in_potential), phase(in_phase), electrode_type(in_electrode_type), \
        pixel_per_angstrom(in_pixel_per_angstrom), net(in_net) {};
  };

  class ElecPolyIterator
  {
  public:
    explicit ElecPolyIterator(std::shared_ptr<Aggregate> root, bool begin=true);
    ElecPolyIterator& operator++(); // recursive part here
    bool operator==(const ElecPolyIterator &other) {return other.elec_poly_iter == elec_poly_iter;}
    bool operator!=(const ElecPolyIterator &other) {return other.elec_poly_iter != elec_poly_iter;}
    std::shared_ptr<ElectrodePoly> operator*() const {return *elec_poly_iter;}

    void setCollection(ElectrodePolyCollection *coll) {collection = coll;}
    ElectrodePolyCollection *collection; // needed for python wrapper
  private:
    ElecPolyIter elec_poly_iter;               // points to the current electrode
    std::shared_ptr<Aggregate> curr;  // current working Aggregate
    std::stack<std::pair<std::shared_ptr<Aggregate>, AggIter>> agg_stack;
    // add a new aggregate pair to the stack
    void push(std::shared_ptr<Aggregate> agg);
    // pop the aggregate stack
    void pop();
  };

  class ElectrodePolyCollection
  {
  public:
    ElectrodePolyCollection(std::shared_ptr<Aggregate> elec_poly_tree_in)
      : elec_poly_tree_inner(elec_poly_tree_in) {};
    ElecPolyIterator begin() {return ElecPolyIterator(elec_poly_tree_inner);}
    ElecPolyIterator end() {return ElecPolyIterator(elec_poly_tree_inner, false);}
    std::shared_ptr<Aggregate> elec_poly_tree_inner;
  };

  // electrode
  struct Electrode {
    int layer_id;
    double x1,x2,y1,y2;      // pixel location of electrode.
    double potential;  // voltage that the electrode is set to
    double phase;
    int electrode_type;
    int net;
    double pixel_per_angstrom;
    Electrode(int in_layer_id, double in_x1, double in_x2, double in_y1, double in_y2, \
              double in_potential, double in_phase, int in_electrode_type, double in_pixel_per_angstrom, int in_net)
      : layer_id(in_layer_id), x1(in_x1), x2(in_x2), y1(in_y1), y2(in_y2), \
        potential(in_potential), phase(in_phase), electrode_type(in_electrode_type), \
        pixel_per_angstrom(in_pixel_per_angstrom), net(in_net) {};
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
    std::vector<std::shared_ptr<ElectrodePoly>> elec_polys;

    // Properties
    int size(); // returns the number of contained elecs, including those in children aggs
  };


}//end namespace phys

#endif
