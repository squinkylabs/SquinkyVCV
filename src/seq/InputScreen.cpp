#include "InputScreen.h"

void InputScreenSet::show()
{

}

void InputScreenSet::add(InputScreenPtr is)
{
    screens.push_back(is);
}

InputScreenSet::~InputScreenSet()
{
    DEBUG("dtor iss");
}