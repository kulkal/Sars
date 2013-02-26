#pragma once
#include <string>
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>

class BaseObject
{
	std::string _Name;
public:
	void SetName(std::string InName){_Name = InName;}
	BaseObject();
	virtual ~BaseObject();
};