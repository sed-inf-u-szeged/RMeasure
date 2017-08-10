#
# 'make depend' uses makedepend to automatically generate dependencies 
#               (dependencies are added to end of Makefile)
# 'make'        build executable file 'measureTool'
# 'make clean'  removes all .o and executable files
#

# define the C compiler to use
CC = g++

# define any compile-time flags
CFLAGS = -fPIC -g -std=c++0x

LDFLAGS = -shared

# define the CPP source files
RAPLSRCS =  RaplMethod.cpp
TIMERSRCS = TimerMethod.cpp
SCOPESRCS = PicoScopeMethod.cpp PicoScopeModel.cpp
SRCS =  SourceCapability.cpp

ifeq ($(SCOPE), 1)
SRCS += $(SCOPESRCS)
endif

ifeq ($(RAPL), 1)
SRCS += $(RAPLSRCS)
endif

ifeq ($(TIMER), 1)
SRCS += $(TIMERSRCS)
endif

# define the CPP object files
#
# Below we are replacing the suffix .cpp of all words in the macro SRCS
# with the .o suffix
#
OBJS = $(SRCS:.cpp=.o)

# define the library file
MAIN = librmeasure.so

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all:    $(MAIN) 
	@echo librmeasure.so has been created

$(MAIN): $(OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(MAIN) $(OBJS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .cpp file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)
.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
