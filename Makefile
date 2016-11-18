# define the C compiler to use
CC = g++

# define any compile-time flags
CFLAGS = -Wall -std=c++14 -pedantic


INCLUDES = -I/usr/include/bullet  -I/usr/include/irrlicht -I/usr/include/bullet/LinearMath


LFLAGS = -L./bullet/lib  -L./Irrlicht/lib 


LIBS = -lIrrlicht -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath 

# define the C++ source files
SRCS = src/main.cpp src/Game.cpp src/EventReceiver.cpp

# define the C++ object files 
#

.PHONY: depend clean

build/game: build/game.o build/main.o build/event.o build/loader.o
	g++ $(CFLAGS) -o build/game build/game.o build/main.o build/event.o $(LFLAGS) $(LIBS) 
build/game.o: src/Game.cpp
	g++ $(CFLAGS) $(INCLUDES) -c src/Game.cpp -o build/game.o
build/main.o: src/main.cpp
	g++ $(CFLAGS) $(INCLUDES) -c src/main.cpp -o build/main.o
build/event.o: src/EventReceiver.cpp
	g++ $(CFLAGS) $(INCLUDES) -c src/EventReceiver.cpp -o build/event.o
build/loader.o: src/Loader.cpp
	g++ $(CFLAGS) $(INCLUDES) -c src/Loader.cpp -o build/loader.o

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)

clean:
	$(RM) build/*.o build/game

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
