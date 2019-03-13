/** @file:     sqcommands.cc
 *  @author:   Samuel
 *  @created:  2019.03.26
 *  @license:  GNU LGPL v3
 *
 *  @desc:     A job result set storing SQCommands
 */

#include "sqcommands.h"

using namespace comp;

SQCommands::SQCommands(QXmlStreamReader *rs)
  : JobResult(SQCommandsResult)
{
  readFromXMLStream(rs);
}

void SQCommands::readFromXMLStream(QXmlStreamReader *rs)
{
  auto unrecognizedXMLElement = [](QXmlStreamReader &rs)
  {
    qWarning() << tr("Invalid element encountered on line %1 - %2")
      .arg(rs.lineNumber()).arg(rs.name().toString());
    rs.skipCurrentElement();
  };

  while (rs->readNextStartElement()) {
    if (rs->name() == "sqc") {
      sq_commands.append(rs->readElementText());
    } else {
      unrecognizedXMLElement(*rs);
    }
  }
}
