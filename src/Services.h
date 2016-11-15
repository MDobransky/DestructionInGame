#ifndef SERVICES_H
#define SERVICES_H

#include <EventReceiver.h>
#include <Material.h>
#include <Loader.h>

namespace gg {


class Services
{
private:
    std::unique_ptr<ObjectCreator> o;
    std::unique_ptr<EventReceiver> e;
    std::unique_ptr<Loader> l;
public:
    Services();
    static ObjectCreator* getObjectCreator();
    static EventReceiver* getEventReceiver();
    static Loader* getLoader();
};

}

#endif // SERVICES_H
