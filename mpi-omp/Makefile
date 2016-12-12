APP = mpi-omp
OBJ = mpi-omp.o

APP_FEP = mpi-omp_fep
OBJ_FEP = mpi-omp_fep.o

CC = mpicc
CFLAGS = -g -Wall -Wextra -fopenmp
LDFLAGS = -L../libde/ -lde -lm -fopenmp
INCLUDE_DIRS = -I/usr/include/ffmpeg -I../utils

build: $(APP)

$(OBJ): mpi-omp.c
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $^ -o $@

$(APP): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

fep: $(APP_FEP)

$(OBJ_FEP): mpi-omp.c
	$(CC) $(CFLAGS) -I../ffmpeg -I../utils -c $^ -o $@

$(APP_FEP): mpi-omp_fep.o
	$(CC) -L../libde/ -lde_fep -lm -fopenmp -Wl,-rpath=../libraries $^ -o $@

clean:
	rm -rf $(OBJ) $(APP) $(OBJ_FEP) $(APP_FEP) out.mpg
