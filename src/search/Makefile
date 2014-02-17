CC = g++ -std=c++0x
CFLAGS = -Wall
LINKER = g++ 
CCOPTS = -O3
SRCS = search.cpp porter2_stemmer.cpp
PROG = search 
LIBS = `pkg-config --cflags --libs libxml++-2.6`

all: $(PROG)

$(PROG):	$(SRCS)
		$(CC) $(CFLAGS) $(CCOPTS) -o $(PROG) $(SRCS) $(LIBS)

clean:
	    rm -f *.o $(PROG)
