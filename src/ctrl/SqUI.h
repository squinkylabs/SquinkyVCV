#pragma once

#include "event.hpp"

namespace sq {
  
#ifdef __V1
    using EventAction = const event::Action;
    using EventChange = event::Change;
    using Event = rack::event::Event;
#else
    using Action = rack::EventAction;       // what is this?
    using EventAction = rack::EventAction;
    using EventChange = rack::EventChange;
#endif

#ifdef __V1
    inline void consumeEvent(const Event* evt, ParamWidget* widget)
    {
       evt->consume(widget);
    }
#else
    inline void consumeEvent(Event* evt)
    {
        evt->consumed = true;
    }
#endif

}