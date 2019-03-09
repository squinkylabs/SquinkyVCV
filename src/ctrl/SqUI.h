#pragma once

#ifdef __V1
#include "widget/event.hpp"
#endif

namespace sq {
  
#ifdef __V1
    using EventAction = ActionEvent;
    using EventChange = ChangeEvent;
    using Event = Event;
#else
    using Action = rack::EventAction;       // what is this?
    using EventAction = rack::EventAction;
    using EventChange = rack::EventChange;
    using Event = rack::Event;
#endif

#ifdef __V1
    inline void consumeEvent(const Event* evt, Widget* widget)
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