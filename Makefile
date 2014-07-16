CC = gcc
CFLAGS = -lhdf5 -lnetcdf
MODS = heat hdf5tonetcdf

% : %.c
	$(CC) $< -o $@ $(CFLAGS)

all:	$(MODS)

run:
	./heat
	./hdf5tonetcdf 0 25

run_h5:
	python heat.h5.py data.h5 1 50

run_net:
	python heat.nc.py data.nc

clean:
	rm $(MODS)

mrproper: clean
	rm data.h5 data.nc
