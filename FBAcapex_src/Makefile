# Comilation file for RPI
PROG_NAME = ../fbacapex
OBJS = capex.o
LDFLAGS = -g

CC = gcc
CXX = g++
STRIP = strip

# output from sdl-config
SDL_FLAGS=-I/usr/include/SDL -I/usr/include/glib-2.0 -I/usr/lib/arm-linux-gnueabihf/glib-2.0/include 
#SDL_LIBS=-L/opt/open2x/gcc-4.1.1-glibc-2.3.6/lib -Wl,-rpath,/opt/open2x/gcc-4.1.1-glibc-2.3.6/lib -lSDL -lpthread -lm -ldl
SDL_LIBS=-lSDL -lm -ldl -lglib-2.0

CFLAGS = $(SDL_FLAGS) -O2 -funroll-loops -Wextra -Werror
CXXFLAGS = $(SDL_FLAGS) -O2 -funroll-loops -Wall
LIBS = $(SDL_LIBS)

TARGET = $(PROG_NAME)

all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CXX) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
	$(STRIP) $(TARGET)

clean:
	/bin/rm -rf *~ *.o $(TARGET)
