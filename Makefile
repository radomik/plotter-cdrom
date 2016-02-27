
# Set to yes if want to debug program with GDB debugger
DEBUG_BUILD := no

# Set to yes to test compilation not on Raspberry PI
GPIO_MOCK := yes

CC := gcc
CFLAGS := \
 -Wall \
 -I/usr/local/include \
 -march=native -mtune=native

LDFLAGS := -lm
OUT_BIN := plotter-cdrom

ifeq ("$(DEBUG_BUILD)","yes")
	CFLAGS += -O0 -g -ggdb
else
	CFLAGS += -pipe -O2 -fomit-frame-pointer
endif

ifeq ("$(GPIO_MOCK)","yes")
	CFLAGS += -D GPIO_MOCK
else
	LDFLAGS += -L/usr/local/lib -lwiringPi
endif

# for i in *.c; do c=`printf "%16s" $i`; o=`echo $i | sed 's/\.c/\.o/g'`; o=`printf "%16s" $o`; echo -e "\t\$(CC) -c \$(CFLAGS) $c -o $o"; done
# for i in *.c; do o=`echo $i | sed 's/\.c/\.o/g'`; o=`printf "%18s" $o`; echo " $o \\"; done
# for i in *.c; do c=`printf "%16s" $i`; o=`echo $i | sed 's/\.c/\.o/g'`; o=`printf "%16s" $o`; echo -e "\tif [ -w $o ] ; then rm $o; fi"; done

all:
	$(CC) -c $(CFLAGS)         Bitmap.c -o         Bitmap.o
	$(CC) -c $(CFLAGS)     BmpPlotter.c -o     BmpPlotter.o
	$(CC) -c $(CFLAGS)     Controller.c -o     Controller.o
	$(CC) -c $(CFLAGS)           Gpio.c -o           Gpio.o
	$(CC) -c $(CFLAGS)            Log.c -o            Log.o
	$(CC) -c $(CFLAGS)     SvgPlotter.c -o     SvgPlotter.o
	$(CC) -c $(CFLAGS)             Ui.c -o             Ui.o
	$(CC) -c $(CFLAGS)           Vars.c -o           Vars.o
	$(CC) -c $(CFLAGS)  plotter-cdrom.c -o  plotter-cdrom.o


	$(CC) $(CFLAGS) \
           Bitmap.o \
       BmpPlotter.o \
       Controller.o \
             Gpio.o \
              Log.o \
       SvgPlotter.o \
               Ui.o \
             Vars.o \
    plotter-cdrom.o \
$(LDFLAGS) -o $(OUT_BIN)

	if [ -w         Bitmap.o ] ; then rm         Bitmap.o; fi
	if [ -w     BmpPlotter.o ] ; then rm     BmpPlotter.o; fi
	if [ -w     Controller.o ] ; then rm     Controller.o; fi
	if [ -w           Gpio.o ] ; then rm           Gpio.o; fi
	if [ -w            Log.o ] ; then rm            Log.o; fi
	if [ -w     SvgPlotter.o ] ; then rm     SvgPlotter.o; fi
	if [ -w             Ui.o ] ; then rm             Ui.o; fi
	if [ -w           Vars.o ] ; then rm           Vars.o; fi
	if [ -w  plotter-cdrom.o ] ; then rm  plotter-cdrom.o; fi


