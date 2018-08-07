// @file:     input_field.cc
// @author:   Jake
// @created:  2017.05.02
// @editted:  2018.08.07  - Nathan
// @license:  GNU LGPL v3
//
// @desc:     InputField definitions

#include "input_field.h"

// VALIDATOR CLASS

// constructor
gui::Validator::Validator(QObject *parent)
  : QRegExpValidator(parent)
{

}

// destructor
gui::Validator::~Validator()
{}

// INPUTFIELD CLASS

gui::InputField::InputField(QWidget *parent)
  : QLineEdit(parent)
{
  this->validator = new gui::Validator();
  this->cmd_history = new QStringList();
  max_history = 100;
  position = 0;
  installEventFilter(this);
  initCompleters();
  completer = cmd_comp;
  setCompleter(completer);

}

void gui::InputField::initCompleters()
{
  //dir_comp takes care of the filesystem completion
  dir_comp = new Completer();
  fsm = new QFileSystemModel(dir_comp);
  fsm->setRootPath("");
  dir_comp->setModel(fsm);
  //cmd_comp takes care of the command completion
  cmd_comp = new Completer(commandStringList());
  //item_comp takes care of item type completion
  item_comp = new Completer(itemTypeList());
}

QStringList gui::InputField::itemTypeList()
{
  QStringList list;
  for (int i = 0; i != int(prim::Item::LastItemType); ++i)
  {
    QString item = prim::Item::getQStringItemType(prim::Item::ItemType(i));
    if (!list.contains(item))
      list.append(item);
  }
  return list;
}

QStringList gui::InputField::commandStringList()
{
  QFile file(":/help_text.xml");
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    qFatal(QObject::tr("Error help text file to read: %1")
        .arg(file.errorString()).toLatin1().constData(), 0);
    return QStringList();
  }
  QStringList list;
  QXmlStreamReader rs(&file);
  rs.readNext();
  while (!rs.atEnd()) {
    if (rs.isStartElement()) {
      if (rs.name().toString() == "command")
        list.append(rs.readElementText());
    }
    rs.readNext();
  }
  file.close();
  return list;
}

gui::InputField::~InputField()
{
  delete validator;
  delete cmd_history;
  delete fsm;
  delete dir_comp;
  delete cmd_comp;
  delete item_comp;
}


QString gui::InputField::pop()
{
  QString input = this->text().trimmed();
  this->clear();
  cmd_history->append(input);
  if (cmd_history->size() > max_history) {
    cmd_history->pop_front();
  }
  position = cmd_history->size();
  return input;
}

void gui::InputField::insertCompletion(QString completion)
{
  int extra = completion.length() - completer->completionPrefix().length();
  insert(completion.right(extra));
}

void gui::InputField::manageCompleters()
{
  QStringList words = getWords();
  if (words.count() <= 1) {
    completer = cmd_comp;
  } else {
    if (words.first() == QString("run")) {
      completer = dir_comp;
    } else if ( words.first() == QString("add") ||
                words.first() == QString("remove") ||
                words.first() == QString("move")) {
      completer = item_comp;
    }
  }
}

QStringList gui::InputField::getSuggestions()
{
  QStringList words = getWords();
  if (words.count() <= 1) {
    //this must be the command.
    return commandStringList();
  } else {
    //check which completer is set
    if (completer == dir_comp) {
      //run expects a path
      QDir dir(words.last());
      dir.setSorting(QDir::DirsFirst);
      if (dir.entryList().isEmpty()) {
        QString dir_string = words.last();
        dir_string.truncate(dir_string.lastIndexOf(QDir::separator())+1);
        dir.setPath(dir_string);
        QString filter = words.last();
        filter = filter.right(filter.size()-filter.lastIndexOf(QDir::separator())-1);
        filter.append("*");
        dir.setNameFilters(QStringList(filter));
      }
      return dir.entryList();
    } else if (completer == item_comp) {
      QString pattern = QString("^") + words.last();
      QRegExp rx(pattern);
      return itemTypeList().filter(rx);
    }
  }
  return QStringList();
}

QStringList gui::InputField::getWords()
{
  return text().split(QRegExp("(\\s|\\n|\\r)+"), QString::SkipEmptyParts);
}

bool gui::InputField::eventFilter(QObject *obj, QEvent *event)
{
  (void)obj; //so compiler doesn't complain about unused object.
  if (event->type() == QEvent::KeyPress) {
    QStringList candidates = getWords();
    QString word;
    if (candidates.isEmpty())
      word = text();
    else
      word = candidates.last();
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->key() == Qt::Key_Tab) {
      manageCompleters();
      completer->setCompletionPrefix(word);
      if (completer->completionCount() == 1) {
        deselect();
        insertCompletion(completer->currentCompletion());
        if (completer == dir_comp) {
          if (fsm->isDir(fsm->index(getWords().last())))
            insertCompletion(QDir::separator());
        }
        completer->setCompletionPrefix(getWords().last());
        end(false); //move cursor to end
        return true;
      } else {
        qDebug() << "suggestions:" << getSuggestions();
      }
    }
  }
  return false;
}

void gui::InputField::keyPressEvent(QKeyEvent *e)
{
  if (e->key() == Qt::Key_Up) {
    if (position > 0) {
      if (position == cmd_history->size())
        current_cmd = this->text();
      position--;
      this->setText(cmd_history->at(position));
    }
  } else if (e->key() == Qt::Key_Down) {
    if (position < cmd_history->size()) {
      position++;
      if (position == cmd_history->size())
        this->setText(current_cmd);
      else
        this->setText(cmd_history->at(position));
    }
  } else {
    QLineEdit::keyPressEvent(e);
  }
}
