CXX = g++
CXXFLAGS = -g -std=c++11 -fPIC -Wall

LDFLAGS = -Wl,-rpath,/usr/local/lib
LDLIBS = -lstdc++ -lboost_serialization -lboost_graph

STYLE = --style="{BasedOnStyle: Google, ColumnLimit: 180}"

.PHONY: clean format doc test

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
	for i in *.h   ; do clang-format-10 $(STYLE) -i $$i; done
	for i in *.cpp ; do clang-format-10 $(STYLE) -i $$i; done

doc:
	$(MAKE) -C ../doc/

test:
	$(MAKE) scheduleCompare
	$(MAKE) -C ../test/
