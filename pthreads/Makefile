APP = pthreads
OBJ = pthreads.o

APP_FEP = pthreads_fep
OBJ_FEP = pthreads_fep.o

CC = gcc
CFLAGS = -g -Wall -Wextra
LDFLAGS = -L../libde/ -lde -lm -lpthread
INCLUDE_DIRS = -I/usr/include/ffmpeg -I../utils

build: $(APP)

$(OBJ): pthreads.c
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $^ -o $@

$(APP): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

fep: $(APP_FEP)

$(OBJ_FEP): pthreads.c
	$(CC) $(CFLAGS) -I../ffmpeg -I../utils -c $^ -o $@

$(APP_FEP): pthreads_fep.o
	$(CC) -L../libde/ -lde_fep -lm -lpthread -Wl,-rpath=../libraries $^ -o $@

clean:
	rm -rf $(OBJ) $(APP) $(OBJ_FEP) $(APP_FEP) out.mpg
