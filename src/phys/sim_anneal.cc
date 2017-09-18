// @file:     sim_anneal.cc
// @author:   Samuel
// @created:  2017.08.23
// @editted:  2017.08.23 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Simulated annealing physics engine

#include "sim_anneal.h"

using namespace phys;

SimAnneal::SimAnneal(const std::string& fname)
  : PhysicsEngine(fname)
{

}


bool SimAnneal::runSim()
{
  // INIT VARS
  // user controlled (TODO move to .xml in the future)
  float E_begin, E_end, E_converge = 1E-5; // NOTE arbitrary
  int converge_count = 0, converge_threshold = 10;
  int max_t = 10000, preanneal_t = 1000;
  n_dbs = problem.db_tree->size();
  kT = 2.568E-2; kT_step = 0.999999; // kT = Boltzmann constant (eV/K) * 298 K, NOTE kT_step arbitrary
  v_freeze = 0, v_freeze_step = 0.001; // NOTE v_freeze_step arbitrary
  v_0 = 10;

  // general
  int i=0,j=0;

  // hopping variables
  int from_db, to_db;
  bool hop_done;
  int hop_count;
  float E_pre_hop, E_post_hop;
  bool converged = false;

  std::cout << "n_dbs=" << n_dbs << std::endl;

  // grab all physical locations (in original distance unit)
  std::cout << "Grab all physical locations..." << std::endl;
  for(Problem::DBIterator db_iter = problem.begin(); db_iter != problem.end(); ++db_iter)
    db_phys_loc.push_back(std::make_pair((**db_iter).x, (**db_iter).y));

  // resize vectors
  v_eff.resize(n_dbs);
  v_electrodes.resize(n_dbs);
  v_ij.resize(n_dbs);
  db_r.resize(n_dbs);
  db_charges.resize(n_dbs);

  // pre-calculation
  std::cout << "Performing pre-calculation..." << std::endl;
  std::cout << "Size of db_phys_loc=" << db_phys_loc.size() << std::endl;
  for(i=0; i<n_dbs; i++) {
    db_r[i].resize(n_dbs);
    v_ij[i].resize(n_dbs);
    for(j=0; j<n_dbs; j++) {
      if (i>j) {
        db_r[i][j] = db_r[j][i];
        v_ij[i][j] = v_ij[j][i];
      }
      else if (i==j) {
        db_r[i][j] = 0;
        v_ij[i][j] = div_0;
      }
      else {
        db_r[i][j] = sqrt(pow(db_phys_loc[i].first - db_phys_loc[j].first, 2.0) + pow(db_phys_loc[i].second - db_phys_loc[j].second, 2.0))*db_distance_scale; // converted to m
        //v_ij[i][j] = exp(-db_r[i][j]/debye_length) / db_r[i][j];
        v_ij[i][j] = 1.6E-19 / (4*3.14*8.854E-12) * exp(-db_r[i][j]/debye_length) / db_r[i][j]; // (close to) real v_ij, TODO revert to the version above after verification stage
        std::cout << "db_r[" << i << "][" << j << "]=" << db_r[i][j] << ", v_ij[" << i << "][" << j << "]=" << v_ij[i][j] << std::endl;
      }
    }
    v_eff[i] = 0;
    v_electrodes[i] = 0; // NOTE temporary
    db_charges[i] = 0;
  }

  // SIM ANNEAL
  std::cout << "Performing simulated annealing..." << std::endl;
  while(!converged) {
    E_begin = systemEnergy();

    // Population
    std::cout << "Population update, v_freeze=" << v_freeze << std::endl;
    for(i=0; i<n_dbs; i++) {
      v_eff[i] = v_0 + v_electrodes[i];
      for(j=0; j<n_dbs; j++)
        if(i!=j)
          v_eff[i] -= v_ij[i][j] * db_charges[j];

      // accept population change?
      db_charges[i] = acceptPop(i) ? !db_charges[i] : db_charges[i];
    }
    printCharges();

    // Hopping
    std::cout << "Hopping" << std::endl;
    hop_done = false;
    hop_count = 0;
    // TODO might have to store the previous hop(s) that has been performed to reduce redundant calculations
    while(!hop_done) {
      E_pre_hop = systemEnergy(); // original energy
      from_db = getRandDBInd(true);
      to_db = getRandDBInd(false);

      if(from_db == -1 || to_db == -1)
        break; // hopping not possible

      // perform the hop
      db_charges[from_db] = 0;
      db_charges[to_db] = 1;
      E_post_hop = systemEnergy(); // new energy

      // accept hop given energy change? reverse hop if energy delta is unaccpted
      if(!acceptHop(E_post_hop-E_pre_hop)) // TODO WARNING acceptance function here contains placeholder values
        db_charges[from_db] = 1, db_charges[to_db] = 0;

      hop_count++; // NOTE might not be useful at the end, delete this line if so

      // determine hop_done
      if(hop_count == 10) // TODO WARNING completely arbitrary
        hop_done = true;
    }
    std::cout << std::endl;

    // Pre-annealing
    if(preanneal_t > 0)
      preanneal_t--;
    else
      timeStep();

    // determine convergence
    E_end = systemEnergy();
    if(!preanneal_t && (E_end-E_begin < E_converge)) {
      converge_count++;
      if(converge_count >= converge_threshold)
        converged = true;
    }
    else
      converge_count = 0;

    // print statistics
    std::cout << "Cycle: " << ((preanneal_t > 0) ? -preanneal_t : t);
    std::cout << ", ending energy: " << E_end;
    std::cout << ", delta: " << E_end-E_begin << std::endl << std::endl;

    if(!converged && t >= max_t) {
      std::cout << "Maximum time steps (" << max_t << ") reached, convergence unsuccessful." << std::endl;
      break;
    }
  }

  return true;
}


void SimAnneal::timeStep()
{
  t++;
  kT *= kT_step; // TODO use exponential drop off instead of constant decrement
  v_freeze += v_freeze_step;
}


void SimAnneal::printCharges()
{
  for(int i : db_charges)
    std::cout << i;
  std::cout << std::endl;
}





// ACCEPTANCE FUNCTIONS


bool SimAnneal::acceptPop(int db_ind)
{
  //float k = 8.61733E-5; // Boltzmann constant in eV/K, might have to switch to other units later
  //float v = direction ? v_eff[db_ind]-v_freeze : v_freeze-v_eff[db_ind];
  int curr_charge = db_charges[db_ind];
  float v = curr_charge ? v_eff[db_ind] + v_freeze : - v_eff[db_ind] + v_freeze; // 1->0 : 0->1
  float prob;
  
  prob = 1. / ( 1 + exp( v/kT ) );

  std::cout << "v_eff=" << v_eff[db_ind] << ", prob of population change from " << curr_charge << " to " << !curr_charge << ": " << prob << std::endl;
  //std::cout << "v_eff=" << v_eff[db_ind] << ", v_freeze=" << v_freeze << std::endl;

  return evalProb(prob);
}


// acceptance function for hopping
bool SimAnneal::acceptHop(float v_diff)
{
  if (v_diff < 0)
    return true;

  // some acceptance function, acceptance probability falls off exponentially
  float prob = 0; // TODO WARNING placeholder
  return evalProb(prob);
}


// takes a probability and generates true/false accordingly
bool SimAnneal::evalProb(float prob)
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(0.0,1.0);

  return prob >= dis(gen);
}





// ACCESSORS


int SimAnneal::getRandDBInd(bool occ)
{
  std::vector<int> dbs;

  // store the indices of dbs that have the desired occupation
  for (unsigned int i=0; i<db_charges.size(); i++)
    if (db_charges[i] == occ)
      dbs.push_back(i);

  if (dbs.empty())
    return -1; // no potential candidates

  // pick one from them
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(0.0,static_cast<float>(dbs.size()));

  return dbs[dis(gen)];
}


float SimAnneal::systemEnergy()
{
  assert(n_dbs > 0);
  float v = v_0;
  for(int i=0; i<n_dbs; i++) {
    v += db_charges[i] * v_electrodes[i];
    for(int j=i; j<n_dbs; j++)
      v += db_charges[i] * db_charges[j] * v_ij[i][j];
  }
  return v * har_to_ev;
}

