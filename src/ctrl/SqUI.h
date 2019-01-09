#pragma once


namespace sq {
   // event::Action
#ifdef _V1
    using EventAction = event::Action;
    using EventChange = event::Change;
#else
    using Action = rack::EventAction;
#endif
}