#pragma once
#define FLOAT_MAX  3.40282e+038
namespace Math
{
	template<class T>
	T Clamp(T v, T a, T b)
	{
		return max(a, min(v, b));
	}

	template<class T>
	T Max(T a, T b)
	{
		if(a > b)
			return a;
		else
			return b;
	}

	template<class T>
	T Min(T a, T b)
	{
		if( a < b)
			return a;
		else
			return b;
	}
}