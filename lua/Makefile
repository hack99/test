#Source file

SRC = test_lua.cpp
#Object file
OBJ = $(SRC:.cpp=.o)

#Output execution file
PROGRAM = test_lua
PROGRAM_EXT = bin

#Compiler
CC = g++

#Boost
BOOST = /localview/vrgm63/tmp/boost_1_44_0
LUA = /localview/vrgm63/luajit
#Include
INCLUDE = -I/usr/include/ncurses -I/usr/include/ -I$(BOOST) -I$(LUA)/include
#Linker Parameter
LINKPARAM = -L$(BOOST)/stage/lib -L$(LUA)/lib -lpthread -lncurses \
	-lboost_thread -lboost_system -lm -lluajit
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

