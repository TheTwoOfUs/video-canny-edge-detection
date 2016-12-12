APP = serial
OBJ = serial.o

APP_FEP = serial_fep
OBJ_FEP = serial_fep.o

CC = gcc
CFLAGS = -g -Wall -Wextra
LDFLAGS = -L../libde/ -lde -lm
INCLUDE_DIRS = -I/usr/include/ffmpeg -I../utils

build: $(APP)

$(OBJ): serial.c
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $^ -o $@

$(APP): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

fep: $(APP_FEP)

$(OBJ_FEP): serial.c
	$(CC) $(CFLAGS) -I../ffmpeg -I../utils -c $^ -o $@

$(APP_FEP): serial_fep.o
	$(CC) -L../libde/ -lde_fep -lm -Wl,-rpath=../libraries $^ -o $@

clean:
	rm -rf $(OBJ) $(APP) $(OBJ_FEP) $(APP_FEP) out.mpg
