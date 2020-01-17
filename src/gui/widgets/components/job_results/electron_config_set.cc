/** @file:     electron_config_set.cc
 *  @author:   Samuel
 *  @created:  2019.03.17
 *  @license:  GNU LGPL v3
 *
 *  @desc:     Stores electron configurations of DB layouts.
 */

#include "electron_config_set.h"

using namespace comp;

typedef comp::ChargeConfigSet ECS;

ECS::ChargeConfigSet(QXmlStreamReader *rs)
  : JobResult(ChargeConfigsResult)
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
  QList<ChargeConfig> charge_configs_read;  // read charge configs
  while (rs->readNextStartElement()) {
    if (rs->name() == "dist") {
      ChargeConfig charge_config;

      for (QXmlStreamAttribute &attr : rs->attributes()) {
        if (attr.name().toString() == QLatin1String("energy")) {
          charge_config.energy = attr.value().toFloat();
        } else if (attr.name().toString() == QLatin1String("count")) {
          charge_config.config_occ = attr.value().toInt();
        } else if (attr.name().toString() == QLatin1String("physically_valid")) {
          charge_config.is_valid = attr.value().toInt();
        } else if (attr.name().toString() == QLatin1String("state_count")) {
          charge_config.state_count = attr.value().toInt();
        }
      }
      if (charge_config.state_count != 2 && charge_config.state_count != 3) {
        qCritical() << "Unrecognized state count " << charge_config.state_count;
        throw;
      }

      QString dist = rs->readElementText();

      // convert string distribution to array of int
      int neg_charge;
      for (QString charge_str : dist) {
        if (charge_config.state_count == 2) {
          // legacy format where 1=DB- and 0=DB0
          neg_charge = charge_str.toInt();
          if (neg_charge == 1) {
            charge_config.dbm_count++;
          }
        } else {
          // preferred new format
          if (charge_str == "+") {
            neg_charge = -1;
            charge_config.dbp_count++;
          } else if (charge_str == "0") {
            neg_charge = 0;
            charge_config.db0_count++;
          } else if (charge_str == "-") {
            neg_charge = 1;
            charge_config.dbm_count++;
          } else {
            qCritical() << "Unrecognized charge string " << charge_str;
            throw;
          }
        }
        charge_config.config.append(neg_charge);
      }

      charge_configs_read.append(charge_config);

      // stats bookkeeping
      net_charge_occ[charge_config.netNegCharge()] += charge_config.config_occ;
      total_config_count += charge_config.config_occ;
      //qDebug() << tr("Distribution: %1, Energy: %2, Count: %3").arg(dist).arg(read_dist.energy).arg(config_count);
    } else {
      unrecognizedXMLElement(*rs);
    }
  }

  // sort by energy
  std::sort(charge_configs_read.begin(), charge_configs_read.end(),
            [](const ChargeConfig &a, const ChargeConfig &b) -> bool
            {
              return a.energy > b.energy;
            });

  // insert sorted list to multimap so configs are "binned" by net charge
  for (ChargeConfig charge_config : charge_configs_read) {
    charge_configs.insert(charge_config.netNegCharge(), charge_config);
  }

  // TODO the above sorting solution is a short term one, in the future when 
  // there are more ways to visualize electron config sets (e.g. scatter plot
  // or histogram) the pre-processing have to be optimized accordingly.

  // TODO consider adding deduplication support to SiQADConn
}

QList<ECS::ChargeConfig> ECS::degenerateConfigs(const ECS::ChargeConfig &t_config) const
{
  QList<ECS::ChargeConfig> degen_configs;
  for (ECS::ChargeConfig config : charge_configs)
    if (config.energy == t_config.energy)
      degen_configs.append(config);
  return degen_configs;
}

int ECS::lowestPhysicallyValidInd(const QList<ChargeConfig> &charge_configs)
{
  for (int i=0; i<charge_configs.size(); i++) {
    if (charge_configs.at(i).is_valid == true) {
      return i;
    }
  }
  return -1;
}
