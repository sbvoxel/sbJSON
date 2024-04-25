SBJSON_OBJ = sbJSON.o
UTILS_OBJ = sbJSON_Utils.o
SBJSON_LIBNAME = libcjson
UTILS_LIBNAME = libcjson_utils
SBJSON_TEST = sbJSON_test

SBJSON_TEST_SRC = sbJSON.c test.c

LDLIBS = -lm

LIBVERSION = 1.7.17
SBJSON_SOVERSION = 1
UTILS_SOVERSION = 1

SBJSON_SO_LDFLAG=-Wl,-soname=$(SBJSON_LIBNAME).so.$(SBJSON_SOVERSION)
UTILS_SO_LDFLAG=-Wl,-soname=$(UTILS_LIBNAME).so.$(UTILS_SOVERSION)

PREFIX ?= /usr/local
INCLUDE_PATH ?= include/cjson
LIBRARY_PATH ?= lib

INSTALL_INCLUDE_PATH = $(DESTDIR)$(PREFIX)/$(INCLUDE_PATH)
INSTALL_LIBRARY_PATH = $(DESTDIR)$(PREFIX)/$(LIBRARY_PATH)

INSTALL ?= cp -a

CC = gcc -std=c99

# validate gcc version for use fstack-protector-strong
MIN_GCC_VERSION = "4.9"
GCC_VERSION := "`$(CC) -dumpversion`"
IS_GCC_ABOVE_MIN_VERSION := $(shell expr "$(GCC_VERSION)" ">=" "$(MIN_GCC_VERSION)")
ifeq "$(IS_GCC_ABOVE_MIN_VERSION)" "1"
    CFLAGS += -fstack-protector-strong
else
    CFLAGS += -fstack-protector
endif

PIC_FLAGS = -fPIC
R_CFLAGS = $(PIC_FLAGS) -pedantic -Wall -Werror -Wstrict-prototypes -Wwrite-strings -Wshadow -Winit-self -Wcast-align -Wformat=2 -Wmissing-prototypes -Wstrict-overflow=2 -Wcast-qual -Wc++-compat -Wundef -Wswitch-default -Wconversion $(CFLAGS)

uname := $(shell sh -c 'uname -s 2>/dev/null || echo false')

#library file extensions
SHARED = so
STATIC = a

## create dynamic (shared) library on Darwin (base OS for MacOSX and IOS)
ifeq (Darwin, $(uname))
	SHARED = dylib
	SBJSON_SO_LDFLAG = ""
	UTILS_SO_LDFLAG = ""
endif

#sbJSON library names
SBJSON_SHARED = $(SBJSON_LIBNAME).$(SHARED)
SBJSON_SHARED_VERSION = $(SBJSON_LIBNAME).$(SHARED).$(LIBVERSION)
SBJSON_SHARED_SO = $(SBJSON_LIBNAME).$(SHARED).$(SBJSON_SOVERSION)
SBJSON_STATIC = $(SBJSON_LIBNAME).$(STATIC)

#sbJSON_Utils library names
UTILS_SHARED = $(UTILS_LIBNAME).$(SHARED)
UTILS_SHARED_VERSION = $(UTILS_LIBNAME).$(SHARED).$(LIBVERSION)
UTILS_SHARED_SO = $(UTILS_LIBNAME).$(SHARED).$(UTILS_SOVERSION)
UTILS_STATIC = $(UTILS_LIBNAME).$(STATIC)

SHARED_CMD = $(CC) -shared -o

.PHONY: all shared static tests clean install

all: shared static tests

shared: $(SBJSON_SHARED) $(UTILS_SHARED)

static: $(SBJSON_STATIC) $(UTILS_STATIC)

tests: $(SBJSON_TEST)

test: tests
	./$(SBJSON_TEST)

.c.o:
	$(CC) -c $(R_CFLAGS) $<

#tests
#sbJSON
$(SBJSON_TEST): $(SBJSON_TEST_SRC) sbJSON.h
	$(CC) $(R_CFLAGS) $(SBJSON_TEST_SRC)  -o $@ $(LDLIBS) -I.

#static libraries
#sbJSON
$(SBJSON_STATIC): $(SBJSON_OBJ)
	$(AR) rcs $@ $<
#sbJSON_Utils
$(UTILS_STATIC): $(UTILS_OBJ)
	$(AR) rcs $@ $<

#shared libraries .so.1.0.0
#sbJSON
$(SBJSON_SHARED_VERSION): $(SBJSON_OBJ)
	$(CC) -shared -o $@ $< $(SBJSON_SO_LDFLAG) $(LDFLAGS)
#sbJSON_Utils
$(UTILS_SHARED_VERSION): $(UTILS_OBJ)
	$(CC) -shared -o $@ $< $(SBJSON_OBJ) $(UTILS_SO_LDFLAG) $(LDFLAGS)

#objects
#sbJSON
$(SBJSON_OBJ): sbJSON.c sbJSON.h
#sbJSON_Utils
$(UTILS_OBJ): sbJSON_Utils.c sbJSON_Utils.h sbJSON.h


#links .so -> .so.1 -> .so.1.0.0
#sbJSON
$(SBJSON_SHARED_SO): $(SBJSON_SHARED_VERSION)
	ln -s $(SBJSON_SHARED_VERSION) $(SBJSON_SHARED_SO)
$(SBJSON_SHARED): $(SBJSON_SHARED_SO)
	ln -s $(SBJSON_SHARED_SO) $(SBJSON_SHARED)
#sbJSON_Utils
$(UTILS_SHARED_SO): $(UTILS_SHARED_VERSION)
	ln -s $(UTILS_SHARED_VERSION) $(UTILS_SHARED_SO)
$(UTILS_SHARED): $(UTILS_SHARED_SO)
	ln -s $(UTILS_SHARED_SO) $(UTILS_SHARED)

#install
#sbJSON
install-cjson:
	mkdir -p $(INSTALL_LIBRARY_PATH) $(INSTALL_INCLUDE_PATH)
	$(INSTALL) sbJSON.h $(INSTALL_INCLUDE_PATH)
	$(INSTALL) $(SBJSON_SHARED) $(SBJSON_SHARED_SO) $(SBJSON_SHARED_VERSION) $(INSTALL_LIBRARY_PATH)
#sbJSON_Utils
install-utils: install-cjson
	$(INSTALL) sbJSON_Utils.h $(INSTALL_INCLUDE_PATH)
	$(INSTALL) $(UTILS_SHARED) $(UTILS_SHARED_SO) $(UTILS_SHARED_VERSION) $(INSTALL_LIBRARY_PATH)

install: install-cjson install-utils

#uninstall
#sbJSON
uninstall-cjson: uninstall-utils
	$(RM) $(INSTALL_LIBRARY_PATH)/$(SBJSON_SHARED)
	$(RM) $(INSTALL_LIBRARY_PATH)/$(SBJSON_SHARED_VERSION)
	$(RM) $(INSTALL_LIBRARY_PATH)/$(SBJSON_SHARED_SO)
	$(RM) $(INSTALL_INCLUDE_PATH)/sbJSON.h

#sbJSON_Utils
uninstall-utils:
	$(RM) $(INSTALL_LIBRARY_PATH)/$(UTILS_SHARED)
	$(RM) $(INSTALL_LIBRARY_PATH)/$(UTILS_SHARED_VERSION)
	$(RM) $(INSTALL_LIBRARY_PATH)/$(UTILS_SHARED_SO)
	$(RM) $(INSTALL_INCLUDE_PATH)/sbJSON_Utils.h

remove-dir:
	$(if $(wildcard $(INSTALL_LIBRARY_PATH)/*.*),,rmdir $(INSTALL_LIBRARY_PATH))
	$(if $(wildcard $(INSTALL_INCLUDE_PATH)/*.*),,rmdir $(INSTALL_INCLUDE_PATH))

uninstall: uninstall-utils uninstall-cjson remove-dir

clean:
	$(RM) $(SBJSON_OBJ) $(UTILS_OBJ) #delete object files
	$(RM) $(SBJSON_SHARED) $(SBJSON_SHARED_VERSION) $(SBJSON_SHARED_SO) $(SBJSON_STATIC) #delete sbJSON
	$(RM) $(UTILS_SHARED) $(UTILS_SHARED_VERSION) $(UTILS_SHARED_SO) $(UTILS_STATIC) #delete sbJSON_Utils
	$(RM) $(SBJSON_TEST)  #delete test
