// @file:     input_field.cc
// @author:   Jake
// @created:  2017.05.02
// @editted:  2017.05.11  - Jake
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
}


gui::InputField::~InputField()
{
  delete validator;
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

void gui::InputField::keyPressEvent(QKeyEvent *e)
{
  if (e->key() == Qt::Key_Up) {
    if (position > 0) {
      position--;
      this->setText(cmd_history->at(position));
    } else {
      this->setText(QString());
    }
  } else if (e->key() == Qt::Key_Down) {
    if (position < cmd_history->size() - 1) {
      position++;
      this->setText(cmd_history->at(position));
    } else {
      this->setText(QString());
    }
  } else {
    QLineEdit::keyPressEvent(e);
  }
}
