#
# File: Makefile
# General makefile. Invokes all sub-makefiles in the folders.
#
# Date: 29.01.2019
# Author: Ulrich Becker
#

.PHONY: all                          
all:
	@(for i in $$(ls); do \
	     [ -d "$$i" ] && [ -f "$$i/Makefile" ] && make -C $$i all; \
	     done;)

.PHONY: clean
clean:
	@(for i in $$(ls); do \
	     [ -d "$$i" ] && [ -f "$$i/Makefile" ] && $(MAKE) -C $$i clean; \
	     done;)

#=================================== EOF ======================================