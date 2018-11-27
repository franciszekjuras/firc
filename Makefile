IDIR =include
CC=gcc
CFLAGS=-I$(IDIR) -std=c11

ODIR=obj
#LDIR =../lib

LIBS=-lm

_DEPS = firctrl.h utils.h fir.h fpga.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o utils.o fir.o fpga.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

firctrl: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~ firctrl
