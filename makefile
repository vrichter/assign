
CC=g++
CC_FLAGS= -g
PROG=assign
SRC=hungarian/hungarian.cpp assign.cpp

$(PROG) : $(SRC)
		$(CC) $(CC_FLAGS) -o $(PROG) $(SRC)

clean : $(PROG)
		rm $(PROG)

all : $(PROG)
