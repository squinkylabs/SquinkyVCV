#pragma once


namespace sq {
   // event::Action
#ifdef __V1
    using EventAction = const event::Action;
    using EventChange = event::Change;
#else
    using Action = rack::EventAction;       // what is this?
    using EventAction = rack::EventAction;
    using EventChange = rack::EventChange;
#endif
}