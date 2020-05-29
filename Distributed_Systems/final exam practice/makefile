BIN_FILES = camera

CC = gcc

CCFLAGS = -Wall -gcc

LDFLAGS = -L$(INSTALL_PATH)/lib/
LDLIBS = -lpthread

all: CFLAGS = $(CCFLAGS)
all: $(BIN_FILES)
.PHONY : all

camera: camera.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

clean:
	rm -f $(BIN_FILES) *.o

.SUFFIXES:
.PHONY: clean