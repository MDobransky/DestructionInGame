# define the C compiler to use
CC = g++

# define any compile-time flags
CFLAGS = -Wall


INCLUDES = -I/usr/include/bullet  -I/usr/include/irrlicht -I/usr/include/bullet/LinearMath


LFLAGS = -L./bullet/lib  -L./Irrlicht/lib 


LIBS = -lIrrlicht -lGL -lXxf86vm -lX11 -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath 

# define the C++ source files
SRCS = main.cpp

# define the C++ object files 
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#




.PHONY: depend clean

game: game.o
	g++ $(CFLAGS) -o game main.o $(LFLAGS) $(LIBS) 

game.o: main.cpp
	g++ $(CFLAGS) $(INCLUDES) -c main.cpp 

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)

clean:
	$(RM) *.o *~ game

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
