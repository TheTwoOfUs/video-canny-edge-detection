APP = mpi
OBJ = mpi.o

APP_FEP = mpi_fep
OBJ_FEP = mpi_fep.o

CC = mpicc
CFLAGS = -g -Wall -Wextra
LDFLAGS = -L../libde/ -lde -lm
INCLUDE_DIRS = -I/usr/include/ffmpeg -I../utils

build: $(APP)

$(OBJ): mpi.c
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $^ -o $@

$(APP): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

fep: $(APP_FEP)

$(OBJ_FEP): mpi.c
	$(CC) $(CFLAGS) -I../ffmpeg -I../utils -c $^ -o $@

$(APP_FEP): mpi_fep.o
	$(CC) -L../libde/ -lde_fep -lm -Wl,-rpath=../libraries $^ -o $@

clean:
	rm -rf $(OBJ) $(APP) $(OBJ_FEP) $(APP_FEP) out.mpg
