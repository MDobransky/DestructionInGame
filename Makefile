CXX= g++ -std=c++14 -g -O2
LD= g++ -std=c++14
CXXFLAGS= -Wall -pedantic -frounding-math
INC=-isystem /usr/include/bullet  -isystem /usr/include/irrlicht -isystem /usr/include/bullet/LinearMath -isystem include
SRCDIR=src/
BUILDDIR=build/
LDFLAGS= -L/usr/lib
LIBS= -lIrrlicht -lBulletSoftBody -lBulletDynamics -lBulletCollision \
    -lLinearMath -lvoro++ -lgmp -lCGAL -lCGAL_Core -lmpfr -lpthread

HEADERS= $(notdir $(wildcard $(SRCDIR)*.h))
OBJS= $(addprefix $(BUILDDIR), $(subst .cpp,.o,$(notdir $(wildcard $(SRCDIR)*.cpp))))
PROG= $(BUILDDIR)game
VPATH=src/

all: $(PROG)

$(BUILDDIR)%.o: %.cpp $(SRCDIR)$(HEADERS) | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INC) $< -c -o $@
	
$(PROG): lib/hacd.a  $(OBJS) | $(BUILDDIR)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) lib/hacd.a -o $(PROG)

lib/hacd.a:
	make -C lib/hacd

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -f $(OBJS) $(PROG)
	make -C lib/hacd clean
