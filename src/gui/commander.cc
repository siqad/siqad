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

void gui::Commander::addKeyword(QString keyword)
{
  keyword.remove(" ");
  input_kws.append(keyword);
}

void gui::Commander::clearKeywords()
{
  input_kws.clear();
}

QStringList gui::Commander::cleanBrackets(QString* input)
{
  QString p_l = "(";
  QString p_r = ")";
  int count_l = input->count(p_l);
  int count_r = input->count(p_r);
  QStringList list = QStringList();
  if (count_l != count_r) {
    qDebug() << "Parentheses mismatch";
  } else {
    QString temp;
    int ind_l;
    int ind_r;
    for (int i = 0; i < count_l; i++) {
      ind_l = input->indexOf(p_l);
      ind_r = input->indexOf(p_r);
      temp.append(input->mid(ind_l, ind_r-ind_l+1));
      input->remove(ind_l, ind_r-ind_l+1);
    }
    list = cleanNumbers(&temp);
  }
  return list;
}

QStringList gui::Commander::cleanNumbers(QString* input)
{
  QRegExp rx("(\\d+)");
  QStringList list;
  int pos = 0;
  while ((pos = rx.indexIn(*input, pos)) != -1) {
    list << rx.cap(1);
    pos += rx.matchedLength();
  }
  return list;
}

QStringList gui::Commander::cleanAlphas(QString* input)
{
  QRegExp rx("([a-zA-Z]+)");
  QStringList list;
  int pos = 0;
  while ((pos = rx.indexIn(*input, pos)) != -1) {
    list << rx.cap(1);
    pos += rx.matchedLength();
  }
  return list;
}


void gui::Commander::parseInputs(QString input)
{
  QString input_orig = input;
  brackets = cleanBrackets(&input);
  qDebug() << brackets;
  numericals = cleanNumbers(&input);
  qDebug() << numericals;
  alphas = cleanAlphas(&input);
  qDebug() << alphas;

  if (!alphas.isEmpty()) {
    if (input_kws.contains(alphas.first())) {
      if (!performCommand()) {
        dialog_pan->echo(QObject::tr("Error occured with command '%1'").arg(input_orig));
      }
    } else {
      dialog_pan->echo(QObject::tr("Command '%1' not recognised.").arg(input_orig));
    }
  }
}

bool gui::Commander::performCommand()
{
  return false;
}

void gui::Commander::commandAddItem(QStringList args)
{
  if ((args.size() == 3) || (args.size() == 4)) {
    //item_type, layer_id, one set of arguments guaranteed present
    QString item_type = args.takeFirst().remove(" ");
    QString layer_id = args.takeFirst().remove(" ");
    QStringList item_args = args; //Whatever is left
    if (!design_pan->commandCreateItem(item_type, layer_id, item_args)) {
      dialog_pan->echo(QObject::tr("Item creation failed."));
    }
  } else {
    dialog_pan->echo(QObject::tr("add takes 3 or 4 arguments, %1 provided.").arg(args.size()));
  }
}


void gui::Commander::commandRemoveItem(QStringList args)
{
  if ((args.size() == 2) || (args.size() == 3)) {
    //item_type, one set of arguments guaranteed present
    QString item_type = args.takeFirst().remove(" ");
    // QStringList item_args = args;
    if (!design_pan->commandRemoveItem(item_type, args)) {
      dialog_pan->echo(QObject::tr("Item removal failed."));
    }
  } else {
    dialog_pan->echo(QObject::tr("remove takes 2 or 3 arguments, %1 provided.").arg(args.size()));
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
    if ((file.exists()) && (qfi.suffix()=="sqs")) { //check for extension
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

void gui::Commander::commandMoveItem(QStringList args)
{
  if ((args.size() == 3) || (args.size() == 4)) {
    //item_type, one set of arguments guaranteed present
    QString item_type = args.takeFirst().remove(" ");
    // QStringList item_args = args;
    if (!design_pan->commandMoveItem(item_type, args)) {
    //   dialog_pan->echo(QObject::tr("Item removal failed."));
      dialog_pan->echo(QObject::tr("Item move unsuccessful."));
    }
  } else {
    dialog_pan->echo(QObject::tr("move takes at least 3 arguments, %1 provided.").arg(args.size()));
  }
}
