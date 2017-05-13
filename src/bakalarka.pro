TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    CollisionResolver.cpp \
    EventReceiver.cpp \
    Game.cpp \
    Loader.cpp \
    ObjectCreator.cpp \
    MeshManipulators.cpp

HEADERS += \
    CollisionResolver.h \
    EventReceiver.h \
    Game.h \
    Loader.h \
    Object.h \
    ObjectCreator.h \
    MeshManipulators.h
INCLUDEPATH += \
    /usr/include/bullet \
    /usr/include/irrlicht \
    usr/include/bullet/LinearMath

LIBS += \
    -L/usr/lib \
    -lIrrlicht -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath
