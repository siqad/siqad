// @file:     commander.cc
// @author:   Nathan
// @created:  2018.06.26
// @editted:  2018.06.26  - Nathan
// @license:  GNU LGPL v3
//
// @desc:     Commander definitions. Handles the commands grabbed by the dialog_panel.

#include "commander.h"
#include "settings/settings.h"
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
    dialog_pan->echo("Parentheses mismatch");
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
  // Captures ONLY numerical sequences not immediate
  // preceded or succeeded by letters.
  QRegExp rx("[^a-zA-Z0-9./_](-?[\\d.]+)(?![a-zA-Z])(?![0-9])");
  QStringList list;
  int pos = 0;
  while ((pos = rx.indexIn(*input, pos)) != -1) {
    list << rx.cap(1);
    pos += rx.matchedLength();
  }
  for (QString item : list) {
    input->remove(item);
  }
  return list;
}

QStringList gui::Commander::cleanAlphas(QString* input)
{
  QRegExp rx("([a-zA-Z0-9./_]+)");
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
  input_orig = input;
  brackets = cleanBrackets(&input);
  numericals = cleanNumbers(&input);
  alphas = cleanAlphas(&input);
  if (!alphas.isEmpty()) {
    if (input_kws.contains(alphas.first())) {
      if (!performCommand())
        dialog_pan->echo(QObject::tr("Error occured with command '%1'").arg(input_orig));
    } else {
      dialog_pan->echo(QObject::tr("Command '%1' not recognised.").arg(input_orig));
    }
  }
}

bool gui::Commander::performCommand()
{
  QString command = alphas.takeFirst().remove(" ");
  if (command == QObject::tr("add"))
    return commandAddItem();
  else if (command == QObject::tr("remove"))
    return commandRemoveItem();
  else if (command == QObject::tr("echo"))
    return commandEcho();
  else if (command == QObject::tr("help"))
    return commandHelp();
  else if (command == QObject::tr("run"))
    return commandRun();
  else if (command == QObject::tr("move"))
    return commandMoveItem();
  return false;
}

bool gui::Commander::commandAddItem()
{
  if (!alphas.isEmpty()) {
    QString item_type = alphas.takeFirst();
    QString layer_id;
    if (!alphas.isEmpty())
      layer_id = alphas.takeFirst();
    else if (!numericals.isEmpty())
      layer_id = numericals.takeFirst();
    else {
      dialog_pan->echo(QObject::tr("Missing argument, quitting commandAddItem()"));
      return false;
    }
    if (!design_pan->commandCreateItem(item_type, layer_id, brackets)) {
      dialog_pan->echo(QObject::tr("Item creation failed."));
      return false;
    }
  } else {
    dialog_pan->echo(QObject::tr("Item type not specified"));
    return false;
  }
  return true;
}

bool gui::Commander::commandRemoveItem()
{
  if (!alphas.isEmpty()) {
    QString item_type = alphas.takeFirst();
    if (!design_pan->commandRemoveItem(item_type, brackets, numericals)) {
      dialog_pan->echo(QObject::tr("Item removal failed."));
      return false;
    }
  } else {
    dialog_pan->echo(QObject::tr("Item type not specified"));
    return false;
  }
  return true;
}

bool gui::Commander::commandMoveItem()
{
  if (!alphas.isEmpty()) {
    QString item_type = alphas.takeFirst();
    if (!design_pan->commandMoveItem(item_type, brackets, numericals)) {
      dialog_pan->echo(QObject::tr("Item move unsuccessful."));
      return false;
    }
  } else {
    dialog_pan->echo(QObject::tr("Item type not specified"));
    return false;
  }
  return true;
}

bool gui::Commander::commandEcho()
{
  QString input_copy = input_orig;
  QString target = QString("echo ");
  input_copy.remove(input_copy.indexOf(target), target.size());
  dialog_pan->echo(input_copy);
  return true;
}

bool gui::Commander::commandHelp()
{
  QFile file(":/help_text.xml");
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    qFatal(QObject::tr("Error help text file to read: %1")
        .arg(file.errorString()).toLatin1().constData(), 0);
    return false;
  }
  QString command, description, usage;
  QXmlStreamReader rs(&file);
  rs.readNext();
  while (!rs.atEnd()) {
    if (rs.isStartElement()) {
      if (rs.name().toString() == "command")
        command = rs.readElementText();
      else if (rs.name().toString() == "text")
        description = rs.readElementText();
      else if (rs.name().toString() == "usage")
        usage = rs.readElementText();
    } else if (rs.isEndElement()) {
      if (rs.name().toString() == "entry") {
        if ((alphas.isEmpty()) || alphas.contains(command)) {
          dialog_pan->echo(QString("\nCommand:  ")+command);
          dialog_pan->echo(QString("  Description:  ")+description);
          dialog_pan->echo(QString("  Usage:  ")+usage);
        }
      }
    }
    rs.readNext();
  }
  file.close();
  return true;
}

bool gui::Commander::commandRun()
{
  for (QString path: alphas) {
    QFile file(path);
    QFileInfo qfi(file);
    if ((file.exists()) && (qfi.suffix()=="sqs")) { //check for extension
      dialog_pan->echo(QObject::tr("Running file %1.").arg(file.fileName()));
      if (file.open(QIODevice::ReadOnly)) {
         QTextStream in(&file);
         while (!in.atEnd()) {
            QString line = in.readLine();
            if (!line.isEmpty())
              parseInputs(line);
         }
         file.close();
      }
    } else {
      dialog_pan->echo(QObject::tr("Error opening '%1'. Check that file exists and that the extension is '.sqs'.").arg(file.fileName()));
    }
  }
  return true;
}
