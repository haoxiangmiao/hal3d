#ifndef __HALEINTERFACEHDR
#define __HALEINTERFACEHDR

#pragma once

#include "hale_data.h" // An important part of the interface
#include "../shared.h"
#include "../mesh.h"
#include "../shared_data.h"

// Controllable parameters for the application
#define GAM 1.4
#define C_Q 3.0
#define C_M (1.5/C_T)

#ifdef __cplusplus
extern "C" {
#endif

  // Solve a single timestep on the given mesh
  void solve_unstructured_hydro_2d(
      Mesh* mesh, const int ncells, const int nnodes, const double visc_coeff1, 
      const double visc_coeff2, double* cell_centroids_x, double* cell_centroids_y, 
      int* cells_to_nodes, int* cells_offsets, int* nodes_to_cells, int* cells_to_cells,
      int* nodes_offsets, double* nodes_x0, double* nodes_y0, double* nodes_x1, 
      double* nodes_y1, int* boundary_index, int* boundary_type, double* boundary_normal_x, 
      double* boundary_normal_y, double* energy0, double* energy1, double* density0, 
      double* density1, double* pressure0, double* pressure1, double* velocity_x0, 
      double* velocity_y0, double* velocity_x1, double* velocity_y1, 
      double* cell_force_x, double* cell_force_y, double* node_force_x, 
      double* node_force_y, double* node_force_x2, double* node_force_y2, 
      double* cell_mass, double* nodal_mass, double* nodal_volumes, 
      double* nodal_soundspeed, double* limiter, double* sub_cell_energy, 
      double* sub_cell_mass, double* sub_cell_velocity_x, 
      double* sub_cell_velocity_y);

  // Controls the timestep for the simulation
  void set_timestep(
      const int ncells, const int* cells_to_nodes, const int* cells_to_nodes_off,
      const double* nodes_x, const double* nodes_y, const double* energy0, double* dt);

#ifdef __cplusplus
}
#endif

#endif

