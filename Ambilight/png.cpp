#include "stdafx.h"
#include <iostream>
#include "png.h"
#include "lodepng.h"


namespace Mambi
{
	bool SaveTextureToPng(ID3D11Texture2D* texture, const D3D11_TEXTURE2D_DESC& desc, const char* filename)
	{
		CComPtr<ID3D11Device> device;
		CComPtr<ID3D11DeviceContext> context;
		texture->GetDevice(&device);
		device->GetImmediateContext(&context);

		ID3D11Texture2D* mapped = NULL;
		D3D11_MAPPED_SUBRESOURCE map;
		HRESULT hr = context->Map(texture, 0, D3D11_MAP_READ, 0, &map);
		if (FAILED(hr))
		{
			D3D11_TEXTURE2D_DESC tmpDesc;
			tmpDesc.Width = desc.Width;
			tmpDesc.Height = desc.Height;
			tmpDesc.MipLevels = desc.MipLevels;
			tmpDesc.ArraySize = desc.ArraySize;
			tmpDesc.Format = desc.Format;
			tmpDesc.SampleDesc.Count = 1;
			tmpDesc.SampleDesc.Quality = 0;
			tmpDesc.Usage = D3D11_USAGE_STAGING;
			tmpDesc.BindFlags = 0;
			tmpDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			tmpDesc.MiscFlags = 0;

			ID3D11Texture2D* tmp;
			hr = device->CreateTexture2D(&tmpDesc, nullptr, &tmp);
			if (FAILED(hr))
			{
				return false;
			}

			context->CopyResource(tmp, texture);

			hr = context->Map(tmp, 0, D3D11_MAP_READ, 0, &map);
			if (FAILED(hr))
			{
				tmp->Release();
				return false;
			}
			mapped = tmp;
		}
		else
		{
			mapped = texture;
			mapped->AddRef();
		}

		bool res = SaveDataToPng((bgra_t*)map.pData, map.RowPitch, desc.Width, desc.Height, filename);

		context->Unmap(mapped, 0);
		mapped->Release();

		return res;
	}


	bool SaveDataToPng(bgra_t* data, size_t pitch, DisplayDim w, DisplayDim h, const char* filename)
	{
		pitch /= sizeof(bgra_t);
		rgba_t* buff(new rgba_t[w * h]);

		for (DisplayDim y = 0; y < h; ++y)
		{
			for (DisplayDim x = 0; x < w; ++x)
			{
				bgra_t& in = data[y * pitch + x];
				rgba_t& out = buff[y * w + x];
				out.r = in.r;
				out.g = in.g;
				out.b = in.b;
				out.a = 255;			
			}

		}

		bool res = lodepng::encode(filename, (unsigned char*)buff, w, h);
		delete[] buff;

		if (res != 0)
		{
			std::cout << "Failed to save png: " << lodepng_error_text(res) << std::endl;
			return false;
		}
		else
		{
			return true;
		}		
	}
}