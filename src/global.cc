
#include "global.h"
#include "assert.h"

namespace gui{

double Unit::distanceUnitValue(DistanceUnit du) {
  switch (du) {
    case pm:
      return 1e-12;
      break;
    case ang:
      return 1e-10;
      break;
    case nm:
      return 1e-9;
      break;
    case um:
      return 1e-6;
      break;
    case mm:
      return 1e-3;
      break;
    case m:
      return 1;
      break;
    default:
      return -1;
      break;
  }
}

double Unit::valueConvertDistanceUnit(double val, DistanceUnit from_unit, DistanceUnit to_unit)
{
  double conv_scale = distanceUnitValue(from_unit) / distanceUnitValue(to_unit);
  assert(conv_scale > 0);
  return val * conv_scale;
}

QString Unit::distanceUnitString(DistanceUnit du)
{
  QMetaEnum du_enum = QMetaEnum::fromType<DistanceUnit>();
  return du_enum.valueToKey(du);
}

QStringList Unit::distanceUnitStringList(DistanceUnit start, DistanceUnit end)
{
  assert(end >= start);
  QMetaEnum du_enum = QMetaEnum::fromType<DistanceUnit>();
  QStringList du_str_list;
  for (int i=start; i<end; i++) {
    du_str_list.append(du_enum.valueToKey(i));
  }
  return du_str_list;
}

Unit::DistanceUnit Unit::stringToDistanceUnit(QString unit)
{
  QMetaEnum du_enum = QMetaEnum::fromType<DistanceUnit>();
  return static_cast<DistanceUnit>(du_enum.keyToValue(unit.toLatin1()));
}


} // end of gui namespace
