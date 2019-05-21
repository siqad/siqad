/** @file:     electron_config_set.cc
 *  @author:   Samuel
 *  @created:  2019.03.17
 *  @license:  GNU LGPL v3
 *
 *  @desc:     Stores electron configurations of DB layouts.
 */

#include "electron_config_set.h"

using namespace comp;

typedef comp::ElectronConfigSet ECS;

ECS::ElectronConfigSet(QXmlStreamReader *rs)
  : JobResult(ElectronConfigsResult)
{
  readFromXMLStream(rs);
}

void ECS::readFromXMLStream(QXmlStreamReader *rs)
{
  auto unrecognizedXMLElement = [](QXmlStreamReader &rs)
  {
    qWarning() << tr("Invalid element encountered on line %1 - %2")
      .arg(rs.lineNumber()).arg(rs.name().toString());
    rs.skipCurrentElement();
  };

  // read from stream
  QList<ElectronConfig> elec_configs_read;  // read electron configs
  while (rs->readNextStartElement()) {
    if (rs->name() == "dist") {
      ElectronConfig elec_config;

      for (QXmlStreamAttribute &attr : rs->attributes()) {
        if (attr.name().toString() == QLatin1String("energy")) {
          elec_config.energy = attr.value().toFloat();
        } else if (attr.name().toString() == QLatin1String("count")) {
          elec_config.config_occ = attr.value().toInt();
        } else if (attr.name().toString() == QLatin1String("physically_valid")) {
          elec_config.is_valid = attr.value().toInt();
        } else if (attr.name().toString() == QLatin1String("state_count")) {
          elec_config.state_count = attr.value().toInt();
        }
      }
      if (elec_config.state_count != 2 && elec_config.state_count != 3) {
        qCritical() << "Unrecognized state count " << elec_config.state_count;
        throw;
      }

      QString dist = rs->readElementText();

      // convert string distribution to array of int
      int neg_charge;
      for (QString charge_str : dist) {
        if (elec_config.state_count == 2) {
          // legacy format where 1=DB- and 0=DB0
          neg_charge = charge_str.toInt();
          if (neg_charge == 1) {
            elec_config.dbm_count++;
          }
        } else {
          // preferred new format
          if (charge_str == "+") {
            neg_charge = -1;
            elec_config.dbp_count++;
          } else if (charge_str == "0") {
            neg_charge = 0;
            elec_config.db0_count++;
          } else if (charge_str == "-") {
            neg_charge = 1;
            elec_config.dbm_count++;
          } else {
            qCritical() << "Unrecognized charge string " << charge_str;
            throw;
          }
        }
        elec_config.config.append(neg_charge);
      }

      elec_configs_read.append(elec_config);

      // stats bookkeeping
      elec_count_occ[elec_config.dbm_count] += elec_config.config_occ;
      total_config_count += elec_config.config_occ;
      //qDebug() << tr("Distribution: %1, Energy: %2, Count: %3").arg(dist).arg(read_dist.energy).arg(config_count);
    } else {
      unrecognizedXMLElement(*rs);
    }
  }

  // sort by energy
  std::sort(elec_configs_read.begin(), elec_configs_read.end(),
            [](const ElectronConfig &a, const ElectronConfig &b) -> bool
            {
              return a.energy > b.energy;
            });

  // insert sorted list to multimap so configs are "binned" by population
  for (ElectronConfig elec_config : elec_configs_read) {
    elec_configs.insert(elec_config.dbm_count, elec_config);
  }

  // TODO the above sorting solution is a short term one, in the future when 
  // there are more ways to visualize electron config sets (e.g. scatter plot
  // or histogram) the pre-processing have to be optimized accordingly.

  // TODO consider adding deduplication support to SiQADConn
}

QList<ECS::ElectronConfig> ECS::degenerateConfigs(const ECS::ElectronConfig &t_config) const
{
  QList<ECS::ElectronConfig> degen_configs;
  for (ECS::ElectronConfig config : elec_configs)
    if (config.energy == t_config.energy)
      degen_configs.append(config);
  return degen_configs;
}

int ECS::lowestPhysicallyValidInd(const QList<ElectronConfig> &elec_configs)
{
  for (int i=0; i<elec_configs.size(); i++) {
    if (elec_configs.at(i).is_valid == true) {
      return i;
    }
  }
  return -1;
}
