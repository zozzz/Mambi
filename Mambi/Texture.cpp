#include "stdafx.h"
#include "Texture.h"


namespace Mambi
{
	std::shared_ptr<MappedTexture> Texture::Map()
	{
		return std::shared_ptr<MappedTexture>(new MappedTexture(_texture, _desc));
	}


	MappedTexture::MappedTexture(ID3D11Texture2D* texture, D3D11_TEXTURE2D_DESC& desc)
		: _mapped(NULL), _tmp(NULL), _device(NULL), _context(NULL)
	{
		texture->GetDevice(&_device);
		_device->GetImmediateContext(&_context);

		HRESULT hr = _context->Map(texture, 0, D3D11_MAP_READ, 0, &_map);
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

			hr = _device->CreateTexture2D(&tmpDesc, nullptr, &_tmp);
			if (FAILED(hr))
			{
				throw new std::exception("Failed to create temp texture");
			}

			_context->CopyResource(_tmp, texture);

			hr = _context->Map(_tmp, 0, D3D11_MAP_READ, 0, &_map);
			if (FAILED(hr))
			{
				throw new std::exception("Failed to map the texture");
			}
			_mapped = _tmp;
		}
		else
		{
			_mapped = texture;
		}
	}

	MappedTexture::~MappedTexture()
	{
		if (_mapped != NULL) 
		{
			_context->Unmap(_mapped, 0);
			_mapped = NULL;
		}

		if (_tmp != NULL)
		{
			_tmp->Release();
			_tmp = NULL;
		}

		if (_device != NULL)
		{
			_device->Release();
			_device = NULL;
		}

		if (_context != NULL)
		{
			_context->Release();
			_context = NULL;
		}
	}
}
