#include "hale.h"
#include "../../comms.h"
#include "../../shared.h"
#include "../hale_data.h"
#include "../hale_interface.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Solve a single timestep on the given mesh
void solve_unstructured_hydro_3d(Mesh* mesh, HaleData* hale_data,
                                 UnstructuredMesh* umesh, const int timestep) {

  // On the first timestep we need to determine dt, and store the rezoned mesh
  if (timestep == 0) {
    printf("\nInitialising timestep and storing initial mesh.\n");

    set_timestep(umesh->ncells, umesh->nodes_x0, umesh->nodes_y0,
                 umesh->nodes_z0, hale_data->energy0, &mesh->dt,
                 umesh->cells_to_faces_offsets, umesh->cells_to_faces,
                 umesh->faces_to_nodes_offsets, umesh->faces_to_nodes, 
                 hale_data->reduce_array);

    // We are storing our original mesh to allow an Eulerian remap
    store_rezoned_mesh(umesh->nnodes, umesh->nodes_x0, umesh->nodes_y0,
                       umesh->nodes_z0, hale_data->rezoned_nodes_x,
                       hale_data->rezoned_nodes_y, hale_data->rezoned_nodes_z);
  }

  // Describe the subcell node layout
  printf("\nPerforming the Lagrangian Phase\n");

  struct Profile out;

  // Perform the Lagrangian phase of the ALE algorithm where the mesh will
  // move
  // due to the pressure (ideal gas) and artificial viscous forces
  START_PROFILING(&out);
  lagrangian_phase(mesh, umesh, hale_data);
  STOP_PROFILING(&out, "Lagrangian phase");

  if (hale_data->visit_dump) {
    write_unstructured_to_visit_3d(umesh->nnodes, umesh->ncells, timestep * 2,
                                   umesh->nodes_x0, umesh->nodes_y0,
                                   umesh->nodes_z0, umesh->cells_to_nodes,
                                   hale_data->density0, 0, 1);
  }

  if (hale_data->perform_remap) {
    printf("\nPerforming Gathering Phase\n");

    double initial_mass = 0.0;
    double initial_ie_mass = 0.0;
    double initial_ke_mass = 0.0;
    vec_t initial_momentum = {0.0, 0.0, 0.0};

    // gathers all of the subcell quantities on the mesh
    START_PROFILING(&out);
    gather_subcell_quantities(umesh, hale_data, &initial_momentum,
                              &initial_mass, &initial_ie_mass,
                              &initial_ke_mass);
    STOP_PROFILING(&out, "Gather phase");

    printf("\nPerforming Advection Phase\n");

    // Performs a remap and some scattering of the subcell values
    START_PROFILING(&out);
    advection_phase(umesh, hale_data);
    STOP_PROFILING(&out, "Advection phase");

    printf("\nPerforming Eulerian Mesh Rezone\n");

    // Performs an Eulerian rezone, returning the mesh and reconciling fluxes
    START_PROFILING(&out);
    eulerian_rezone(umesh, hale_data);
    STOP_PROFILING(&out, "Rezone phase");

    printf("\nPerforming Repair Phase\n");

    // Fixes any extrema introduced by the advection
    START_PROFILING(&out);
    mass_repair_phase(umesh, hale_data);
    STOP_PROFILING(&out, "Repair phase");
    printf("\nPerforming the Scattering Phase\n");

    // Perform the scatter step of the ALE remapping algorithm
    START_PROFILING(&out);
    scatter_phase(umesh, hale_data, &initial_momentum, initial_mass,
                  initial_ie_mass, initial_ke_mass);
    STOP_PROFILING(&out, "Scatter phase");

    // Fixes any extrema introduced by the advection
    START_PROFILING(&out);
    velocity_repair_phase(umesh, hale_data);
    energy_repair_phase(umesh, hale_data);
    STOP_PROFILING(&out, "Repair phase");

    PRINT_PROFILING_RESULTS(&out);
  }
}
