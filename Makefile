# define the C compiler to use
CC = g++

# define any compile-time flags
CFLAGS = -Wall -std=c++14 -pedantic -g


INCLUDES = -I/usr/include/bullet  -I/usr/include/irrlicht -I/usr/include/bullet/LinearMath


LFLAGS = -L/usr/lib


LIBS = -lIrrlicht -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath

# define the C++ source files
SRCS = src/main.cpp src/Game.cpp src/EventReceiver.cpp src/CollisionResolver.cpp src/ObjectCreator.cpp

# define the C++ object files 
#

.PHONY: depend clean

all: build/tetgen/tetlib build/game

build/tetgen/tetlib: src/tetgen/tetgen.cxx build/tetgen/predicates.o
	g++ -c -o2  src/tetgen/tetgen.cxx -o build/tetgen/tetgen.o
	ar r build/tetgen/libtet.a build/tetgen/tetgen.o build/tetgen/predicates.o
build/tetgen/predicates.o: src/tetgen/predicates.cxx
	g++ -c -o2 src/tetgen/predicates.cxx -o build/tetgen/predicates.o
build/game: build/game.o build/main.o build/event.o build/loader.o build/creator.o build/material.o
	g++ $(CFLAGS) -o build/game build/*.o $(LFLAGS) $(LIBS) -Lbuild/tetgen -ltet
build/game.o: src/Game.cpp
	g++ $(CFLAGS) $(INCLUDES) -c src/Game.cpp -o build/game.o
build/main.o: src/main.cpp
	g++ $(CFLAGS) $(INCLUDES) -c src/main.cpp -o build/main.o
build/event.o: src/EventReceiver.cpp
	g++ $(CFLAGS) $(INCLUDES) -c src/EventReceiver.cpp -o build/event.o
build/loader.o: src/Loader.cpp
	g++ $(CFLAGS) $(INCLUDES) -c src/Loader.cpp -o build/loader.o
build/creator.o: src/ObjectCreator.cpp src/Material.h
	g++ $(CFLAGS) $(INCLUDES) -c src/ObjectCreator.cpp -o build/creator.o
build/material.o: src/Material.cpp
	g++ $(CFLAGS) $(INCLUDES) -c src/Material.cpp -o build/material.o
#build/collisionresolver.o: src/CollisionResolver.cpp
#	g++ $(CFLAGS) $(INCLUDES) -c src/CollisionResolver.cpp -o build/collisionresolver.o


clean:
	$(RM) build/*.o build/game build/tetgen/*

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
