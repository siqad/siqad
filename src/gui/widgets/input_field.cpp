// @file:     input_field.cpp
// @author:   Jake
// @created:  2017.05.02
// @editted:  2017.05.02  - Jake
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
}


gui::InputField::~InputField()
{}


QString gui::InputField::pop()
{
  QString input = this->text().trimmed();
  this->clear();

  return input;
}
