#pragma once

namespace Math
{
	template<class T>
	T Clamp(T v, T a, T b)
	{
		return max(a, min(v, b));
	}
}