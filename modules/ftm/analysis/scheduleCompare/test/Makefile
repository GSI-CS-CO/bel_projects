# Makefile for tests of scheduleCompare

# folders to ignore (no tests inside)
IGNORE = --ignore=dot*/
# pass the command line arguments to pytest
OPTIONS ?=

test:
	TEST_BINARY_SCHEDULECOMPARE=../main/scheduleCompare python3 -m pytest $(IGNORE) $(OPTIONS)

fast:
	TEST_BINARY_SCHEDULECOMPARE=../main/scheduleCompare python3 -m pytest --failfast $(IGNORE) $(OPTIONS)

.PHONY: test fast
