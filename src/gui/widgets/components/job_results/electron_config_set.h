/** @file:     electron_config_set.h
 *  @author:   Samuel
 *  @created:  2019.03.17
 *  @license:  GNU LGPL v3
 *
 *  @desc:     Stores charge configurations of DB layouts.
 */

#ifndef _COMP_CHRG_CONFIG_SET_H_
#define _COMP_CHRG_CONFIG_SET_H_

#include <QtWidgets>

#include "job_result.h"

namespace comp{

  //! Stores charge configurations of DB layouts.
  class ChargeConfigSet : public JobResult
  {
    Q_OBJECT

  public:


    //! Charge configurations (-1 for DB-, 0 for DB0, +1 for DB+)
    struct ChargeConfig
    {
      QList<int> config;
      float energy=0;       // energy of this configuration
      int dbm_count=0;      // number of DB- sites in this config
      int db0_count=0;      // number of DB0 sites in this config
      int dbp_count=0;      // number of DB+ sites in this config
      int is_valid=-1;      // is physically valid, -1 for unknown (not provided)
      int state_count=2;    // number of supported states, if 2 then 0=DB0 and 1=DB-; if 3 then {+,0,-} = {DB+, DB0, DB0}
      int config_occ=0;     // number of occurances of this config

      int netNegCharge() {return dbm_count - dbp_count;}

      bool operator == (const ChargeConfig &other) const {
        if (config.length() != other.config.length()
            || energy != other.energy
            || config_occ != other.config_occ
            || config != other.config) {
          return false;
        }
        return true;
      }
    };

    //! Empty constructor.
    ChargeConfigSet() : JobResult(ChargeConfigsResult) {};

    //! Constructor taking a QXmlStreamReader to read the results directly. The 
    //! results are internally sorted in ascending order of charge count.
    ChargeConfigSet(QXmlStreamReader *rs);

    // TODO alternative constructor taking relevant information
    
    //! Destructor.
    ~ChargeConfigSet() {};

    //! Read charge config sets from XML stream.
    void readFromXMLStream(QXmlStreamReader *rs);

    //! Return whether this config set is empty.
    bool isEmpty() {return charge_configs.isEmpty();}

    //! Return the order of DB physical locations. TODO what unit does SiQADConn return?
    QList<QPointF> dbPhysicalLocations() {return phys_locs;}

    //! Set the order of DB physical locations.
    void setDBPhysicalLocations(const QList<QPointF> &t_phys_locs) {phys_locs = t_phys_locs;}

    //! Return a QMap mapping net charge to the number of occurances of 
    //! configurations with that net charge.
    QMap<int, int> netChargeOccurances() {return net_charge_occ;}

    //! Return the number of charge configurations (duplicates counted).
    int totalConfigCount() {return total_config_count;}

    //! Return the net charge which has the highest accumulated occurances.
    //! If there's a tie, the configuration with the lower net charge
    //! is returned.
    int mostPopularNetCharge()
    {
      QMap<int, int>::iterator max_it = net_charge_occ.begin();
      QMap<int, int>::iterator it;
      for (it = net_charge_occ.begin(); it != net_charge_occ.end(); it++)
        if (it.value() > max_it.value())
          max_it = it;
      return max_it.key();
    }

    //! Return all available net charges.
    QList<int> netCharges() const {return charge_configs.uniqueKeys();}

    //! Return charge configurations with the specified net charge.
    //! If all_configs is set to true, charge_count is ignored and all configs
    //! are returned. Otherwise, only configs with the specified charge_count
    //! are returned.
    QList<ChargeConfig> chargeConfigs(bool phys_valid_filter=false,
                                      bool all_configs=true,
                                      const int &net_charge=-1) const
    {
      QList<ChargeConfig> configs = all_configs ? charge_configs.values() : charge_configs.values(net_charge);
      if (configs.size() > 0 && phys_valid_filter)
        physicallyValidFilter(configs);
      return configs;
    }

    //! Filter out physically invalid states in the provided list reference.
    void physicallyValidFilter(QList<ChargeConfig> &configs) const
    {
      QList<ChargeConfig>::iterator it = configs.begin();
      while (it != configs.end()) {
        if ((*it).is_valid != 1)
          it = configs.erase(it);
        else
          ++it;
      }
    }

    //! Return degenerate states of the given charge configuration including
    //! the given config.
    QList<ChargeConfig> degenerateConfigs(const ChargeConfig &config) const;

    //! Return the index to the lowest energy state which is physically valid 
    //! in the given list of charge configs. If there is no physically valid
    //! index, return -1.
    static int lowestPhysicallyValidInd(const QList<ChargeConfig> &charge_configs);

  private:

    //QList<ChargeConfig> charge_configs;   // charge configurations
    QList<QPointF> phys_locs;                     // physical location of DBs
    QMultiMap<int, ChargeConfig> charge_configs;  // charge configurations with net charge as key
    QMap<int, int> net_charge_occ;                // the accumulated occurances of each net charge
    int total_config_count=0;                     // total number of charge configurations (duplicates counted)
  };

} // end of comp namespace

#endif
