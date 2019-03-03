#pragma once
#include "stdafx.h"
#include "Color.h"
#include "DisplaySample.h"


namespace Mambi
{
	bool SaveTextureToPng(ID3D11Texture2D* texture, const D3D11_TEXTURE2D_DESC& desc, const char* filename);
	bool SaveDataToPng(bgra_t* data, size_t pitch, DisplayDim w, DisplayDim h, const char* filename);
}