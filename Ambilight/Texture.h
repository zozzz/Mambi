#pragma once
#include "stdafx.h"


namespace Mambi
{

	class Texture
	{
	public:
		Texture(ID3D11Device* device, D3D11_TEXTURE2D_DESC& desc): Desc(desc), Res(NULL)
		{
			HRESULT hr = device->CreateTexture2D(&Desc, NULL, &Res);
			if (FAILED(hr))
			{
				throw new std::exception("Failed to create texture");
			}
		}


		~Texture() 
		{
			if (Res != NULL)
			{
				Res->Release();
				Res = NULL;
			}
		}


		const D3D11_TEXTURE2D_DESC Desc;
		ID3D11Texture2D* Res;
	};

}