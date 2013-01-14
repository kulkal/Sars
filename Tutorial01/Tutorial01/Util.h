#pragma once;
#include <D3D11.h>

#define SetD3DResourceDebugName(Name, Resource) \
	Resource->SetPrivateData( WKPDID_D3DDebugObjectName, strlen( Name ) - 1, Name );

