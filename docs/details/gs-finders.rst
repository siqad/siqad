.. _gs_finders_doc:

Ground State Charge Config Finders
**********************************

Ground State Model
==================

.. todo::

   A general discussion of the ground state model will be made available here in the near future. The model is discussed in :cite:`Ng2020_siqad` in the context of SimAnneal, which is equally informative.


ExhaustiveGS
------------

ExhaustiveGS assesses all possible charge configurations in any arbitrary DB layouts with the ground state model. It scales at :math:`\text{O}(N^3)` where :math:`N` is the DB count, which makes it prohibitively costly to run for larger DB problems. As a rule of thumb, we advise avoiding ExhaustiveGS for problems with more than ~15 DBs but actual performance depends on your machine.


SimAnneal
---------

SimAnneal is a heuristic algorithm for simulated annealing which attempts to find the ground state metastable charge configuration for given DB layouts. For details on the implementation and associated models, please refer to Section IV.A. in :cite:`Ng2020_siqad`. Here, we focus on the discussion of runtime parameters assuming that you've already read that section of the paper.

Here is a table of configurable SimAnneal parameters and their descriptions:

.. table:: SimAnneal Parameters
   :widths: auto

   ===========================  ======================================  =================
   Variable                     Param Name in SiQAD                     Description
   ===========================  ======================================  =================
   Physical:
   --------------------------------------------------------------------------------------
   :math:`\vec{n}`              (Exported by SiQAD)                     All DB locations.
   :math:`\mu_-`                mu (eV)                                 More negative = more favorable for DB-, see :cite:`Ng2020_siqad`.
   :math:`\epsilon_r`           Relative permittivity                   Affects electric field strength, see :cite:`Ng2020_siqad`.
   :math:`\lambda_\text{TF}`    Screening distance (nm)                 Thomas-Fermi screening distance, see :cite:`Ng2020_siqad`.
   :math:`V^\text{ext}`         (Inter-plugin transfer)                 External potentials, requires `inter-plugin transfer`_.
   Simulation:
   --------------------------------------------------------------------------------------
   :math:`N_\text{inst}`        Instance count                          Number of SimAnneal instances to spawn.
   :math:`N_\text{cycles}`      Anneal cycles                           Number of annealing time steps per instance.
   :math:`T_0`                  Initial temperature (K)                 Initial annealing temperature.
   :math:`T_\text{min}`         Minimum temperature (K)                 Minimum annealing temperature.
   :math:`V_{f0}`               Initial V_freeze (eV)                   Initial :math:`V_f`, see :cite:`Ng2020_siqad`.
   :math:`V_{f}'`               Final V_freeze (eV)                     Final :math:`V_f`, see :cite:`Ng2020_siqad`.
   :math:`\tau_{V_f'}`          V_freeze cycles                         Normalized time step at which :math:`V_f = V_f'` and ceases to increase further.
   :math:`\tau_{T_0/e}`         Temperature 1/e point                   Normalized time step at which :math:`T = \frac{T_0}{e}`.
   :math:`f_\text{hop}`         Hop attempt factor                      Set the number of attempted hops at each time step to :math:`f_\text{hop} \times` count of DB0 sites.
   Results:
   --------------------------------------------------------------------------------------
   :math:`f_\text{results}`     Result queue size                       Set the number of charge configurations to return per instance to :math:`f_\text{results} \times N_\text{cycles}`
   ===========================  ======================================  =================


The following additional variables are related to schedule restarting. It is still unclear to us whether they offer advantages that optimizing the annealing schedule doesn't already offer, therefore they are disabled by default. The parameters include:

* Strategically reset V_freeze: enable V_freeze reset if simulation seems to be stuck at unstable states for a user-defined number of cycles after V_freeze has reached the maximum value.
* Reset temperature as well: when resetting V_freeze, also reset the annealing temperature.
* Reset V_freeze: V_freeze value to reset to, leave at :math:`-1` for SimAnneal to decide automatically.
* Physical validity check cycles: after V_freeze has reached the maximum value, reset V_freeze if the charge configurations do not fit metastability criteria for this many cycles consecutively.

.. todo::

    Include a plot showing some key variables affecting temperature and v_freeze schedules.


QUBO Mapping
------------

.. todo::

    We have attempted to map the ground state model to QUBO. The effort resulted in some success but accuracy and performance was far behind SimAnneal. More information regarding the QUBO mapping will be provided in the future.


Inter-plugin Transfer
=====================

.. todo::
    
   Workflow for exporting PoisSolver potentials to SimAnneal.
