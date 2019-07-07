#pragma once

#ifdef __V1x
#include "event.hpp"
#endif

namespace sq {
  
#ifdef __V1x
   using EventAction = rack::event::Action;
   using EventChange = rack::event::Change;
  //  using Event = Event;
#else
    using Action = rack::EventAction;       // what is this?
    using EventAction = rack::EventAction;
    using EventChange = rack::EventChange;
    using Event = rack::Event;
#endif

#ifdef __V1x
    inline void consumeEvent(const rack::event::Base* evt, rack::Widget* widget)
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