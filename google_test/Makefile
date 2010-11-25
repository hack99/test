#Source file

SRC = test_gtest.cpp
#Object file
OBJ = $(SRC:.cpp=.o)

#Output execution file
PROGRAM = test_gtest
PROGRAM_EXT = bin

#Compiler
CC = g++

#Boost
BOOST = /localview/vrgm63/tmp/boost_1_44_0
GTEST = /localview/vrgm63/tmp/gtest-1.5.0
#Include
INCLUDE = -I/usr/include/ncurses -I/usr/include/ -I$(BOOST) -I$(GTEST)/include
#Linker Parameter
LINKPARAM = -L$(BOOST)/stage/lib -L$(GTEST)/lib/.libs -lpthread -lncurses \
	-lboost_thread -lboost_system -lgtest -lgtest_main
#Options for development
CFLAGS = -ansi -g -Wall
#Options for release
#CFLAGS = -ansi -O -Wall

all: $(PROGRAM).$(PROGRAM_EXT)

$(PROGRAM).$(PROGRAM_EXT): $(OBJ)
	$(CC) -o $(PROGRAM).$(PROGRAM_EXT) $(LINKPARAM) $(OBJ)  

.SUFFIXES = .cpp

.cpp.o:
	$(CC) $(INCLUDE) $(CFLAGS) -c $<

clean:
	rm *.o

