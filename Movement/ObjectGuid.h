#pragma once

#ifdef USE_FAKE_CLASSES

class ObjectGuid
{
public:
	ObjectGuid& ReadAsPacked() { return *this;}
	ObjectGuid& WriteAsPacked() { return *this;}
};

#endif

