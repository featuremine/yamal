#
# Makefile for libytp
#
# Use config.mak to override any of the following variables.
# Do not make changes here.
#

srcdir = .
prefix = /opt/featuremine/tools
includedir = $(prefix)/include
libdir = $(prefix)/lib
testsdir = $(prefix)/tests
bindir = $(prefix)/bin
pythondir = $(prefix)/python
scriptsdir = $(prefix)/scripts
sourcesdir = $(prefix)/src

CPPFLAGS =
CFLAGS =
CFLAGS_AUTO =
CXXFLAGS =
CXXFLAGS_AUTO =

AR      = $(CROSS_COMPILE)ar
RANLIB  = $(CROSS_COMPILE)ranlib

-include config.mak

CFLAGS_ALL   = -I$(srcdir)/include -I./include $(CPPFLAGS) $(CFLAGS_AUTO) $(CFLAGS)
CXXFLAGS_ALL = -I$(srcdir)/include -I./include $(CPPFLAGS) $(CXXFLAGS_AUTO) $(CXXFLAGS)
LDFLAGS_ALL  = $(LDFLAGS_AUTO) $(LDFLAGS)

INSTALL = install

# syncer/component.cpp
SRCS_YTP   := yamal.cpp peer.cpp channel.cpp time.cpp control.cpp sequence.cpp timeline.cpp
OBJS_YTP   := $(addprefix obj/src/ytp/,$(addsuffix .o,$(SRCS_YTP)))
SRCS_TESTS := compiles.c common.cpp yamal.cpp peer.cpp channel.cpp time.cpp control.cpp
SRCS_TESTS += timeline.cpp sequence.cpp sequence_test.cpp
OBJS_TEST  := $(addprefix obj/tests/,$(addsuffix .o,$(SRCS_TESTS)))
BINS_TEST  := $(patsubst obj/tests/%.o,tests/%.bin,$(OBJS_TEST))
SRCS_TOOLS := yamal_producer.cpp yamal_consumer.cpp ytp_writer.cpp ytp_reader.cpp ytp_perf.cpp
OBJS_TOOLS  := $(addprefix obj/bin/,$(addsuffix .o,$(SRCS_TOOLS)))
BINS_TOOLS  := $(patsubst obj/bin/%.o,bin/%.bin,$(OBJS_TOOLS))
ALL_OBJS   := $(OBJS_YTP) $(OBJS_TEST) $(OBJS_TOOLS)

INCLUDES = $(wildcard $(srcdir)/include/*.h $(srcdir)/include/*/*.h $(srcdir)/include/*/*/*.h $(srcdir)/include/*.hpp $(srcdir)/include/*/*.hpp $(srcdir)/include/*/*/*.hpp)
ALL_INCLUDES = $(sort $(INCLUDES:$(srcdir)/%=%))
SCRIPTS = $(wildcard $(srcdir)/scripts/*.sh $(srcdir)/scripts/*.py)
PYTHON_PY = $(wildcard $(srcdir)/python/*.py $(srcdir)/python/tests/*.py $(srcdir)/python/tools/*.py $(srcdir)/python/tools/*/*.py)
PYTHON_SRC = #$(wildcard $(srcdir)/src/fmc++/python/*)

STATIC_LIBS = lib/libytp.a 
SHARED_LIBS = lib/libytp.so
ALL_LIBS = $(STATIC_LIBS) $(SHARED_LIBS)
PKGCONFIG_LIBS = lib/pkgconfig/libytp.pc

all: $(ALL_LIBS) $(BINS_TOOLS)

test: $(BINS_TEST)
	for t in $(BINS_TEST) ; do \
		echo "Running test $$t" ; \
		$$t ; \
	done

OBJ_DIRS = $(sort $(patsubst %/,%,$(dir $(ALL_OBJS))))
DEP_DIRS = $(patsubst obj/%,obj/d/%,$(OBJ_DIRS))
LIB_DIRS = $(sort $(patsubst %/,%,$(dir $(ALL_LIBS))))
TEST_DIRS = $(sort $(patsubst %/,%,$(dir $(BINS_TEST))))
TOOL_DIRS = $(sort $(patsubst %/,%,$(dir $(BINS_TOOLS))))

$(ALL_OBJS): include/ytp/version.h | $(OBJ_DIRS) $(DEP_DIRS)
$(ALL_LIBS): | $(LIB_DIRS)
$(BINS_TEST): | $(TEST_DIRS)
$(BINS_TOOLS): | $(TOOL_DIRS)

$(OBJ_DIRS) $(DEP_DIRS) $(LIB_DIRS) $(TEST_DIRS) $(TOOL_DIRS) include:
	mkdir -p $@

YTP_VERSION := $(shell cat $(srcdir)/VERSION)
YTP_VERSION_MAJOR := $(word 1,$(subst ., ,$(YTP_VERSION)))
YTP_VERSION_MINOR := $(word 2,$(subst ., ,$(YTP_VERSION)))
YTP_VERSION_PATCH := $(word 3,$(subst ., ,$(YTP_VERSION)))

include/ytp/version.h: $(srcdir)/include/ytp/version.h.in | include
	mkdir -p $(dir $@)
	sed -e 's/@YTP_VERSION_MAJOR@/$(YTP_VERSION_MAJOR)/'  \
	-e 's/@YTP_VERSION_MINOR@/$(YTP_VERSION_MINOR)/'  \
	-e 's/@YTP_VERSION_PATCH@/$(YTP_VERSION_PATCH)/'  \
	-e 's/@YTP_VERSION@/"$(YTP_VERSION)"/'  \
	$< > $@

DEPFLAGS     = -MT $@ -MMD -MP -MF obj/d/$*.Td
POSTCOMPILE  = @mv -f obj/d/$*.Td obj/d/$*.d && touch $@
.SECONDARY: 
.PRECIOUS: obj/d/%.d

CC_CMD = $(CC) $(DEPFLAGS) $(CFLAGS_ALL) -c -o $@ $<
CXX_CMD = $(CXX) $(DEPFLAGS) $(CXXFLAGS_ALL) -c -o $@ $<

obj/%.c.o: $(srcdir)/%.c
	$(CC_CMD)
	$(POSTCOMPILE)

obj/%.cpp.o: $(srcdir)/%.cpp
	$(CXX_CMD)
	$(POSTCOMPILE)

obj/d/%.d: ;
include $(wildcard $(addsuffix /*.d,$(DEP_DIRS)))

lib/libytp.a: $(OBJS_YTP)
	rm -f $@
	$(AR) rc $@ $(OBJS_YTP)
	$(RANLIB) $@

lib/libytp.so: $(OBJS_YTP)
	rm -f $@
	$(CXX) -shared $(OBJS_YTP) -o $@
#TODO: I think is necessary for our toolchain (show with /home/ivan/programs/libtree): -Wl,-rpath='/opt/featuremine/tools/x86_64-linux-musl/lib'

tests/%.c.bin: obj/tests/%.c.o $(STATIC_LIBS)
	$(CXX) $< $(STATIC_LIBS) $(LDFLAGS_ALL) -o $@

tests/%.cpp.bin: obj/tests/%.cpp.o $(STATIC_LIBS)
	$(CXX) $< $(STATIC_LIBS) $(LDFLAGS_ALL) -o $@

bin/%.c.bin: obj/bin/%.c.o $(STATIC_LIBS)
	$(CXX) $< $(STATIC_LIBS) $(LDFLAGS_ALL) -o $@

bin/%.cpp.bin: obj/bin/%.cpp.o $(STATIC_LIBS)
	$(CXX) $< $(STATIC_LIBS) $(LDFLAGS_ALL) -o $@

$(DESTDIR)$(libdir)/%: lib/%
	$(INSTALL) -D -m 644 $< $@

$(DESTDIR)$(includedir)/%: $(srcdir)/include/%
	$(INSTALL) -D -m 644 $< $@

$(DESTDIR)$(includedir)/%: include/%
	$(INSTALL) -D -m 644 $< $@

$(DESTDIR)$(testsdir)/%: tests/%
	$(INSTALL) -D -m 755 $< $@

$(DESTDIR)$(bindir)/%: bin/%
	$(INSTALL) -D -m 755 $< $@

$(DESTDIR)$(pythondir)/%: $(srcdir)/python/%
	$(INSTALL) -D -m 755 $< $@

$(DESTDIR)$(sourcesdir)/%: $(srcdir)/src/%
	$(INSTALL) -D -m 644 $< $@

$(DESTDIR)$(scriptsdir)/%: $(srcdir)/scripts/%
	$(INSTALL) -D -m 755 $< $@

install-libs: $(ALL_LIBS:lib/%=$(DESTDIR)$(libdir)/%)
install-pkgconfig: $(PKGCONFIG_LIBS:lib/pkgconfig/%=$(DESTDIR)$(libdir)/pkgconfig/%)

install-headers: $(ALL_INCLUDES:include/%=$(DESTDIR)$(includedir)/%)
install-headers: $(DESTDIR)$(includedir)/ytp/version.h

ifneq ($(wildcard tests/*),)
install-tests: $(BINS_TEST:tests/%=$(DESTDIR)$(testsdir)/%)
else
install-tests:
endif

install-bin: $(BINS_TOOLS:bin/%=$(DESTDIR)$(bindir)/%)

install-python: $(PYTHON_PY:$(srcdir)/python/%=$(DESTDIR)$(pythondir)/%) $(PYTHON_SRC:$(srcdir)/src/%=$(DESTDIR)$(sourcesdir)/%)

install-scripts: $(SCRIPTS:$(srcdir)/scripts/%=$(DESTDIR)$(scriptsdir)/%)

install: install-libs install-pkgconfig install-headers install-bin install-python install-scripts

clean:
	rm -rf obj lib tests include bin

distclean: clean
	rm -f config.mak

.PHONY: all test clean install install-libs install-pkgconfig install-headers install-tests install-bin install-scripts install-python
