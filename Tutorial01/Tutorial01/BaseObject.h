#pragma once
#include <string>
class BaseObject
{
	std::string _Name;
public:
	void SetName(std::string InName){_Name = InName;}
	BaseObject();
	virtual ~BaseObject();
};