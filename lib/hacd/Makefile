CXX= g++ -std=c++14 -g -O2
LD= g++ -std=c++14
CXXFLAGS= -Wall -pedantic -frounding-math
INC=-isystem ../../include

LFLAGS= -L/usr/lib

OBJS= $(subst .cpp,.o,$(notdir $(wildcard $(SRCDIR)*.cpp)))

VPATH=src/

all: hacd.a

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INC) $< -c -o $@
	
hacd.a:  $(OBJS)
	ar rvs ../hacd.a $(OBJS)
	rm -f $(OBJS)

clean:
	rm -f $(OBJS) ../hacd.a
