/** @file:     electron_config_set.h
 *  @author:   Samuel
 *  @created:  2019.03.17
 *  @license:  GNU LGPL v3
 *
 *  @desc:     Stores electron configurations of DB layouts.
 */

#ifndef _COMP_ELEC_CONFIG_SET_H_
#define _COMP_ELEC_CONFIG_SET_H_

#include <QtWidgets>

#include "job_result.h"

namespace comp{

  //! Stores electron configurations of DB layouts.
  class ElectronConfigSet : public JobResult
  {
    Q_OBJECT

  public:


    //! Electron configurations (1 for electron, 0 for neutral)
    //! TODO in the future, add support for positive charge configurations.
    struct ElectronConfig
    {
      QList<int> config;
      float energy=0;       // energy of this configuration
      int elec_count=0;     // number of electrons in this config
      int config_occ=0;     // number of occurances of this config

      bool operator == (const ElectronConfig &other) const {
        if (config.length() != other.config.length()
            || energy != other.energy
            || elec_count != other.elec_count
            || config_occ != other.config_occ
            || config != other.config) {
          return false;
        }
        return true;
      }
    };

    //! Empty constructor.
    ElectronConfigSet() : JobResult(ElectronConfigsResult) {};

    //! Constructor taking a QXmlStreamReader to read the results directly. The 
    //! results are internally sorted in ascending order of electron count.
    ElectronConfigSet(QXmlStreamReader *rs);

    // TODO alternative constructor taking relevant information
    
    //! Destructor.
    ~ElectronConfigSet() {};

    //! Read electron config sets from XML stream.
    void readFromXMLStream(QXmlStreamReader *rs);

    //! Return whether this config set is empty.
    bool isEmpty() {return elec_configs.isEmpty();}

    //! Return the order of DB physical locations. TODO what unit does SiQADConn return?
    QList<QPointF> dbPhysicalLocations() {return phys_locs;}

    //! Set the order of DB physical locations.
    void setDBPhysicalLocations(const QList<QPointF> &t_phys_locs) {phys_locs = t_phys_locs;}

    //! Return a QMap mapping electron count to the number of occurances of 
    //! configurations with that count.
    QMap<int, int> electronCountOccurances() {return elec_count_occ;}

    //! Return the number of electron configurations (duplicates counted).
    int totalConfigCount() {return total_config_count;}

    //! Return the electron count which has the highest accumulated occurances.
    //! If there's a tie, the configuration with the lower electron count
    //! is returned.
    int mostPopularElectronCount()
    {
      QMap<int, int>::iterator max_it = elec_count_occ.begin();
      QMap<int, int>::iterator it;
      for (it = elec_count_occ.begin(); it != elec_count_occ.end(); it++)
        if (it.value() > max_it.value())
          max_it = it;
      return max_it.key();
    }

    //! Return all available electron counts.
    QList<int> electronCounts() const {return elec_configs.uniqueKeys();}

    //! Return all electron configurations.
    QList<ElectronConfig> electronConfigs() const {return elec_configs.values();}

    //! Return electron configurations with the specified electron count.
    QList<ElectronConfig> electronConfigs(const int &elec_count) const
    {
      return elec_configs.values(elec_count);
    }

    //! Return degenerate states of the given electron configuration.
    QList<ElectronConfig> degenerateConfigs(const ElectronConfig &config) const;

  private:

    //QList<ElectronConfig> elec_configs;   // electron configurations
    QList<QPointF> phys_locs;                     // physical location of DBs
    QMultiMap<int, ElectronConfig> elec_configs;  // electron configurations with electron count as key
    QMap<int, int> elec_count_occ;                // the accumulated occurances of each electron count
    int total_config_count=0;                     // total number of electron configurations (duplicates counted)
  };

} // end of comp namespace

#endif
