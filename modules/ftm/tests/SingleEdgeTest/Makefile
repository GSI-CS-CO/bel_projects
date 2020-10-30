
CXX = g++
CXXFLAGS = -g -std=c++11 -fPIC -Wall -I../../include -fprofile-arcs -ftest-coverage

LDFLAGS = -Wl,-rpath,/usr/local/lib
LDLIBS = -L../../lib -lstdc++ -lboost_serialization -lboost_graph -lcarpedm -lgcov

STYLE = --style="{BasedOnStyle: Google, ColumnLimit: 180}"

.PHONY: clean format coverage doc

SingleEdgeTest: SingleEdgeGraph.o SingleEdgeTables.o SingleEdgeTest.o SingleEdgeStatus.o

clean:
	rm -f *.o *.gcda *.gcno *.gcov SingleEdgeTest SingleEdgeTest.info

format:
	clang-format-10 $(STYLE) -i SingleEdgeTest.h
	clang-format-10 $(STYLE) -i SingleEdgeTest.cpp
	clang-format-10 $(STYLE) -i SingleEdgeTables.h
	clang-format-10 $(STYLE) -i SingleEdgeTables.cpp
	clang-format-10 $(STYLE) -i SingleEdgeGraph.h
	clang-format-10 $(STYLE) -i SingleEdgeGraph.cpp
	clang-format-10 $(STYLE) -i SingleEdgeStatus.h
	clang-format-10 $(STYLE) -i SingleEdgeStatus.cpp

coverage:
	lcov --zerocounters --directory . --directory ../../ftmx86
	./SingleEdgeTest
	lcov --capture --directory . --directory ../../ftmx86 --output-file SingleEdgeTest.info
	genhtml SingleEdgeTest.info --output-directory coverage/

doc:
	doxygen SingleEdgeTest_doxy.conf
