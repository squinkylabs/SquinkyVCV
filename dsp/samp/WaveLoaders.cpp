
#include "WaveLoader.h"


#pragma once


class WaveFileLoader : public WaveInfoInterface {

};

class FlacFileLoader : public WaveInfoInterface {

};

class TestFileLoader  : public WaveInfoInterface {

};




WaveLoader::WaveInfoPtr WaveLoader::loaderFactory(const std::string& extension) {
     assert(false);
     return nullptr;
 }