#include "stdafx.h"
#include <assert.h>
#include "Ambilight.h"
#include "utils.h"


namespace Mambi
{
	Frame::Frame(Output* const output, std::shared_ptr<DisplaySamples> samples)
		: _output(output), _samples(samples), _txSamples(NULL)
	{
		HRESULT hr = output->Instance->DuplicateOutput(output->Device, &_duplicatedOutput);
		if (FAILED(hr))
		{
			throw new std::exception("Failed to get duplicated output");
		}

		ZeroMemory(&_mapSamples, sizeof(D3D11_MAPPED_SUBRESOURCE));
	}


	Frame::~Frame()
	{
		if (_txAck)
		{
			_txAck->Release();
			_txAck = NULL;

			if (_duplicatedOutput != NULL)
			{
				_duplicatedOutput->ReleaseFrame();
				_duplicatedOutput = NULL;
			}
		}

		if (_txSamples != NULL)
		{
			_txSamples->Release();
			_txSamples = NULL;
		}
	}


	HRESULT Frame::Acquire()
	{
		DXGI_OUTDUPL_FRAME_INFO info;
		CComPtr<IDXGIResource> res;

		HRESULT hr = _duplicatedOutput->AcquireNextFrame(100, &info, &res);
		if (SUCCEEDED(hr))
		{
			hr = res->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&_txAck));
			if (SUCCEEDED(hr))
			{
				if (_txSamples != NULL)
				{
					return S_OK;
				}
				else
				{
					_txAck->GetDesc(&_txAckDesc);
					if (CreateSamplesTexture())
					{
						return S_OK;
					}
					else
					{
						return S_FALSE;
					}
				}
			}
			else
			{
				return hr;
			}
		}
		else if (hr == DXGI_ERROR_INVALID_CALL)
		{
			return DXGI_ERROR_ACCESS_LOST;
		}
		else
		{
			return hr;
		}
	}


	bool Frame::UpdateSamplesText()
	{
		if (_mapSamples.pData != NULL)
		{
			_output->Context->Unmap(_txSamples, 0);
		}

		for (auto& sample : _samples->Items())
		{
			_output->Context->CopySubresourceRegion(_txSamples, 0, sample.Dst.left, sample.Dst.top, 0, _txAck, 0, &sample.Src);
		}

		assert(_txAck != NULL);
		_txAck->Release();
		_txAck = NULL;
		_duplicatedOutput->ReleaseFrame();


		HRESULT hr = _output->Context->Map(_txSamples, 0, D3D11_MAP_READ, 0, &_mapSamples);
		if (SUCCEEDED(hr))
		{
			return true;
		}
		else
		{
			ZeroMemory(&_mapSamples, sizeof(D3D11_MAPPED_SUBRESOURCE));
			return false;
		}
	}


	bool Frame::CreateSamplesTexture()
	{
		_txSamplesDesc = _txAckDesc;
		_txSamplesDesc.Width = _samples->Width();
		_txSamplesDesc.Height = _samples->Height();
		_txSamplesDesc.BindFlags = 0;
		_txSamplesDesc.Usage = D3D11_USAGE_STAGING;
		_txSamplesDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		_txSamplesDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

		HRESULT hr = _output->Device->CreateTexture2D(&_txSamplesDesc, 0, &_txSamples);
		if (SUCCEEDED(hr))
		{
			return true;
		}
		else
		{
			throw new std::exception("Failed to create samples texture");
			return false;
		}
	}



#ifdef _DEBUG
	bool Frame::SaveSamplesToPng(const char* filename) const
	{
		return SaveDataToPng(SamplesData(), SamplesPitch(), _txSamplesDesc.Width, _txSamplesDesc.Height, filename);
	}
#endif

}
