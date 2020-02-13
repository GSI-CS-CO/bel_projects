###############################################################################
##                                                                           ##
##     Test-makefile for building a simple "halle world" application for     ##
##     the softcoreprocessoer LM32 and for Linux in only one step            ##
##                                                                           ##           
##---------------------------------------------------------------------------##
## File:    New-Make/Makefile                                                ##
## (c):     GSI Helmholtz Centre for Heavy Ion Research GmbH                 ##
## Author:  Ulrich Becker <u.becker@gsi.de>                                  ##
## Date:    22.01.2019                                                       ##
###############################################################################
.PHONY: all
all:
	$(MAKE) -C buildLm32 all
	$(MAKE) -C buildNative all

.PHONY: clean
clean:
	$(MAKE) -C buildLm32 clean
	$(MAKE) -C buildNative clean

.PHONY: rebuild
rebuild:
	$(MAKE) -C buildLm32 rebuild
	$(MAKE) -C buildNative rebuild

# Uploads the LM32 application to SCU
.PHONY: load
load:
	$(MAKE) -C buildLm32 load

# Starts the Linux application
.PHONY: run
run:
	$(MAKE) -C buildNative run

#=================================== EOF ======================================
