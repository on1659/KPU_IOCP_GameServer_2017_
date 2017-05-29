#pragma once

static unsigned int gInstance = 0;
class Object
{
public:
	
	Object(const std::string& name = "Object");

	virtual ~Object();

	virtual bool Start() { return true; };

	virtual bool Start(void* data) { return true; };

	virtual bool End() { return true; }

	const unsigned int getID() const { return nInstanceID; }
	
public:
	std::string  name;
	DWORD		 tag{ 0 };

private:
	unsigned int nInstanceID{ 0 };

};

