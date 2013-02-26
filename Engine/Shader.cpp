#include "Shader.h"
#include <cassert>
#include "Engine.h"
#include "Util.h"

Shader::Shader(void)
	:_ConstantBuffer(NULL)
{
}


Shader::~Shader(void)
{
	if(_ConstantBuffer) _ConstantBuffer->Release();
}
