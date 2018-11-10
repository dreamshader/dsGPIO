SHELL := /bin/bash
#################################################################################
# Makefile for dsGPIO - generic access to the GPIOs of a Raspberry Pi
#
#################################################################################
# This Makefile is part of my DS3234 project
#// // // // #	git://github.com/dreamshader/DS3234.git
#
#  Copyright (C) 2018 Dreamshader (aka Dirk Schanz)
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#  
#      http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#################################################################################
CXX = g++
CC  = gcc
AR  = ar
RANLIB  = ranlib
LDCONFIG?=ldconfig
#
CCDEBUG  = -g
CXXDEBUG = -g
#
CXXFLAGS = -Wall
CXXLIBSOFLAGS = -fPIC -shared 
#
# CXXEXTRAFLAGS = -DLINUX
# CXXEXTRAFLAGS = -DLINUX -DDEBUG -DDEBUG_STATUS_BITS
# CXXEXTRAFLAGS = -DRASPBERRY
# CXXEXTRAFLAGS = -DRASPBERRY -DDEBUG
#
SOURCEDIR = ../src
#
SOLIBNAME = libdsGPIO.so
STATLIBNAME = libdsGPIO.a
#
LIB_SRC = $(SOURCEDIR)/dsGPIO.c

SRC_INC = $(SOURCEDIR)/dsGPIO.h

LIB_OBJ = dsGPIO.o

EXAMPLE_SRC = $(SOURCEDIR)/gpioTest.c

EXAMPLE_NAME = gpioTest

BUILD_FLAGS = -I. -L ../build
#
#
EXTRALIBS = -lrt -lpthread
CXXEXTRAFLAGS = -DLINUX -DDEBUG -DDEBUG_STATUS_BITS -DRASPBERRY
#

#
all: $(STATLIBNAME) $(SOLIBNAME) $(EXAMPLE_NAME)
#all: $(STATLIBNAME) $(SOLIBNAME)


$(STATLIBNAME): $(LIB_SRC) $(SRC_INC)
	$(CXX) $(CXXFLAGS) $(CXXEXTRAFLAGS) -c $(LIB_SRC)
	$(AR) rcs $(STATLIBNAME)  $(LIB_OBJ)
	$(RANLIB) $(STATLIBNAME)

$(SOLIBNAME): $(LIB_SRC) $(SRC_INC)
	$(CXX) $(CXXFLAGS) $(CXXEXTRAFLAGS) $(CXXDEBUG) $(CXXLIBSOFLAGS) -c $(LIB_SRC)
	$(CXX) -shared  -Wl,-soname,$(SOLIBNAME) -o $(SOLIBNAME) $(LIB_OBJ)


$(EXAMPLE_NAME): $(EXAMPLE_SRC) $(LIB_SRC) $(SRC_INC)
	$(CXX) -o $(EXAMPLE_NAME) $(CXXDEBUG) $(CXXEXTRAFLAGS) $(EXAMPLE_SRC) $(LIB_SRC) $(BUILD_FLAGS) ${EXTRALIBS}


install: $(STATLIBNAME) $(SOLIBNAME)
	sudo install -m 0755 -d                     /usr/local/include
	sudo install -m 0644 $(SOURCEDIR)/dsGPIO.h  /usr/local/include
	sudo install -m 0755 -d                     /usr/local/lib
	sudo install -m 0644 libdsGPIO.a            /usr/local/lib
	sudo install -m 0644 libdsGPIO.so           /usr/local/lib
	$(LDCONFIG)

uninstall:
	sudo rm -f /usr/local/include/dsGPIO.h
	sudo rm -f /usr/local/lib/libdsGPIO.a
	sudo rm -f /usr/local/lib/libdsGPIO.so
	$(LDCONFIG)

clean:
	$rm *.o *.so *.a

