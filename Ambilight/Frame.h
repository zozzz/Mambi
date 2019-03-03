#pragma once
#include "stdafx.h"
#include "DisplaySample.h"
#include "Output.h"

#ifdef _DEBUG
#  include "png.h"
#endif


namespace Mambi 
{

	class Frame
	{
	public:
		Frame(Output* const output, std::shared_ptr<DisplaySamples> samples);
		~Frame();

		inline bgra_t* SamplesData() const { return reinterpret_cast<bgra_t*>(_mapSamples.pData); }
		inline auto& SamplesPitch() const { return _mapSamples.RowPitch; }

		HRESULT Acquire();		
		bool UpdateSamplesText();
		
#ifdef _DEBUG
		bool SaveSamplesToPng(const char* filename) const;
#endif

	private:		
		bool CreateSamplesTexture();

	private:
		Output* const _output;
		std::shared_ptr<DisplaySamples> _samples;

		CComPtr<IDXGIOutputDuplication> _duplicatedOutput;

		D3D11_TEXTURE2D_DESC _txAckDesc;
		ID3D11Texture2D* _txAck;

		D3D11_TEXTURE2D_DESC _txSamplesDesc;
		ID3D11Texture2D* _txSamples;
		D3D11_MAPPED_SUBRESOURCE _mapSamples;
	};

}