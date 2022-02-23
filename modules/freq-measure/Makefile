CXXFLAGS = `pkg-config saftlib dim --cflags` -Wall -g -O2
LDFLAGS  = `pkg-config saftlib dim --libs`   
CXX = g++ --std=c++11

BIN_PROGRAMS = freq-measure freq-monitor

all: $(BIN_PROGRAMS)

clean:
	rm -f $(BIN_PROGRAMS) *.o

