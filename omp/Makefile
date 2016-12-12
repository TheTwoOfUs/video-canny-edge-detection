APP = omp
OBJ = omp.o

APP_FEP = omp_fep
OBJ_FEP = omp_fep.o

CC = gcc
CFLAGS = -g -Wall -Wextra -fopenmp
LDFLAGS = -L../libde/ -lde -lm -fopenmp
INCLUDE_DIRS = -I/usr/include/ffmpeg -I../utils

build: $(APP)

$(OBJ): omp.c
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $^ -o $@

$(APP): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

fep: $(APP_FEP)

$(OBJ_FEP): omp.c
	$(CC) $(CFLAGS) -I../ffmpeg -I../utils -c $^ -o $@

$(APP_FEP): omp_fep.o
	$(CC) -L../libde/ -lde_fep -lm -Wl,-rpath=../libraries -fopenmp $^ -o $@

clean:
	rm -rf $(OBJ) $(APP) $(OBJ_FEP) $(APP_FEP) out.mpg
