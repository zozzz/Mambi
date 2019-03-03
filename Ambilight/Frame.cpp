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
		printf("Frame::~Frame\n");

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
			ZeroMemory(&_mapSamples, sizeof(D3D11_MAPPED_SUBRESOURCE));
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
		return SUCCEEDED(hr);
	}


#if 0
	// https://www.compuphase.com/graphic/scale3.htm
	bool Frame::UpdateSamplesAvg()
	{
		bgra_t* data = reinterpret_cast<bgra_t*>(_mapSamples.pData);
		size_t pitch = _mapSamples.RowPitch / sizeof(UINT32);
		size_t count;
		DisplayDim w;
		DisplayDim h;
		size_t r, g, b;
		UINT32 avg;

		for (auto& sample: _samples->Items())
		{
			r = 0;
			g = 0;
			b = 0;
			count = 0;
			avg = 0;

			for (DisplayDim y = sample.Dst.top; y < sample.Dst.bottom; ++y)
			{
				size_t offset = y * pitch;
				for (DisplayDim x = sample.Dst.left; x < sample.Dst.right; ++x)
				{
					bgra_t& inp = data[offset + x];
					
					/*
					double scaling = 1.0 / (double)(count + 1);
					r = inp.r * scaling + r * (1.0 - scaling);
					g = inp.g * scaling + g * (1.0 - scaling);
					b = inp.b * scaling + b * (1.0 - scaling);
					++count;
					*/

					/*
					r = (r + inp.r) / count;
					g = (g + inp.g) / count;
					b = (b + inp.b) / count;
					*/

					//avg = ((avg ^ inp) >> 1) + (avg & inp);

					r += inp.r;
					g += inp.g;
					b += inp.b;
					++count;
				}
			}

			sample.Avg.r = r / count;
			sample.Avg.g = g / count;
			sample.Avg.b = b / count;
		}

		return true;
	}
#endif


	bool Frame::CreateSamplesTexture()
	{
		_txSamplesDesc = _txAckDesc;
		_txSamplesDesc.Width = _samples->Width();
		_txSamplesDesc.Height = _samples->Height();
		_txSamplesDesc.BindFlags = 0;
		_txSamplesDesc.Usage = D3D11_USAGE_STAGING;
		_txSamplesDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		_txSamplesDesc.MiscFlags = 0;
		
		HRESULT hr = _output->Device->CreateTexture2D(&_txSamplesDesc, 0, &_txSamples);
		if (FAILED(hr))
		{
			throw new std::exception("Failed to create samples texture");
			return false;
		}
		return true;
	}



#ifdef _DEBUG
	bool Frame::SaveSamplesToPng(const char* filename) const
	{
		return SaveDataToPng(SamplesData(), SamplesPitch(), _txSamplesDesc.Width, _txSamplesDesc.Height, filename);
	}
#endif

}