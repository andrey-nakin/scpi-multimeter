VERSION = 0.1.0
LIBNAME = scpimm

CFLAGS += -Wall -Wextra -O1 -g -fPIC -Iinc -I/home/andrey/workspace/scpi-parser/libscpi/inc
LDFLAGS += -Wl,--as-needed -Ldist -L../../scpi-parser/libscpi/dist
TESTFLAGS += -lcunit -lscpimm -lscpi -lm -lpthread -lrt

OBJDIR=obj
DISTDIR=dist
TESTDIR=test

STATICLIBFLAGS = rcs
SHAREDLIBFLAGS = $(LDFLAGS) -shared -Wl,-soname

STATICLIB = lib$(LIBNAME).a
SHAREDLIB = lib$(LIBNAME).so

SHAREDLIBVER = $(SHAREDLIB).$(VERSION)

SRCS = $(addprefix src/, configure.c display.c dmm.c ieee488.c input.c measure.c sample.c scpimm.c sense.c system.c route.c trigger.c utils.c)

OBJS = $(addprefix $(OBJDIR)/, $(notdir $(SRCS:.c=.o)))

HDRS = $(addprefix inc/scpimm/, scpimm.h) $(addprefix src/, configure.h display.h dmm.h ieee488.h input.h measure.h sample.h sense.h system.h route.h trigger.h utils.h)

#TESTS = $(addprefix test/, test_generic.c test_configure.c test_display.c test_dmm.c test_input.c test_sample.c test_sense.c test_system.c test_route.c test_trigger.c)
TESTS = $(addprefix test/, test_dmm.c)

TESTS_OBJS = $(TESTS:.c=.o)
TESTS_BINS = $(TESTS_OBJS:.o=.test)

.PHONY: all clean static shared test

all: static shared

$(OBJDIR):
	mkdir -p $@

$(DISTDIR):
	mkdir -p $@

$(OBJDIR)/%.o: src/%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

$(DISTDIR)/$(STATICLIB): $(OBJS)
	$(AR) $(STATICLIBFLAGS) $(DISTDIR)/$(STATICLIB) $(OBJS)

$(DISTDIR)/$(SHAREDLIBVER): $(OBJS)
	$(CC) $(SHAREDLIBFLAGS),$(SHAREDLIB) -o $(DISTDIR)/$(SHAREDLIBVER) $(OBJS)

$(DISTDIR)/$(SHAREDLIB): $(DISTDIR)/$(SHAREDLIBVER)
	$(RM) $(DISTDIR)/$(SHAREDLIB)
	ln -s $(SHAREDLIBVER) $(DISTDIR)/$(SHAREDLIB)

static: $(DISTDIR)/$(STATICLIB)

shared: $(DISTDIR)/$(SHAREDLIB)

$(OBJS): $(HDRS) $(DISTDIR) $(OBJDIR)

clean:
	$(RM) -r $(OBJDIR) $(DISTDIR) $(TESTS_BINS) $(TESTS_OBJS)

test: static $(TESTS_BINS)
	for t in $(TESTS_BINS); do ./$$t; done

#$(TESTDIR)/%.o: $(TESTDIR)/%.c
#	gcc -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

#$(TESTDIR)/%.test: $(TESTDIR)/%.o
#	gcc $(TESTFLAGS) $< $(DISTDIR)/$(STATICLIB) -o $@ $(LDFLAGS)

#$(TESTDIR)/%.o: $(TESTDIR)/%.c
#	gcc -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

$(TESTDIR)/%.test: $(TESTDIR)/%.c test/test_utils.c test/test_utils.h test/default_multimeter.c test/default_multimeter.h
	gcc --static $(CFLAGS) $(CPPFLAGS) test/test_utils.c test/default_multimeter.c -Ldist -L../../scpi-parser/libscpi/dist $< -o $@ $(TESTFLAGS) 

# $(LDFLAGS)

