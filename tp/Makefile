#Source file

SRC = test_tp.cpp
#Object file
OBJ = $(SRC:.cpp=.o)

#Output execution file
PROGRAM = test_tp

#Compiler
CC = g++

#Boost
BOOST = /localview/vrgm63/tmp/boost_1_44_0
#Include
INCLUDE = -I/usr/include/ncurses -I/usr/include/ -I$(BOOST)
#Linker Parameter
LINKPARAM = -L$(BOOST)/stage/lib -lpthread -lncurses -lboost_thread
#Options for development
CFLAGS = -ansi -g -Wall
#Options for release
#CFLAGS = -ansi -O -Wall

all: $(PROGRAM)

$(PROGRAM): $(OBJ)
	$(CC) -o $(PROGRAM) $(LINKPARAM) $(OBJ)  

.SUFFIXES = .cpp

.cpp.o:
	$(CC) $(INCLUDE) $(CFLAGS) -c $<

clean:
	rm *.o

