CXX= g++ -std=c++14
LD= g++ -std=c++14
CXXFLAGS= -Wall -g -O3 -pedantic
TETFLAGS= -O3 -Wno-unused-result -Wno-unused-but-set-variable 
INC=-I/usr/include/bullet  -I/usr/include/irrlicht -I/usr/include/bullet/LinearMath
SRCDIR=src/
LFLAGS= -L/usr/lib
LIBS= -lIrrlicht -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath

HEADERS= Game.h Loader.h EventReceiver.h Material.h Object.h ObjectCreator.h tetgen/tetgen.h
OBJS= main.o Game.o Loader.o EventReceiver.o Material.o ObjectCreator.o predicates.o tetgen.o
PROG= game
VPATH=src/

all: $(PROG)

%.o: %.cpp $(SRCDIR)$(HEADERS)
	$(CXX) $(CXXFLAGS) $(INC) $< -c -o $@
	
$(PROG):  $(TETGEN) $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROG)	

tetgen.o:	src/tetgen/tetgen.cxx
	$(CXX) $(TETFLAGS) -c src/tetgen/tetgen.cxx -o tetgen.o

predicates.o: src/tetgen/predicates.cxx
	$(CXX) $(TETFLAGS) -c src/tetgen/predicates.cxx -o predicates.o
    
clean:
	rm -f $(OBJS) $(PROG)

