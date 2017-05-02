// @file:     input_field.h
// @author:   Jake
// @created:  2017.05.02
// @editted:  2017.05.02  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Input line for taking customized commands, inputs must pass a mask
//            to avoid unwanted commands

#ifndef _GUI_INPUT_FIELD_H
#define _GUI_INPUT_FIELD_H

#include <QObject>
#include <QWidget>
#include <QString>

#include <QRegExpValidator>
#include <QLineEdit>



namespace gui{

// customized validator for masking the input
class Validator : public QRegExpValidator
{
  Q_OBJECT

public:

  // constructor
  Validator(QObject *parent=0);

  // destructor
  ~Validator();

};


// customized QLineEdit object for handling command inputs
class InputField : public QLineEdit
{
  Q_OBJECT

public:

  // constructor
  InputField(QWidget *parent=0);

  // destructor
  ~InputField();

  // get the text and clear
  QString pop();

private:
  Validator *validator;

};

} // end gui namespace


#endif
