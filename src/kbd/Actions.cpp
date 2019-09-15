#include "Actions.h"

 Actions::Actions()
 {
     _map = {
         {"insert.default", insertDefault}
     };
 }

 Actions::action Actions::getAction(const std::string& name)
 {
     auto it = _map.find(name);
     if (it == _map.end()) {
         fprintf(stderr, "bad action name: %s\n", name.c_str());
         return nullptr;
     }
     
     action a = it->second;
     return a;
 }


 bool Actions::insertDefault(MidiSequencerPtr)
 {
    fprintf(stderr, "insertDefauls nimp\n");
    return false;
 }