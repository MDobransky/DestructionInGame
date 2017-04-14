CXX= g++ -std=c++14
LD= g++ -std=c++14
CXXFLAGS= -Wall -O3 -pedantic
TETFLAGS= -O3 -Wno-unused-result -Wno-unused-but-set-variable 
INC=-I/usr/include/bullet  -I/usr/include/irrlicht -I/usr/include/bullet/LinearMath
SRCDIR=src/
BUILDDIR=build/
LFLAGS= -L/usr/lib
LIBS= -lIrrlicht -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath

HEADERS= Game.h Loader.h EventReceiver.h Material.h Object.h ObjectCreator.h tetgen/tetgen.h
OBJS= $(addprefix $(BUILDDIR), $(subst .cpp,.o,$(notdir $(wildcard $(SRCDIR)*.cpp))) predicates.o tetgen.o)
PROG= $(BUILDDIR)game
VPATH=src/

all: $(PROG)

$(BUILDDIR)%.o: %.cpp $(SRCDIR)$(HEADERS)
	$(CXX) $(CXXFLAGS) $(INC) $< -c -o $@
	
$(BUILDDIR)%.o: tetgen/%.cxx $(SRCDIR)$(HEADERS)
	$(CXX) $(TETFLAGS) $(INC) $< -c -o $@
	
$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROG)

clean:
	rm -f $(OBJS) $(PROG)

