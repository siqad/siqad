%module siqadconn
%include <std_pair.i>
%include <std_vector.i>
%include <std_string.i>
%include <std_map.i>
%include <exception.i>

%{
#include "siqadconn.h"
%}

namespace std {
    %template(IntVector) vector<int>;
    %template(StringPair) pair<string, string>;
    %template(StringPairVector) vector< pair<string, string> >;
    %template(StringVector) vector<string>;
    %template(StringVector2D) vector< vector<string> >;
    %template(StringMap) map< string, string >;
}

%{
#define SWIG_FILE_WITH_INIT
#define SWIG_PYTHON_2_UNICODE
%}

%include "siqadconn.h"

%feature("shadow") phys::DBIterator::__next__() %{
  def __next__(self):
    db = $action(self)
    if db == None:
      raise StopIteration
    else:
      return db
%}

%feature("shadow") phys::ElecIterator::__next__() %{
  def __next__(self):
    elec = $action(self)
    if elec == None:
      raise StopIteration
    else:
      return elec
%}

%extend phys::SiQADConnector {
  %pythoncode{
    def setExport(self, *args, **kwargs):
      for key in kwargs:
        if key == "db_loc":
          self.setDBLocData(StringPairVector(self.tuplify(kwargs[key])))
        elif key == "db_charge":
          self.setDBChargeData(StringPairVector(self.tuplify(kwargs[key])))
        elif key == "potential":
          self.setElecPotentialData(StringVector2D(self.tuplify(kwargs[key])))
        elif key == "db_pot":
          self.setDBPotData(StringVector2D(self.tuplify(kwargs[key])))
  }
/*
  %pythoncode{
    # Nathan TODO: migrate to a generic implementation. Do the unit conversion
    # in the simulation engine. There's also probably no need to make electrode
    # and DB properties available as dictionaries, I imagine access time might be
    # slower that way and it doesn't make it much more convenient. I can see the
    # appeal of making simulation parameters available as a dictionary though so
    # maybe we can keep doing that.
    #def getSimProps(self, key):
    #  if key == "electrodes":
    #    elecs = []
    #    m_per_A = 1.0E-10
    #    for elec in self.elec_col:
    #      elec_curr = {"x1":float(elec.x1), "x2":float(elec.x2), "y1":float(elec.y1), "y2":float(elec.y2), \
    #                   "potential":float(elec.potential),"layer_id":int(elec.layer_id), \
    #                   "electrode_type":int(elec.electrode_type), "pixel_per_angstrom":float(elec.pixel_per_angstrom), \
    #                   "phase":float(elec.phase)}
    #      elec_curr["x1"] *= m_per_A
    #      elec_curr["x2"] *= m_per_A
    #      elec_curr["y1"] *= m_per_A
    #      elec_curr["y2"] *= m_per_A
    #      elecs.append(elec_curr)
    #    return elecs
    #  if key == "dbs":
    #    dbs = []
    #    for db in self.db_col:
    #      db_curr = {"x":float(db.x), "y":float(db.y), "n": int(db.n), "m": int(db.m), "l": int(db.l)}
    #      dbs.append(db_curr)
    #    return dbs
    #  elif key == "parameters":
    #    sim_keys = ["bcs", "high_pot", "image_resolution", "low_pot", "max_abs_error",\
    #                "max_linear_iters", "max_rel_error", "mode", "sim_resolution", \
    #                "slice_depth", "steps"]
    #    sim_params = {}
    #    for key in sim_keys:
    #      sim_params[key] = self.getParameter(key)
    #    return sim_params
    #  else:
    #    return
  }
*/
  %pythoncode{
    def exportElecPotentialData(self, data_in):
      self.setElecPotentialData(StringVector2D(self.tuplify(data_in)))
  }
  %pythoncode{
    def exportDBPotData(self, data_in):
      self.setDBPotData(StringVector2D(self.tuplify(data_in)))
  }
  %pythoncode{
    def exportDBChargeData(self, data_in):
      self.setDBChargeData(StringPairVector(self.tuplify(data_in)))
  }
  %pythoncode{
    def exportDBLocData(self, data_in):
      self.setDBLocData(StringPairVector(self.tuplify(data_in)))
  }
  %pythoncode{
    def tuplify(self, data):
      if hasattr(data,'__iter__') and not hasattr(data, "strip"):
        return tuple(self.tuplify(i) for i in data)
      else:
        return str(data)
  }
}

%extend phys::DBCollection {
  phys::DBIterator __iter__() {
    phys::DBIterator iter = $self->begin();
    iter.setCollection($self);
    return iter;
  }
}

%extend phys::DBIterator
{
  phys::DBDot *__iter__() {
    return &***($self);
  }

  phys::DBDot *__next__() {
    if (*($self) == $self->collection->end()) {
      //PyErr_SetString(PyExc_StopIteration,"End of list");
      //PyErr_SetNone(PyExc_StopIteration);
      return NULL;
    }
    phys::DBDot *db = &***($self);
    $self->operator++();
    return db;
  }
}

%extend phys::ElectrodeCollection {
  phys::ElecIterator __iter__() {
    phys::ElecIterator iter = $self->begin();
    iter.setCollection($self);
    return iter;
  }
}

%extend phys::ElecIterator
{
  phys::Electrode *__iter__() {
    return &***($self);
  }

  phys::Electrode *__next__() {
    if (*($self) == $self->collection->end()) {
      //PyErr_SetString(PyExc_StopIteration,"End of list");
      //PyErr_SetNone(PyExc_StopIteration);
      return NULL;
    }
    phys::Electrode *elec = &***($self);
    $self->operator++();
    return elec;
  }
}
