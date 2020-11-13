#pragma once

class SShared_GameObject
{
public:
	SShared_GameObject();
	virtual ~SShared_GameObject();

protected:
	Shift::SC_Matrix44 myTransform;

};

