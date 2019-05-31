#pragma once

#ifdef __V1x
#include "event.hpp"
#endif

namespace sq {
  
#ifdef __V1x
   using EventAction = event::Action;
   using EventChange = event::Change;
  //  using Event = Event;
#else
    using Action = rack::EventAction;       // what is this?
    using EventAction = rack::EventAction;
    using EventChange = rack::EventChange;
    using Event = rack::Event;
#endif

#ifdef __V1x
    inline void consumeEvent(const event::Base* evt, Widget* widget)
    {
       evt->consume(widget);
    }
#else
    inline void consumeEvent(Event* evt, Widget* dummy)
    {
        evt->consumed = true;
    }
#endif

}