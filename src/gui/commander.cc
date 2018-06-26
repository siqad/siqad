// @file:     commander.cc
// @author:   Nathan
// @created:  2018.06.26
// @editted:  2018.06.26  - Nathan
// @license:  GNU LGPL v3
//
// @desc:     Commander definitions. Handles the commands grabbed by the dialog_panel.

#include "commander.h"
#include "src/settings/settings.h"
#include <iostream>

gui::Commander::Commander()
{
  clearKeywords();
}

void gui::Commander::doStuff()
{
  dialog_pan->echo(QString("Commander says hello"));
}

void gui::Commander::addKeyword(QString keyword)
{
  keyword.remove(" ");
  input_kws.append(keyword);
}

void gui::Commander::clearKeywords()
{
  input_kws.clear();
}

void gui::Commander::parseInputs(QString input)
{
  dialog_pan->echo(input);
  QStringList inputs = input.split(",", QString::SkipEmptyParts);
  if (input_kws.contains(inputs.first())) {
    // keyword detected
    if (!performCommand(inputs)) {
      dialog_pan->echo(QObject::tr("Error occured with command '%1'").arg(inputs.first()));
    }
  } else {
    dialog_pan->echo(QObject::tr("Command '%1' not recognised.").arg(inputs.first()));
  }
}

bool gui::Commander::performCommand(QStringList cmds)
{
  if (!cmds.isEmpty()){
    QString command = cmds.takeFirst().remove(" ");
    if (command == QObject::tr("add_item")) {
      commandAddItem(cmds);
    } else if (command == QObject::tr("remove_item")) {
      commandRemoveItem(cmds);
    } else if (command == QObject::tr("echo")) {
      commandEcho(cmds);
    } else if (command == QObject::tr("help")) {
      commandHelp(cmds);
    } else if (command == QObject::tr("run")) {
      commandRun(cmds);
    } else {
      return false;
    }
    return true;
  } else {
    return false;
  }
}

void gui::Commander::commandAddItem(QStringList args)
{
  if (args.size() >= 3) {
    //item_type, layer_id, one set of arguments guaranteed present
    QString item_type = args.takeFirst().remove(" ");
    QString layer_id = args.takeFirst().remove(" ");
    QStringList item_args = args;
    if (!design_pan->commandCreateItem(item_type, layer_id, item_args)) {
      dialog_pan->echo(QObject::tr("Item creation failed."));
    }
  } else {
    dialog_pan->echo(QObject::tr("add_item takes at least 3 arguments, %1 provided.").arg(args.size()));
  }
}


void gui::Commander::commandRemoveItem(QStringList args)
{
  if (args.size() >= 2) {
    //item_type, one set of arguments guaranteed present
    QString item_type = args.takeFirst().remove(" ");
    QStringList item_args = args;
    if (!design_pan->commandRemoveItem(item_type, args)) {
      dialog_pan->echo(QObject::tr("Item removal failed."));
    }
  } else {
    dialog_pan->echo(QObject::tr("remove_item takes at least 2 arguments, %1 provided.").arg(args.size()));
  }
}


void gui::Commander::commandEcho(QStringList args)
{
  for (QString arg: args) {
    dialog_pan->echo(arg);
  }
}


void gui::Commander::commandHelp(QStringList args)
{
  QFile file(":/help_text.xml");
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    qFatal(QObject::tr("Error help text file to read: %1")
        .arg(file.errorString()).toLatin1().constData(), 0);
    return;
  }
  QStringList clean_args = QStringList();
  for (QString arg: args) {
    clean_args.append(arg.remove(" "));
  }
  QString command, description, usage;
  QXmlStreamReader rs(&file);
  rs.readNext();
  while (!rs.atEnd()) {
    if (rs.isStartElement()) {
      if (rs.name().toString() == "command") {
        command = rs.readElementText();
      } else if (rs.name().toString() == "text") {
        description = rs.readElementText();
      } else if (rs.name().toString() == "usage") {
        usage = rs.readElementText();
      }
    } else if (rs.isEndElement()) {
      if (rs.name().toString() == "entry") {
        if ((clean_args.isEmpty()) || clean_args.contains(command)) {
          dialog_pan->echo(QString("\nCommand:  ")+command);
          dialog_pan->echo(QString("  Description:  ")+description);
          dialog_pan->echo(QString("  Usage:  ")+usage);
        }
      }
    }
    rs.readNext();
  }
  file.close();
}

void gui::Commander::commandRun(QStringList args)
{
  for (QString arg: args) {
    arg.remove(" ");
    QFile file(arg);
    QFileInfo qfi(file);
    if ((file.exists()) && (qfi.suffix()=="sqs")) {
      dialog_pan->echo(QObject::tr("Running file %1.").arg(file.fileName()));
      if (file.open(QIODevice::ReadOnly))
      {
         QTextStream in(&file);
         while (!in.atEnd())
         {
            QString line = in.readLine();
            if (!line.isEmpty()){
              parseInputs(line);
            }
         }
         file.close();
      }
    } else {
      dialog_pan->echo(QObject::tr("Error opening '%1'. Check that file exists and that the extension is '.sqs'.").arg(file.fileName()));
    }
  }
}
