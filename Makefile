
DEBUG_BUILD := no

CC := gcc
CFLAGS := \
 -Wall \
 -I/usr/local/include \
 -march=native -mtune=native

LDFLAGS := -L/usr/local/lib -lwiringPi -lm
OUT_BIN := plotter-cdrom

ifeq ("$(DEBUG_BUILD)","yes")
	CFLAGS += -O0 -g -ggdb
else
	CFLAGS += -pipe -O2 -fomit-frame-pointer
endif

all:
	$(CC) -c $(CFLAGS) plotter-cdrom.c -o plotter-cdrom.o
	$(CC) $(CFLAGS) plotter-cdrom.o $(LDFLAGS) -o $(OUT_BIN)

	if [ -w plotter-cdrom.o ] ; then rm plotter-cdrom.o; fi

