#include "stdafx.h"
#include "Object.h"


Object::Object(const std::string& name) : name(name), nInstanceID(gInstance++)
{
}


Object::~Object()
{
}
