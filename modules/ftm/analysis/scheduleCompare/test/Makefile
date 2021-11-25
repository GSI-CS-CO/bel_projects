# Makefile for tests of scheduleCompare

BINARY_SCHEDULECOMPARE ?=../main/scheduleCompare
# folders to ignore (no tests inside)
IGNORE =--ignore=dot_hex/  --ignore=dot1/  --ignore=dot/ --ignore=dot_block/ \
  --ignore=dot_boolean/ --ignore=dot_flow/ --ignore=dot_flush/ \
  --ignore=dot_graph_entries/ --ignore=dot_graph_entries_2/ \
  --ignore=dot_switch/ --ignore=dot_tmsg/ --ignore=dot_wait/ \
  --ignore=schedules/ --ignore=permutations/ --ignore=protocol/
# pass the command line arguments to pytest
# Example: OPTIONS="-rP"       show stdout for all tests.
# Example: OPTIONS="-k <test name pattern>" run only tests which match the pattern in the test name.
# Example: OPTIONS='-k "not folder"' do not run all tests with "folder" in the name. This is useful during development since the 'folder-test' are time consuming.
OPTIONS ?=

test:
	TEST_BINARY_SCHEDULECOMPARE=$(BINARY_SCHEDULECOMPARE) python3 -m pytest $(IGNORE) $(OPTIONS)

fast:
	TEST_BINARY_SCHEDULECOMPARE=$(BINARY_SCHEDULECOMPARE) python3 -m pytest --stepwise $(IGNORE) $(OPTIONS)
	
collect:
	python3 -m pytest --collect-only

.PHONY: test fast collect
