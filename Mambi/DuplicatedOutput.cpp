#include "stdafx.h"
//#include <wincodec.h>
#include "DuplicatedOutput.h"
#include "Console.h"
#include "utils.h"
#include "Color.h"
//#include "VertexShader.h"
//#include "PixelShader.h"


namespace Mambi {

	std::shared_ptr<DuplicatedOutput::Device> DuplicatedOutput::CreateDevice(CComPtr<IDXGIAdapter1> adapter)
	{
		std::shared_ptr<DuplicatedOutput::Device> device(new DuplicatedOutput::Device());

		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_1
		};
		UINT numFeatureLevels = ARRAYSIZE(featureLevels);
		
		/*
		HRESULT hr = D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &device->Instance, &device->FeatureLevel, &device->Context);
		*/

		HRESULT hr = D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &device->Instance, &device->FeatureLevel, &device->Context);
		
		if (FAILED(hr))
		{
			throw new std::exception("Failed to create device");
		}
		


		/*
		// VERTEX shader
		UINT Size = ARRAYSIZE(g_VS);
		hr = device->Instance->CreateVertexShader(g_VS, Size, nullptr, &device->VertexShader);
		if (FAILED(hr))
		{
			throw new std::exception("Failed to create vertex shader");			
		}
		
		// Input layout
		D3D11_INPUT_ELEMENT_DESC Layout[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
		UINT NumElements = ARRAYSIZE(Layout);
		hr = device->Instance->CreateInputLayout(Layout, NumElements, g_VS, Size, &device->InputLayout);
		if (FAILED(hr))
		{
			throw new std::exception("Failed to create input layout");			
		}
		device->Context->IASetInputLayout(device->InputLayout);


		// Pixel shader
		Size = ARRAYSIZE(g_PS);
		hr = device->Instance->CreatePixelShader(g_PS, Size, nullptr, &device->PixelShader);
		if (FAILED(hr))
		{
			throw new std::exception("Failed to create pixel shader");			
		}

		// Set up sampler
		D3D11_SAMPLER_DESC SampDesc;
		RtlZeroMemory(&SampDesc, sizeof(SampDesc));
		SampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		SampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		SampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		SampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		SampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		SampDesc.MinLOD = 0;
		SampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		hr = device->Instance->CreateSamplerState(&SampDesc, &device->SamplerLinear);
		if (FAILED(hr))
		{
			throw new std::exception("Failed to create sampler state");			
		}
		*/

		return device;
	}


	DuplicatedOutput::DuplicatedOutput(std::shared_ptr<DuplicatedOutput::Device> device, CComPtr<IDXGIOutput> output, DXGI_OUTPUT_DESC outputDesc, DISPLAY_DEVICE outputDevice)
		: _device(device), _output(output), _outputDesc(outputDesc), _outputDevice(outputDevice)
	{

		CComQIPtr<IDXGIOutput1> o;

		if (FAILED(output->QueryInterface(__uuidof(o), reinterpret_cast<void**>(&o))))
		{
			throw new std::exception("Failed to QI for DxgiOutput1");
		}

		if (FAILED(o->DuplicateOutput(_device->Instance, &_duplicatedOutput)))
		{
			throw new std::exception("Failed to get duplicated output");
		}		

		/*
		DXGI_OUTDUPL_DESC outDesc;		
		_duplicatedOutput->GetDesc(&outDesc);

		Console::WriteLine("Dupe Width %ld, Dupe Height %ld, DeskLeft %d DeskTop %d", outDesc.ModeDesc.Width, outDesc.ModeDesc.Height);
		*/
	}


	DuplicatedOutput::~DuplicatedOutput()
	{
	}

	std::shared_ptr<DuplicatedOutput::Frame> DuplicatedOutput::NewFrame()
	{
		return std::shared_ptr<DuplicatedOutput::Frame>(new DuplicatedOutput::Frame(_duplicatedOutput));
		
		/*
		std::shared_ptr<DuplicatedOutput::Frame> frame(new DuplicatedOutput::Frame(_duplicatedOutput));
		HRESULT hr;

		RtlZeroMemory(&frame->_sharedTextDesc, sizeof(D3D11_TEXTURE2D_DESC));
		frame->_sharedTextDesc.Width = Width();
		frame->_sharedTextDesc.Height = Height();
		frame->_sharedTextDesc.MipLevels = 1;
		frame->_sharedTextDesc.ArraySize = 1;
		frame->_sharedTextDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		frame->_sharedTextDesc.SampleDesc.Count = 1;
		frame->_sharedTextDesc.Usage = D3D11_USAGE_DEFAULT;
		frame->_sharedTextDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		//frame->_sharedTextDesc.BindFlags = 0;
		frame->_sharedTextDesc.CPUAccessFlags = 0;
		//frame->_sharedTextDesc.MiscFlags = 0;
		frame->_sharedTextDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
		

		// frame->_sharedText
		hr = _device->Instance->CreateTexture2D(&frame->_sharedTextDesc, nullptr, &frame->_sharedText);
		if (FAILED(hr))
		{
			return NULL;
		}


		// frame->_sharedTextMutex
		hr = frame->_sharedText->QueryInterface(__uuidof(IDXGIKeyedMutex), reinterpret_cast<void**>(&frame->_sharedTextMutex));
		if (FAILED(hr))
		{
			return NULL;
		}
		
		
		// sharedHandle
		HANDLE sharedHandle;
		CComPtr<IDXGIResource> DXGIResource;
		hr = frame->_sharedText->QueryInterface(__uuidof(IDXGIResource), reinterpret_cast<void**>(&DXGIResource));
		if (FAILED(hr))
		{
			return NULL;
		}
		DXGIResource->GetSharedHandle(&sharedHandle);		

		// frame->_sharedSurf
		hr = _device->Instance->OpenSharedResource(sharedHandle, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&frame->_sharedSurf));
		if (FAILED(hr))
		{
			return NULL;
		}

		// frame->_sharedSurfMutex
		hr = frame->_sharedSurf->QueryInterface(__uuidof(IDXGIKeyedMutex), reinterpret_cast<void**>(&frame->_sharedSurfMutex));
		if (FAILED(hr))
		{
			return NULL;
		}

		_device->Context->OMSetRenderTargets(1, &frame->renderTarget, NULL);

		frame->_sharedSurf->GetDesc(&frame->_sharedSurfDesc);
		

		return frame;
		*/
	}

	/*
	bool DuplicatedOutput::AcquireNextFrame(Frame* frame)
	{
		HRESULT hr = _duplicatedOutput->AcquireNextFrame(1000, &frame->_info, &frame->_resource);		

		if (FAILED(hr))
		{
			return false;
		}

		auto releaseFrame = Finally([&] { _duplicatedOutput->ReleaseFrame(); });

		ID3D11Texture2D* texture;
		hr = frame->_resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&texture));
		if (FAILED(hr))
		{
			throw new std::exception("Failed to QI for ID3D11Texture2D from acquired IDXGIResource");
		}
		frame->_texture = new Texture(texture);

#ifndef 0 && NDEBUG
		assert(SaveToPng("D:\\Workspace\\Mambi\\screenshot\\frame.png", frame->_texture));
#endif // !NDEBUG

		return frame;
	}
	*/


	bool DuplicatedOutput::Frame::Acquire()
	{
		Release();

		HRESULT hr = _duplicatedOutput->AcquireNextFrame(1000, &_deskInfo, &_deskResource);
		if (FAILED(hr))
		{
			return false;
		}

		/*if (_deskInfo.AccumulatedFrames == 0)
		{
			return false;
		}*/
		
		hr = _deskResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&_deskText));
		if (FAILED(hr))
		{
			_duplicatedOutput->ReleaseFrame();
			return false;
		}		

		/*
		if (_deskInfo.TotalMetadataBufferSize)
		{
			if (!_meta.EnsureSize(_deskInfo.TotalMetadataBufferSize))
			{
				return false;
			}

			UINT consumedSize;
			// Get move rectangles
			hr = _duplicatedOutput->GetFrameMoveRects(_meta.Size(), reinterpret_cast<DXGI_OUTDUPL_MOVE_RECT*>(_meta.Data()), &consumedSize);
			if (FAILED(hr))
			{
				return false;
			}
			_moveCount = consumedSize / sizeof(DXGI_OUTDUPL_MOVE_RECT);

			hr = _duplicatedOutput->GetFrameDirtyRects(_meta.Size() - consumedSize, reinterpret_cast<RECT*>(_meta.Data() + consumedSize), &consumedSize);
			if (FAILED(hr))
			{
				return false;
			}
			_dirtyCount = consumedSize / sizeof(RECT);
		}		
		*/

#ifndef NDEBUG
		//std::auto_ptr<Texture> _texture(new Texture(_deskText));
		//assert(SaveToPng("D:\\Workspace\\Mambi\\screenshot\\frame.png", _texture.get()));
#endif // !NDEBUG

		return true;
	}

// https://stackoverflow.com/questions/20784385/calculate-average-of-pixels-in-the-front-buffer-of-the-gpu-without-copying-the-f
	bool DuplicatedOutput::Frame::UpdateSamples(DisplaySamples& samples)
	{
		ID3D11Device* device;
		ID3D11DeviceContext* context;
		HRESULT hr;

		_deskText->GetDevice(&device);
		device->GetImmediateContext(&context);
		_deskText->GetDesc(&_deskDesc);

		if (_stagingText == NULL || _stagingTextDesc.Width != _deskDesc.Width || _stagingTextDesc.Height != _deskDesc.Height)
		{
			if (_stagingText != NULL)
			{
				_stagingText->Release();
				_stagingText = NULL;
			}			

			memcpy_s(&_stagingTextDesc, sizeof(D3D11_TEXTURE2D_DESC), &_deskDesc, sizeof(D3D11_TEXTURE2D_DESC));
			//_stagingTextDesc.Width = samples.Width();
			_stagingTextDesc.Width = _deskDesc.Width;
			//_stagingTextDesc.Width = _sharedSurfDesc.Width;
			//_stagingTextDesc.Height = samples.Height();
			_stagingTextDesc.Height = _deskDesc.Height;
			//_stagingTextDesc.Height = _sharedSurfDesc.Height;
			_stagingTextDesc.Usage = D3D11_USAGE_STAGING;
			_stagingTextDesc.BindFlags = 0;
			_stagingTextDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			_stagingTextDesc.MiscFlags = 0;
			
			hr = device->CreateTexture2D(&_stagingTextDesc, nullptr, &_stagingText);
			if (FAILED(hr))
			{
				return false;
			}


			/*memcpy_s(&_samplesDesc, sizeof(D3D11_TEXTURE2D_DESC), &_deskDesc, sizeof(D3D11_TEXTURE2D_DESC));
			_samplesDesc.Width = samples.Width();
			_samplesDesc.Height = samples.Height();
			_samplesDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
			_samplesDesc.MiscFlags = 0;

			hr = device->CreateTexture2D(&_samplesDesc, nullptr, &_samplesText);
			if (FAILED(hr))
			{
				return false;
			}
			*/
		}

		assert(_stagingText != NULL);

		//CopySamplesRect(context, samples);
		//context->CopyResource(_stagingText, _sharedText);

		/*D3D11_BOX src;
		src.top = 0;
		src.left = 0;
		src.right = width * 4;
		src.bottom = height * 4;
		src.front = 0;
		src.back = 0;
		context->CopySubresourceRegion(_stagingText, 0, 0, 0, 0, _deskText, 0, &src);

		context->Flush();
		Sleep(1000);
		*/
		context->CopyResource(_stagingText, _deskText);
		

		D3D11_MAPPED_SUBRESOURCE map;
		hr = context->Map(_stagingText, 0, D3D11_MAP_READ, 0, &map);
		if (FAILED(hr))
		{
			return false;
		}				

		_UpdateSamples(samples, map);



#ifndef NDEBUG
		//SaveToPng("D:\\Workspace\\Mambi\\screenshot\\samples.png", map, _stagingTextDesc);
#endif

		context->Unmap(_stagingText, 0);

		return true;
	}


	void DuplicatedOutput::Frame::CopySamplesRect(ID3D11DeviceContext* context, DisplaySamples& samples)
	{
		//assert(_samplesText != NULL);
		for (auto sample : samples.Items())
		{
			context->CopySubresourceRegion(_sharedSurf, 0, sample.dst.left, sample.dst.top, 0, _deskText, 0, &sample.src);
		}
	}


	bool DuplicatedOutput::Frame::CopyMoveRects()
	{
		return true;
	}


	bool DuplicatedOutput::Frame::CopyDirtyRects()
	{
		return true;
	}


	void DuplicatedOutput::Frame::Release()
	{
		if (_deskText != NULL)
		{
			_deskText->Release();
			_duplicatedOutput->ReleaseFrame();
			_deskText = NULL;
		}	
	}


	// https://www.compuphase.com/graphic/scale3.htm
	bool DuplicatedOutput::Frame::_UpdateSamples(DisplaySamples& samples, D3D11_MAPPED_SUBRESOURCE& map)
	{
		bgra_t* data = (bgra_t*)map.pData;
		bgra_t& pixel = data[0];
		UINT rowPitch = map.RowPitch / sizeof(bgra_t);
		UINT r = 0;
		UINT g = 0;
		UINT b = 0;
		UINT count = 0;
		UINT rowOffset = 0;
		
		for (auto& sample : samples.Items())
		{
			r = 0;
			g = 0;
			b = 0;
			count = sample.width * sample.height;

			auto& src = sample.src;
			for (UINT y=src.top ; y < src.bottom ; ++y)
			{
				rowOffset = rowPitch * y;
				for (UINT x = src.left; x < src.right; ++x)
				{
					pixel = data[rowOffset + x];
					g += pixel.g;
					b += pixel.b;
					r += pixel.r;
				}
			}

			sample.avg.r = r / count;
			sample.avg.g = g / count;
			sample.avg.b = b / count;

			//Console::WriteLine("AVG(%d, %d, %d)", sample.avg.r, sample.avg.g, sample.avg.b);
		}
		
		return true;
	}



#if 0
	void DuplicatedOutput::Frame::UpdateSamples(const DisplaySamples& samples)
	{
		/*
		DisplayDim height = 0;
		for (auto sample: samples) 
		{
			height = max(height, sample.height);
		}

		D3D11_TEXTURE2D_DESC texDesc;
		_texture->GetDesc(&texDesc);
		CComPtr<ID3D11Device> device;
		_texture->GetDevice(&device);
		CComPtr<ID3D11DeviceContext> context;
		device->GetImmediateContext(&context);

		D3D11_TEXTURE2D_DESC stagingDesc;
		stagingDesc.Width = texDesc.Width;
		stagingDesc.Height = height;
		stagingDesc.MipLevels = texDesc.MipLevels;
		stagingDesc.ArraySize = texDesc.ArraySize;
		stagingDesc.Format = texDesc.Format;
		stagingDesc.SampleDesc = texDesc.SampleDesc;
		stagingDesc.Usage = D3D11_USAGE_STAGING;
		stagingDesc.BindFlags = 0;
		stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		stagingDesc.MiscFlags = 0;

		CComPtr<ID3D11Texture2D> stagingTex;
		D3D11_MAPPED_SUBRESOURCE map;

		HRESULT hr = device->CreateTexture2D(&stagingDesc, nullptr, &stagingTex);
		if (FAILED(hr)) 
		{
			return NULL;
		}

		UINT x = 0;
		D3D11_BOX sourceRegion;
		for (auto sample : samples) 
		{			
			sourceRegion.left = sample.left;
			sourceRegion.right = sample.left + sample.width;
			sourceRegion.top = sample.top;
			sourceRegion.bottom = sample.top + sample.height;
			sourceRegion.front = 0;
			sourceRegion.back = 1;
			context->CopySubresourceRegion(stagingTex, 0, x, 0, 0, _texture, 0, &sourceRegion);
			x += sample.width;
		}

		hr = context->Map(stagingTex, 0, D3D11_MAP_READ, 0, &map);
		if (FAILED(hr))
		{
			return NULL;
		}

		rgba_t* data = (rgba_t*)map.pData;

#ifndef NDEBUG
		assert(SaveToPng("D:\\Workspace\\Mambi\\screenshot\\samples.bmp", stagingTex));
#endif // !NDEBUG


		Console::WriteLine("RowPitch = %d, Height = %d, PX(0:0)=RGBA(%d, %d, %d, %d)", map.RowPitch, height,
			data[0].r, data[0].g, data[0].b, data[0].a);

		return (rgba_t*)map.pData;
		*/		
	}	

#endif


#ifndef NDEBUG
	bool SaveToPng(const char* filename, Texture* texture)
	{
		Console::WriteLine("SaveToPng %s", filename);
		auto mapped = texture->Map();
		auto data = mapped->Data<bgra_t>();
		
		std::unique_ptr<rgba_t> buff(new rgba_t[texture->Width() * texture->Height()]);

		for (uint32_t y = 0; y < texture->Height(); ++y)
		{
			for (uint32_t x = 0; x < texture->Width(); ++x)
			{
				auto& c = data->At(x, y);
				auto& o = buff.get()[texture->Width() * y + x];
				o.r = c.r;
				o.g = c.g;
				o.b = c.b;
				o.a = 255;
			}
		}		

		return lodepng::encode(filename, reinterpret_cast<BYTE*>(buff.get()), texture->Width(), texture->Height()) == 0;
	}

	bool SaveToPng(const char* filename, D3D11_MAPPED_SUBRESOURCE& map, D3D11_TEXTURE2D_DESC& desc)
	{	
		Console::WriteLine("SaveToPng %s", filename);
		std::unique_ptr<rgba_t> buff(new rgba_t[desc.Width * desc.Height]);
		TextureData<bgra_t> data((bgra_t*)map.pData, map.RowPitch);

		for (uint32_t y = 0; y < desc.Height; ++y)
		{
			for (uint32_t x = 0; x < desc.Width; ++x)
			{
				auto& c = data.At(x, y);
				auto& o = buff.get()[desc.Width * y + x];
				o.r = c.r;
				o.g = c.g;
				o.b = c.b;
				o.a = 255;
			}
		}

		return lodepng::encode(filename, reinterpret_cast<BYTE*>(buff.get()), desc.Width, desc.Height) == 0;
	}
#endif // !NDEBUG

}