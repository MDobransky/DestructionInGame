CXX= g++ -std=c++14 -g -o2
LD= g++ -std=c++14
CXXFLAGS= -Wall -pedantic -frounding-math
INC=-isystem ../../include

BUILDDIR=build/
LFLAGS= -L/usr/lib

OBJS= $(subst .cpp,.o,$(notdir $(wildcard $(SRCDIR)*.cpp)))

VPATH=src/

all: hacd.a

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INC) $< -c -o $@
	
hacd.a:  $(OBJS)
	ar rvs ../../lib/hacd.a $(OBJS)

clean:
	rm -f $(OBJS) ../../lib/hacd.a

