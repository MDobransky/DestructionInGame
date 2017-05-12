CXX= g++ -std=c++14 -g
LD= g++ -std=c++14
CXXFLAGS= -Wall -O3 -pedantic
INC=-I/usr/include/bullet  -I/usr/include/irrlicht -I/usr/include/bullet/LinearMath
SRCDIR=src/
BUILDDIR=build/
LFLAGS= -L/usr/lib
LIBS= -lIrrlicht -lBulletSoftBody -lBulletDynamics -lBulletCollision \
    -lLinearMath -lvoro++ -lgmp -lCGAL -lCGAL_Core -lmpfr

HEADERS= $(notdir $(wildcard $(SRCDIR)*.h))
OBJS= $(addprefix $(BUILDDIR), $(subst .cpp,.o,$(notdir $(wildcard $(SRCDIR)*.cpp))))
PROG= $(BUILDDIR)game
VPATH=src/

all: $(PROG)

$(BUILDDIR)%.o: %.cpp $(SRCDIR)$(HEADERS) | $(BUILDDIR)
	@$(CXX) $(CXXFLAGS) $(INC) $< -c -o $@
	$(info $@ created)
	
$(PROG):  $(OBJS) | $(BUILDDIR)
	@$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROG)
	$(info $@ created)

$(BUILDDIR):
	@mkdir -p $(BUILDDIR)
	$(info $@ created)

clean:
	@rm -f $(OBJS) $(PROG)
	$(info Project cleaned)
