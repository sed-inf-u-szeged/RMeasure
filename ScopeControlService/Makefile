#
# 'make depend' uses makedepend to automatically generate dependencies
#               (dependencies are added to end of Makefile)
# 'make'        build executable file 'measureTool'
# 'make clean'  removes all .o and executable files
#

# define the C compiler to use
CC = g++

# define any compile-time flags
CFLAGS = -Wall -g -std=c++0x

# define any directories containing header files other than /usr/include
#
INCLUDES = -I/opt/picoscope/include/libps4000a-1.0/

# define library paths in addition to /usr/lib
LFLAGS = -L/usr/local/lib/ -L/opt/picoscope/lib/

# define any libraries to link into executable:
LIBS = -lconfig++ -lxmlrpc_server++ -lxmlrpc_server_abyss++ -lps4000a

# define the CPP source files
SRCS = MeasurementData.cpp Channel.cpp PicoScope.cpp ScopeControlServer.cpp main.cpp

# define the CPP object files
#
# Below we are replacing the suffix .cpp of all words in the macro SRCS
# with the .o suffix
#
OBJS = $(SRCS:.cpp=.o)

# define the executable file
MAIN = scopeControlService

#
# The following part of the makefile is generic; it can be used to
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all:    $(MAIN)
	@echo scopeControlService has been compiled

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

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
