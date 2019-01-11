#pragma once


namespace sq {
   // event::Action
#ifdef __V1
    using EventAction = event::Action;
    using EventChange = event::Change;
#else
    using Action = rack::EventAction;
#endif
}