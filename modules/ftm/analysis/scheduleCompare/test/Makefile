# Makefile for tests of scheduleCompare

BINARY_SCHEDULECOMPARE ?=../main/scheduleCompare
# folders to ignore (no tests inside)
IGNORE = --ignore=dot*/
# pass the command line arguments to pytest
# Example: OPTIONS="-rP"       show stdout for all tests.
# Example: OPTIONS="-k <test name pattern>" run only tests which match the pattern in the test name.
OPTIONS ?=

test:
	TEST_BINARY_SCHEDULECOMPARE=$(BINARY_SCHEDULECOMPARE) python3 -m pytest $(IGNORE) $(OPTIONS)

fast:
	TEST_BINARY_SCHEDULECOMPARE=$(BINARY_SCHEDULECOMPARE) python3 -m pytest --stepwise $(IGNORE) $(OPTIONS)

.PHONY: test fast
