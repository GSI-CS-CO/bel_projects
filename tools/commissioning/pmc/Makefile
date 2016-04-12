SRCDIR=src
BINDIR=bin

SRCS=$(wildcard $(SRCDIR)/*.cpp)
BINS=$(patsubst $(SRCDIR)/%.cpp, %, $(SRCS))

CPPFLAGS= -Wall -g
EB    = /usr/local/lib
LIBPATH = -L$(EB) -Wl,-rpath,$(EB)
LIBS  = -letherbone -lrt

all: $(BINS)

$(BINS): $(BINDIR)
	g++ $(CPPFLAGS) $(LIBPATH) $(SRCDIR)/$@.cpp $(LIBS) -o $(BINDIR)/$@

$(BINDIR):
	mkdir bin

clean:
	rm -r ./bin	
	
