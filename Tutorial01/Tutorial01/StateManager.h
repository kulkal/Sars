#pragma once
#include <d3d11.h>
#include <d3dx11.h>
#include <vector>

enum EDepthStencilState
{
	DS_GBUFFER_PASS,
	DS_LIGHTING_PASS,
	SIZE_DEPTHSTENCILSTATE,
};

enum EBlendState{
	BS_NORMAL, BS_LIGHTING, BS_SHADOW,
	SIZE_BLENDSTATE,
};

enum ESamplerState
{
	SS_LINEAR, SS_SHADOW,
	SIZE_SAMPLERSTATE,
};

class StateManager
{
	// Depth Stencil States
	
	struct DepthStencilStateData
	{
		ID3D11DepthStencilState* DSS;
		UINT StencilRef;
		DepthStencilStateData()
		{
			DSS = NULL;
			StencilRef = 0;
		}
	};
	std::vector<DepthStencilStateData> _DepthStencilStateArray;

	// Blend States
	
	struct BlendStateData
	{
		ID3D11BlendState* BS;
		float BlendFactor[8];
		unsigned int SampleMask;
		BlendStateData()
		{
			BS = NULL;
			for(int i=0;i<8;i++)
				BlendFactor[i] = 1.f;
			SampleMask = 0xffffffff;
		}
	};
	std::vector<BlendStateData> _BlendStateArray;

	struct SamplerStateData
	{
		ID3D11SamplerState*		SS;
	};
	std::vector<SamplerStateData> _SamplerStateArray;

public:
	void Init();
	void InitBlendStates();
	void InitDepthStencilStates();
	void InitSamplerStates();

	void SetBlendState(EBlendState eBS);
	void SetDepthStencilState(EDepthStencilState eDSS);

	void SetPSSampler(int StartSlot, ESamplerState eSS);
public:

	StateManager(void);
	~StateManager(void);
};

extern StateManager* GStateManager;

#define SET_BLEND_STATE( eBS) GStateManager->SetBlendState(eBS);
#define SET_DEPTHSTENCIL_STATE( eDSS) GStateManager->SetDepthStencilState(eDSS);
#define SET_PS_SAMPLER(Slot, eSS) GStateManager->SetPSSampler(Slot, eSS);
