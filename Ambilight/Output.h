#pragma once
#include "stdafx.h"
#include "Common.h"


namespace Mambi
{
	struct Output
	{
		Output()
		{
			ZeroMemory(this, sizeof(Output));
			Display.cb = sizeof(DISPLAY_DEVICE);
		};

		DisplayDim Top;
		DisplayDim Left;
		DisplayDim Width;
		DisplayDim Height;

		DXGI_OUTPUT_DESC Desc;
		DISPLAY_DEVICE Display;
		std::string HardwereId;

		D3D_FEATURE_LEVEL Feature;
		CComPtr<ID3D11Device> Device;
		CComPtr<ID3D11DeviceContext> Context;
		CComPtr<IDXGIAdapter1> Adapter;
		CComPtr<IDXGIOutput1> Instance;
	};
}
