CXX = g++
CXXFLAGS = -g -std=c++11 -fPIC -Wall

LDFLAGS = -Wl,-rpath,/usr/local/lib
LDLIBS = -lstdc++ -lboost_serialization -lboost_graph

STYLE = --style="{BasedOnStyle: Google, ColumnLimit: 180}"

.PHONY: clean format doc

scheduleCompare: scheduleCompare.o scheduleIsomorphism.o printSchedule.o parseSchedule.o ScheduleVertex.o ScheduleEdge.o

scheduleCompare.o: scheduleIsomorphism.h scheduleCompare.h

scheduleIsomorphism.o: scheduleIsomorphism.h scheduleCompare.h printSchedule.h parseSchedule.h ScheduleVertex.h ScheduleEdge.h

printSchedule.o: printSchedule.h scheduleCompare.h scheduleIsomorphism.h

parseSchedule.o: parseSchedule.h scheduleCompare.h scheduleIsomorphism.h

ScheduleVertex.o: ScheduleVertex.h

ScheduleEdge.o: ScheduleEdge.h

clean:
	rm -f *.o *.gcda *.gcno *.gcov scheduleCompare

format:
	clang-format-10 $(STYLE) -i scheduleCompare.h
	clang-format-10 $(STYLE) -i scheduleCompare.cpp
	clang-format-10 $(STYLE) -i scheduleIsomorphism.h
	clang-format-10 $(STYLE) -i scheduleIsomorphism.cpp
	clang-format-10 $(STYLE) -i printSchedule.h
	clang-format-10 $(STYLE) -i printSchedule.cpp
	clang-format-10 $(STYLE) -i parseSchedule.h
	clang-format-10 $(STYLE) -i parseSchedule.cpp
	clang-format-10 $(STYLE) -i ScheduleVertex.h
	clang-format-10 $(STYLE) -i ScheduleVertex.cpp
	clang-format-10 $(STYLE) -i ScheduleEdge.h
	clang-format-10 $(STYLE) -i ScheduleEdge.cpp

doc: