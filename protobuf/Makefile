#Source file

WRITE_SRC = test_write.cpp addressbook.pb.cpp
READ_SRC = test_read.cpp addressbook.pb.cpp
#Object file
WRITE_OBJ = $(WRITE_SRC:.cpp=.o)
READ_OBJ = $(READ_SRC:.cpp=.o)

#Output execution file
WRITE_PROGRAM = test_write
READ_PROGRAM = test_read
PROGRAM_EXT = bin

#Compiler
CC = g++

#Protobuf
PROTOBUF = /localview/vrgm63/tmp/protobuf-2.3.0/src
#Boost
BOOST = /localview/vrgm63/tmp/boost_1_44_0
#Include
INCLUDE = -I/usr/include/ncurses -I/usr/include/ -I$(BOOST) -I$(PROTOBUF)
#Linker Parameter
LINKPARAM = -L$(BOOST)/stage/lib -L$(PROTOBUF)/.libs -lpthread -lncurses \
	-lboost_thread -lboost_system -lprotobuf 
#Options for development
CFLAGS = -ansi -g -Wall
#Options for release
#CFLAGS = -ansi -O -Wall

all: $(WRITE_PROGRAM).$(PROGRAM_EXT) $(READ_PROGRAM).$(PROGRAM_EXT)

$(WRITE_PROGRAM).$(PROGRAM_EXT): $(WRITE_OBJ)
	$(CC) -o $(WRITE_PROGRAM).$(PROGRAM_EXT) $(LINKPARAM) $(WRITE_OBJ)  

$(READ_PROGRAM).$(PROGRAM_EXT): $(READ_OBJ)
	$(CC) -o $(READ_PROGRAM).$(PROGRAM_EXT) $(LINKPARAM) $(READ_OBJ)  

.SUFFIXES = .cpp

.cpp.o:
	$(CC) $(INCLUDE) $(CFLAGS) -c $<

clean:
	rm *.o

