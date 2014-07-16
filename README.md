# README #

### Description ###

This code runs a simulation based on heat diffusion. The aim of this project is to learn how can we use netcdf and hdf5. In order to do that the project is splitted in several steps:

1. Solve heat equation in 3D (heat.c).
2. Store the four dimensions (t,x,y,z) in HDF-5 file (heat.c).
3. Show a slice of the sphere from the HDF-5 file (heat.h5.py).
4. Choose two dimensions from HDF-5 file and store them in the NetCDF file (hdf5tonetcdf.c).
5. Show a slice of the sphere from the NetCDF file (heat.nc.py).


### Run ###

1. Compile the code.
   Type 'make' (you will need installed hdf5 and netcdf libraries).

2. Run the simulation and conversion.
   Type 'make run'.

3. Show a slice from x and z dimensions using the fifty sheet in y. HDF-5 format.
   Type 'make run_h5'

4. Show a slice from y and z dimensions using the twenty-five sheet in x. NetCDF format.
   Type 'make run_net'
