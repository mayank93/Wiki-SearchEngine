CC = g++ -std=c++0x
CFLAGS = -Wall
LINKER = g++ 
CCOPTS = -O3
SRCS = parse.cpp porter2_stemmer.cpp
PROG = parse 
LIBS = `pkg-config --cflags --libs libxml++-2.6`

all: $(PROG)

$(PROG):	$(SRCS)
		$(CC) $(CFLAGS) $(CCOPTS) -o $(PROG) $(SRCS) $(LIBS)

clean:
	    rm -f *.o $(PROG)
