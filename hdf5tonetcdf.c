#include <math.h>
#include <hdf5.h>
#include <netcdf.h>
#include <hdf5_hl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define H5_FILE_NAME "data.h5"
#define ERR { H5Eprint( H5E_DEFAULT, stdout ); exit(2); }
#define NC_FILE_NAME "data.nc"
#define ERROR(e) { printf( "Error: %s\n", nc_strerror(e) ); exit(2); }

int main(int argc, char *argv[]) {
  double *u;
  int i,j,k,t;
  int nx, ny, nz, sheet, first_var, second_var;
  char *first_str, *second_str;
  hsize_t dims_out[4];
  herr_t status;
 
  first_str = (char *) malloc(sizeof(char)*2);
  second_str = (char *) malloc(sizeof(char)*2);

  if(argc < 3) {
    printf("Error missing some parameters\n");
    printf("Use: %s <dimension([x->0|y->1|z->2])> <sheet>\n", argv[0]);
    exit(1);
  }

  sheet = atoi(argv[2]);

  // HDF5  MANAGEMENT //
  hid_t file_id;
  file_id = H5Fopen( H5_FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT );
  if( file_id < 0 ) { ERR; }
 
  // Read "temperatura" dataset.
  hid_t tempD = H5Dopen1(file_id, "temperatura");
  if( tempD < 0 ) { ERR; }

  // Read the initial state
  hid_t tempSel = H5Dget_space( tempD );
  if( tempSel < 0 ) { ERR; }

  H5Sget_simple_extent_dims(tempSel, dims_out, NULL);
  printf("dimensions %lu x %lu x %lu x % lu\n", 
	   (unsigned long)(dims_out[0]), (unsigned long)(dims_out[1]),
           (unsigned long)(dims_out[2]), (unsigned long)(dims_out[3]));

  nx = dims_out[1];
  ny = dims_out[2];
  nz = dims_out[3];

  hsize_t sel_offset[4] = {0, 0, 0, 0};
  hsize_t sel_length[4] = {1, 1, 1, 1};

  switch(atoi(argv[1])) {
    // Y & Z
    case 0: {
       first_var = ny;
       second_var = nz;
       sel_offset[1] = sheet;
       sel_length[2] = ny;
       sel_length[3] = nz;
       memcpy(first_str, "y", sizeof(char));
       memcpy(second_str, "z", sizeof(char));
       break;
    }
    // X & Z
    case 1: {
       first_var = nx;
       second_var = nz;
       sel_offset[2] = sheet;
       sel_length[1] = nx;
       sel_length[3] = nz;
       memcpy(first_str, "x", sizeof(char));
       memcpy(second_str, "z", sizeof(char));
       break;
    }
    // X & Y
    default: {
       first_var = nx;
       second_var = ny;
       sel_offset[3] = sheet;
       sel_length[1] = nx;
       sel_length[2] = ny;
       memcpy(first_str, "x", sizeof(char));
       memcpy(second_str, "y", sizeof(char));
    }
  }

  u = (double*)malloc(sizeof(double)*first_var*second_var);

  hsize_t dims_in[2];
  dims_in[0] = first_var;
  dims_in[1] = second_var;

  // Allocate space
  hid_t memS = H5Screate_simple( 2, dims_in, NULL );
  if( memS < 0 ) { ERR; }
  H5Sselect_hyperslab( tempSel, H5S_SELECT_SET, sel_offset, NULL, sel_length,
                       NULL );

  // NETCDF  MANAGEMENT //
  int ncid, retval;
  // Create dataset
  if( retval = nc_create( NC_FILE_NAME, NC_CLOBBER, &ncid ) ) {
    ERROR(retval);
  }
  int first_dimid, second_dimid, t_dimid;
  // Declare and define dimensions
  if( retval = nc_def_dim( ncid, "t", dims_out[0], &t_dimid ) ) {
    ERR(retval);
  }
  if( retval = nc_def_dim( ncid, first_str, first_var, &first_dimid ) ) {
    ERROR(retval);
  }
  if( retval = nc_def_dim( ncid, second_str, second_var, &second_dimid ) ) {
    ERROR(retval);
  }

  int dimids[3] = { t_dimid, first_dimid, second_dimid };
  int varid;
  // Define variable
  if( retval = nc_def_var( ncid, "temperature", NC_DOUBLE, 3, dimids, &varid )){
    ERROR(retval);
  }
  if( retval = nc_enddef( ncid ) ) {
    ERROR(retval);
  }

  size_t corner_vector[3] = {0,0,0};
  size_t edge_lengths[3] = {1, first_var, second_var};

  // Read all timesteps. Perform format conversion
  for( t = 0; t < dims_out[0]; ++t ) {
    printf( "TIMESTEP = %d\n", t );
    // Read the temperature data from the HDF5 file
    if( tempSel < 0 ) { ERR; }
    status = H5Dread( tempD, H5T_NATIVE_DOUBLE, memS, tempSel, H5P_DEFAULT, u);
    if( status < 0 ) { ERR; }

    // Write the temperature data to the NetCDF file
    if(retval=nc_put_vara_double(ncid, varid, corner_vector, edge_lengths, u)) {
      ERROR(retval);
    }

    // Next step in NetCDF structure
    corner_vector[0] = t+1;

    // Select next hyperslab in the HDF5 file
    if( tempSel < 0 ) { ERR; }
    sel_offset[0] = t+1;
    H5Sselect_hyperslab( tempSel, H5S_SELECT_SET, sel_offset, NULL, sel_length,
                         NULL );
  }

  // Cleanup
  if( H5Sclose( tempSel ) <  0 ) { ERR; }
  if( H5Dclose( tempD ) <  0 ) { ERR; }
  if( H5Fclose( file_id ) <  0 ) { ERR; }
  if( retval = nc_close( ncid ) ) {
    ERROR(retval);
  }
  free(u);
  free(first_str);
  free(second_str);
  return 0;
}
