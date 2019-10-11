#pragma once

#include "rack.hpp"

namespace sq {
  
    using EventAction = ::rack::event::Action;
    using EventChange = ::rack::event::Change;

    inline void consumeEvent(const ::rack::event::Base* evt, ::rack::Widget* widget)
    {
        evt->consume(widget);
    }
}