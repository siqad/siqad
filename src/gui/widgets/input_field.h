// @file:     input_field.h
// @author:   Jake
// @created:  2017.05.02
// @editted:  2017.05.11  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Input line for taking customized commands.

#ifndef _GUI_INPUT_FIELD_H
#define _GUI_INPUT_FIELD_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <QDebug>
#include <QKeyEvent>

#include <QRegExpValidator>
#include <QLineEdit>


// NOTE:
//  Under NO circumstances should the user be allowed to execute arbitrary
//  commands. If a future implementation includes a serial port to allow for
//  more general terminal-like functionality, ALL commands must be white-listed
//  by the Validator. If you don't know what that means, don't implement/modify
//  a serial port.


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

protected:
  virtual void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;

private:
  Validator *validator;
  QStringList *cmd_history;
  int max_history;
  int position;
};

} // end gui namespace


#endif
