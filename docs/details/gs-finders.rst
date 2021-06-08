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


QUBO Mapping
------------

We have attempted to map the ground state model to QUBO. The effort resulted in some success but accuracy and performance was subpar. More information regarding the QUBO mapping can be found in Chapter 3.2.3 in :cite:`Ng2020_thes`.


Interpreting Results
--------------------

After a ground state simulation has been performed, SiQAD displays the lowest energy physically valid configuration. A full guide on interpreting the simulation results will be provided in the future. For now, please pay attention to the following important notes:

* **"Config energy"** provided in the ground state simulation results **do not** represent the energy dissipation to reach this configuration, it is merely the accumulated screened Coulombic potentials (Eq. (1) in :cite:`Ng2020_siqad`). There is currently no known method to accurately estimate the energy cost of SiDBs in actual operation, but attempts at providing pessimistic approximations have been made (expected upper bound in energy use). Please see Chapter 5.2 of :cite:`Chiu2020_thes` and Chapter 4.3 of :cite:`Ng2020_thes` for relevant discussions.

* **"Config occurance"** informs how many times a particular configuration was encountered by the ground state simulator. However, it **does not** represent the physical probability of reaching this state in actual operation. In a physical setting, many considerations need to be taken into account -- the exact layout, nearby defects, rate and method of clocking, etc.

* **"Physically valid"** informs whether the charge configuration is deemed to be metastable. In SimAnneal and ExhaustiveGS, metastability is defined by two criteria (reprinted verbatim from :cite:`Ng2020_thes`):

  * *configuration stability*, where no lower energy charge configurations exist that can be reached within a single hop event; and

  * *population stability*, where the charge state of each DB must be consistent with the energetic position of the charge transition levels relative to the Fermi energy after accounting for band bending effects.


.. todo::
    
   Refine the explanations and link more relevant resources.


Time-to-solution Benchmarks
---------------------------

Time-to-solution (TTS) benchmarks are available in Chapter 3.2.5 in :cite:`Ng2020_thes`.



Inter-plugin Transfer
=====================

.. todo::
    
   Workflow for exporting PoisSolver potentials to SimAnneal.


