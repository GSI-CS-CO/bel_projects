################################################################################
##                                                                            ##
##            Makefile for  building a simple QT-application                  ##
##                                                                            ##
##----------------------------------------------------------------------------##
## File:   bel_projects/modules/linuxTest/qt_test/Makefile                    ##
## Author: Ulrich Becker                                                      ##                 
## (C)     2020 GSI Helmholtz Centre for Heavy Ion Research GmbH              ##
## Date:   25.08.2020                                                         ##
################################################################################
REPOSITORY_DIR = $(shell git rev-parse --show-toplevel)

MIAN_MODULE := qttest.cpp
SOURCE += simpleForm.cpp


UI_PREFIX  = $(GENERATED_DIR)/ui_
MOC_PREFIX = $(GENERATED_DIR)/moc_

SOURCE += $(MOC_PREFIX)simpleFrom.cpp



QTDIR ?= /usr/include/QtGui/
LIBS += QtGui QtCore
LIBS += stdc++

INCLUDE_DIRS += $(QTDIR)

UIC ?= uic
MOC ?= moc

MOC_MSG := "MOC"
UIC_MSG := "UIC"

ifeq ($(V), 1)
   UIC_F = $(UIC) 
   MOC_F = $(MOC)
else
   UIC_F = $(FORMAT) "gen" $(UIC_MSG) $(@); $(UIC)
   MOC_F = $(FORMAT) "gen" $(MOC_MSG) $(@); $(MOC)
endif


include $(REPOSITORY_DIR)/makefiles/makefile.scun

$(UI_PREFIX)simpleForm.hpp: simpleForm.ui $(GENERATED_DIR)
	$(UIC_F) $< -o $@

$(MOC_PREFIX)simpleFrom.cpp: ./simpleForm.hpp $(UI_PREFIX)simpleForm.hpp
	$(MOC_F) $< -o $@

#=================================== EOF ======================================
