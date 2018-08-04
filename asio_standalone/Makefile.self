# Source file
SRC = test_self.cpp

# Object file
OBJ = $(SRC:.cpp=.o)

# Output execution file
PROGRAM = test_self
PROGRAM_EXT = bin

# Compiler
CC = g++

# Asio
ASIO = /home/tianyang/Project/tmp/asio-1.12.1/include

# Include
INCLUDE = -I/usr/include/ncurses -I/usr/include/ -I$(ASIO)

# Linker Parameter
LINKPARAM = -pthread -lncurses

# Options for development
CFLAGS = -ansi -g -Wall -std=c++11 -DASIO_STANDALONE
# Options for release
#CFLAGS = -ansi -O -Wall -std=c++11 -DASIO_STANDALONE

all: $(PROGRAM).$(PROGRAM_EXT)

$(PROGRAM).$(PROGRAM_EXT): $(OBJ)
	$(CC) -o $(PROGRAM).$(PROGRAM_EXT) $(LINKPARAM) $(OBJ)  

.SUFFIXES = .cpp

.cpp.o:
	$(CC) $(INCLUDE) $(CFLAGS) -c $<

clean:
	rm *.o

