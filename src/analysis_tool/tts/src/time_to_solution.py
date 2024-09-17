import numpy as np
import time
import matplotlib.pyplot as plt
from pysimanneal import simanneal
from mnt.pyfiction import *  # Ensure this import is correct
from datetime import datetime


# Function to generate simulation parameters
def initialize_simulation(mu=-0.32, lambda_tf=5.0, epsilon_r=5.6, num_instances=1):
    """
    Initializes physical and SimAnneal simulation parameters.

    Parameters:
    - mu: Chemical potential
    - lambda_tf: Thermal length
    - epsilon_r: Relative permittivity
    - num_instances: Number of instances for SimAnneal

    Returns:
    - physical_parameters: Configured physical parameters for the simulation
    - sp: Configured SimAnneal parameters
    """
    physical_parameters = sidb_simulation_parameters()
    physical_parameters.base = 2
    physical_parameters.mu_minus = mu
    physical_parameters.lambda_tf = lambda_tf
    physical_parameters.epsilon_r = epsilon_r

    sp = simanneal.SimParams()
    sp.mu = mu
    sp.num_instances = num_instances
    return physical_parameters, sp

# Function to generate a random SiDB layout
def generate_layout(generate_params, physical_parameters):
    """
    Generates a random SiDB layout and initializes charge distribution.

    Parameters:
    - generate_params: Parameters for generating the SiDB layout
    - physical_parameters: Physical parameters for the simulation

    Returns:
    - layout: Generated SiDB layout
    - cds: Charge distribution surface object
    """
    layout = generate_random_sidb_layout(sidb_100_lattice(), generate_params)
    cds = charge_distribution_surface_100(layout, physical_parameters)
    return layout, cds

# Function to run QuickExact simulation
def run_quickexact_simulation(layout, physical_params):
    """
    Runs QuickExact simulation and returns the results.

    Parameters:
    - layout: SiDB layout
    - physical_params: Physical parameters for the simulation

    Returns:
    - result_quickexact: Result from QuickExact simulation
    """
    quickexact_params_inst = quickexact_params()
    quickexact_params_inst.simulation_parameters = physical_params
    quickexact_params_inst.base_number_detection = automatic_base_number_detection.OFF
    result_quickexact = quickexact(layout, quickexact_params_inst)
    return result_quickexact

# Function to run SimAnneal simulations for a given set of parameters
def run_simanneal_simulation(sp, layout, physical_parameters, layout_coordinates_angstrom, num_simulations=100):
    """
    Runs SimAnneal simulations and collects results.

    Parameters:
    - sp: SimAnneal parameters
    - layout: SiDB layout
    - physical_parameters: Physical parameters for the simulation
    - layout_coordinates_angstrom: Coordinates of the layout in Angstroms
    - num_simulations: Number of SimAnneal simulations to run

    Returns:
    - all_sa_solution: List of simulation results
    """
    sp.set_db_locs(layout_coordinates_angstrom)
    all_sa_solution = []

    for _ in range(num_simulations):
        sa = simanneal.SimAnneal(sp)

        start_time = time.time()
        sa.invokeSimAnneal()
        simulation_runtime_sa = time.time() - start_time

        results = sa.suggested_gs_results()
        pyfiction_simanneal_results = sidb_simulation_result_100()
        pyfiction_simanneal_results.algorithm_name = "simanneal"
        pyfiction_simanneal_results.simulation_runtime = simulation_runtime_sa

        # Gather charge distributions
        all_cds_solutions = []
        for res in results:
            cds_solution = charge_distribution_surface_100(layout, physical_parameters)
            for c, bit in enumerate(res.config):
                cds_solution.assign_charge_state_by_cell_index(c, sign_to_charge_state(bit))
            all_cds_solutions.append(cds_solution)

        pyfiction_simanneal_results.charge_distributions = all_cds_solutions
        all_sa_solution.append(pyfiction_simanneal_results)

    return all_sa_solution

# Function to calculate time-to-solution (TTS) statistics
def calculate_tts(result_quickexact, all_sa_solution):
    """
    Calculates the time-to-solution (TTS) from the results of SimAnneal simulations.

    Parameters:
    - result_quickexact: Result from QuickExact simulation
    - all_sa_solution: List of SimAnneal simulation results

    Returns:
    - time_to_solution: Time-to-solution statistic
    """
    st = time_to_solution_stats()
    time_to_solution_for_given_simulation_results(result_quickexact, all_sa_solution, 0.997, st)
    return st.time_to_solution

# Function to plot TTS heatmap and/or 3D bar plot
def plot_tts(tts_data, param1_values, param2_values, param1_name, param2_name, plot_heatmap=True, plot_3d=False):
    """
    Plots and saves a heatmap and/or a 3D bar plot of the time-to-solution (TTS) data.

    Parameters:
    - tts_data: 2D array of TTS values
    - param1_values: List of values for the first parameter
    - param2_values: List of values for the second parameter
    - param1_name: Name of the first parameter
    - param2_name: Name of the second parameter
    - plot_heatmap: Boolean flag to plot heatmap
    - plot_3d: Boolean flag to plot 3D bar plot
    """
    # Get current time in the format YYYYMMDD_HHMMSS
    current_time = datetime.now().strftime('%Y%m%d_%H%M%S')

    if plot_heatmap:
        # Adjust figure size to account for different ranges of x and y axes
        plt.figure(figsize=(10, 8))  # Dynamically scale figure width based on aspect ratio

        plt.imshow(tts_data, cmap='viridis', interpolation='nearest', origin='lower',
                   extent=[param1_values[0], param1_values[-1], param2_values[0], param2_values[-1]],
                   aspect='auto')  # 'auto' stretches to fit, or use 'equal' for proportional axes

        plt.colorbar(label='Time-to-Solution (seconds)')
        plt.xlabel(param1_name)
        plt.ylabel(param2_name)
        plt.title(f'Time-to-Solution Heatmap for {param1_name} and {param2_name}')

        plt.tight_layout()  # Ensures everything fits in the figure
        plt.savefig(f'plots/tts_heatmap_{param1_name}_{param2_name}_{current_time}.png')
        plt.show()

    if plot_3d:
        X, Y = np.meshgrid(param1_values, param2_values)
        fig = plt.figure(figsize=(12, 8))
        ax = fig.add_subplot(111, projection='3d')
        ax.bar3d(X.flatten(), Y.flatten(), np.zeros_like(X.flatten()),
                 dx=(param1_values[-1] - param1_values[0]) / len(param1_values),
                 dy=(param2_values[-1] - param2_values[0]) / len(param2_values),
                 dz=tts_data.T.flatten(), color='c', alpha=0.7)

        ax.set_xlabel(param1_name)
        ax.set_ylabel(param2_name)
        ax.set_zlabel('Time-to-Solution (seconds)')
        ax.set_title(f'Time-to-Solution 3D Bar Plot for {param1_name} and {param2_name}')
        plt.savefig(f'plots/tts_3d_plot_{param1_name}_{param2_name}_{current_time}.png')
        plt.show()

# Main function for hyperparameter tuning
# Function for hyperparameter tuning with TTS threshold
def hyperparameter_tuning(physical_parameters, param_types, sp, param1, param1_values, param2, param2_values, layout, layout_coordinates_angstrom, num_simulations, plot_heatmap=True, plot_3d=False):
    """
    Performs hyperparameter tuning by running simulations with different parameter values.
    Aborts if TTS exceeds a given threshold.

    Parameters:
    - physical_parameters: Physical parameters for the simulation
    - sp: SimAnneal parameters
    - param1: Name of the first parameter to tune
    - param1_values: List of values for the first parameter
    - param2: Name of the second parameter to tune
    - param2_values: List of values for the second parameter
    - layout: SiDB layout
    - layout_coordinates_angstrom: Coordinates of the layout in Angstroms
    - num_simulations: Number of SimAnneal simulations to determine TTS
    - tts_threshold: Threshold value for TTS to abort the simulation
    - plot_heatmap: Boolean flag to plot heatmap
    - plot_3d: Boolean flag to plot 3D bar plot
    """
    result_quickexact = run_quickexact_simulation(layout, physical_parameters)

    # Initialize TTS data array
    tts_data = np.zeros((len(param1_values), len(param2_values)))

    # Loop through combinations of param1 and param2
    for i, p1_value in enumerate(param1_values):
        for j, p2_value in enumerate(param2_values):
            try:
                # Set simulation parameters dynamically based on the expected type
                if param1 in param_types:
                    p1_value = param_types[param1](p1_value)  # Cast to the correct type
                if param2 in param_types:
                    p2_value = param_types[param2](p2_value)  # Cast to the correct type

                setattr(sp, param1, p1_value)
                setattr(sp, param2, p2_value)
            except (TypeError, ValueError) as e:
                print(f"Error setting parameters: {e}")
                continue

            # Run the SimAnneal simulation
            all_sa_solution = run_simanneal_simulation(sp, layout, physical_parameters, layout_coordinates_angstrom, num_simulations)

            # Calculate TTS for the current (param1, param2) combination
            tts_value = calculate_tts(result_quickexact, all_sa_solution)
            tts_data[i, j] = tts_value

            print(f"{param1}: {p1_value} | {param2}: {p2_value} | TTS: {tts_value}")

            tts_threshold = 10000  # Threshold value for TTS to abort the simulation
            # Check if the TTS exceeds the threshold
            if tts_value > tts_threshold:
                tts_data[i, j] = 0
                print(f"TTS threshold exceeded: {tts_value} > {tts_threshold}")
                #return  # Exit the function if the TTS threshold is exceeded

    # Plot the TTS data
    plot_tts(tts_data, param1_values, param2_values, param1, param2, plot_heatmap, plot_3d)



def main():
    """
    Main function to set up parameters, generate layout, and perform hyperparameter tuning.
    """
    # Set up layout parameters and physical properties

    physical_parameters, sp = initialize_simulation()

    number_of_simanneal_simulation_runs_to_determine_tts = 400

    # Generate layout parameters
    generate_params = generate_random_sidb_layout_params()
    generate_params.number_of_sidbs = 30
    generate_params.positive_sidbs = positive_charges.OFF
    generate_params.coordinate_pair = ((0, 0), (20, 20))

    layout, cds = generate_layout(generate_params, physical_parameters)

    # Convert coordinates from nm to angstroms
    all_positions_nm = cds.get_all_sidb_locations_in_nm()
    layout_coordinates_angstrom = [[pos[0] * 10, pos[1] * 10] for pos in all_positions_nm]

    param_types = {
        'anneal_cycles': int,
        'T_e_inv_point': float,
        'v_freeze_end_point': float,
        'num_instances': int,
        'result_queue_factor': int,
        'result_queue_size': int,
        'hop_attempt_factor': int,
        'preanneal_cycles': int,
        'alpha': float,
        'T_init': float,
        'T_min': float,
        'v_freeze_init': float,
        'v_freeze_threshold': float,
        'v_freeze_reset': float,
        'v_freeze_cycles': int,
        'phys_validity_check_cycles': int,
        'strategic_v_freeze_reset': bool,
        'reset_T_during_v_freeze_reset': bool,
        'v_freeze_step': float,
    }

    # Automatically set parameter values based on their type
    param1 = 'anneal_cycles'
    param2 = 'T_init'

    if param1 in param_types:
        param1_type = param_types[param1]
        param1_values = np.linspace(200, 600, num=10).astype(param1_type)

    if param2 in param_types:
        param2_type = param_types[param2]
        param2_values = np.linspace(100, 10000, num=10).astype(param2_type)

    # Perform hyperparameter tuning
    hyperparameter_tuning(
        physical_parameters,param_types, sp, param1, param1_values, param2, param2_values,
        layout, layout_coordinates_angstrom, number_of_simanneal_simulation_runs_to_determine_tts,
        plot_heatmap=True,  # Set to False if you don't want to plot heatmap
        plot_3d=True        # Set to False if you don't want to plot 3D bar plot
    )

if __name__ == "__main__":
    main()
