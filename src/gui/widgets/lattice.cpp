#include "lattice.h"


gui::Lattice::Lattice()
{
  settings::LatticeSettings lattice_settings;
  construct(lattice_settings);
}

gui::Lattice::Lattice(QString fname)
{
  settings::LatticeSettings lattice_settings(fname);
  construct(lattice_settings);
}

void gui::Lattice::construct(settings::LatticeSettings &lattice_settings)
{
  settings::GUISettings gui_settings;

  // determine dimension of lattice canvas

}

gui::Lattice::~Lattice()
{}
