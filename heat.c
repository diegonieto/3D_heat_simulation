#include <math.h>
#include <hdf5.h>
#include <hdf5_hl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define DX .01   // Interval size in x direction
#define DY .01   // Interval size in y direction
#define DZ .01   // Interval size in y direction
#define A 0.5    // Diffusion constant
#define STEPS 50 // Timesteps to compute


#define H5_FILE_NAME "data.h5"
#define ERR { H5Eprint( H5E_DEFAULT, stdout ); exit(2); }


void print_matrix( double * u, int nx, int ny ) {
  int i,j;

  for( i = 1; i < nx-1; ++i ) {
    for( j = 1; j < ny-1; ++j ) {
      printf( "%lf ", u[i*ny+j] );
    }
    printf( "\n" );
  }
}

int main() {
  double *u, *ui;
  int i,j,k,t;

  int nx = 1/DX;
  int ny = 1/DY;
  int nz = 1/DZ;

  // To save CPU cycles, we'll compute Delta x^2 and Delta y^2 only once
  double dx2 = DX*DX;
  double dy2 = DY*DY;
  double dz2 = DZ*DZ;

  //double dt = dx2*dy2/(2*A*(dx2+dy2));
  //double dt = 1e-9;
  double dt = 3e-5;

  u = (double*)malloc(sizeof(double)*nx*ny*nz);
  ui = (double*)malloc(sizeof(double)*nx*ny*nz);

  for( i = 0; i < nx; ++i ) {
    for( j = 0; j < ny; ++j ) {
	for(k = 0; k < nz; ++k ) {
      	   double dist_to_center = pow(i*DX-0.5,2)+pow(j*DY-0.5,2)+pow(k*DZ-0.5,2);
	   if( ( dist_to_center < 0.1 ) &&
	       ( dist_to_center > 0.05 ) ) {
       		 ui[i*ny*nz+j*nz+k] = 1.0;
	   } else {
		 ui[i*ny*nz+j*nz+k] = 0.0;
	   }
	}
    }
  }

  // Create HDF5 file. The H5F_ACC_TRUNC parameter tells HDF5 to overwrite the
  // this file if it already exists.
  hid_t file_id;
  file_id = H5Fcreate( H5_FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT );
  if( file_id < 0 ) { ERR; }
 
  // Create a dataspace for the temperature.
  hsize_t dims[4] = { STEPS+1, nx, ny, nz };
  hid_t tempS = H5Screate_simple( 4, dims, NULL );
  if( tempS < 0 ) { ERR; }
  
  // Create the temperature dataset into the root group of the file
  hid_t tempD = H5Dcreate( file_id, "temperatura", H5T_IEEE_F64BE, tempS,
    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
  if( tempD < 0 ) { ERR; }
  
  // Write the initial state
  hid_t tempSel = H5Dget_space( tempD );
  if( tempSel < 0 ) { ERR; }
  hsize_t sel_offset[4] = {0,0,0,0};
  hsize_t sel_length[4] = {1, nx, ny, nz};
  hid_t memS = H5Screate_simple( 3, &dims[1], NULL );
  if( memS < 0 ) { ERR; }
  H5Sselect_hyperslab( tempSel, H5S_SELECT_SET, sel_offset, NULL, sel_length,
                       NULL );
  if( tempSel < 0 ) { ERR; }
  herr_t status = H5Dwrite( tempD, H5T_NATIVE_DOUBLE, memS, tempSel,
                            H5P_DEFAULT, ui );

  // Calculate all timesteps
  for( t = 0; t < STEPS; ++t ) {
    printf( "TIMESTEP = %d\n", t );
    for( i = 1; i < nx-1; ++i ) {
      for( j = 1; j < ny-1; ++j ) {
	for( k = 1; k < nz-1; k++ ) {
          u[i*ny*nz+j*nz+k] = ui[i*ny*nz+j*nz+k] + A*dt*( (ui[(i+1)*ny*nz+j*nz+k] - 2*ui[i*ny*nz+j*nz+k] + ui[(i-1)*ny*nz+j*nz+k])/dx2 + 
                                                          (ui[i*ny*nz+(j+1)*nz+k] - 2*ui[i*ny*nz+j*nz+k] + ui[i*ny*nz+(j-1)*nz+k])/dy2 +
                                                          (ui[i*ny*nz+j*nz+k+1]   - 2*ui[i*ny*nz+j*nz+k] + ui[i*ny*nz+j*nz+k-1])/dz2) ;
	}
      }
    }
    memcpy( ui, u, nx*ny*nz*sizeof(double) );

    // Write the temperature data to the file
    if( tempSel < 0 ) { ERR; }
    sel_offset[0] = t+1;
    H5Sselect_hyperslab( tempSel, H5S_SELECT_SET, sel_offset, NULL, sel_length,
                         NULL );
    if( tempSel < 0 ) { ERR; }
    status = H5Dwrite( tempD, H5T_NATIVE_DOUBLE, memS, tempSel, H5P_DEFAULT, ui);
    if( status < 0 ) { ERR; }
     
  }

  if( H5Fclose( file_id ) <  0 ) { ERR; }

  free(u);
  free(ui);
}
