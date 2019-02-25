#pragma once
#include "Color.h"
#include "DisplaySample.h"
#include "Texture.h"
#include "utils.h"

#ifndef NDEBUG
#	include "lodepng.h"
#endif // !NDEBUG



namespace Mambi
{

	class DuplicatedOutput
	{
	public:
		struct Device {
			ID3D11Device* Instance;
			ID3D11DeviceContext* Context;
			ID3D11VertexShader* VertexShader;
			ID3D11InputLayout* InputLayout;
			ID3D11PixelShader* PixelShader;
			ID3D11SamplerState* SamplerLinear;

			D3D_FEATURE_LEVEL FeatureLevel;

			Device(): Instance(NULL), Context(NULL), VertexShader(NULL), InputLayout(NULL), PixelShader(NULL), SamplerLinear(NULL){ }

			~Device()
			{
				if (Instance != NULL) 
				{
					Instance->Release();
					Instance = NULL;
				}

				if (Context != NULL)
				{
					Context->Release();
					Context = NULL;
				}

				if (VertexShader != NULL)
				{
					VertexShader->Release();
					VertexShader = NULL;
				}

				if (InputLayout != NULL)
				{
					InputLayout->Release();
					InputLayout = NULL;
				}

				if (PixelShader != NULL)
				{
					PixelShader->Release();
					PixelShader = NULL;
				}

				if (SamplerLinear != NULL)
				{
					SamplerLinear->Release();
					SamplerLinear = NULL;
				}
			}
		};

		class Frame 
		{
		public:
			friend DuplicatedOutput;

			Frame(IDXGIOutputDuplication* const duplicatedOutput) 
				: _duplicatedOutput(duplicatedOutput), 
				  _deskResource(NULL),
				  _deskText(NULL),
				  _stagingText(NULL) {};

			~Frame()
			{
				Console::WriteLine("Frame::~Frame");
				if (_stagingText != NULL)
				{
					_stagingText->Release();
					_stagingText = NULL;
				}
				Release();
			}

			bool Acquire();
			void Release();
			bool UpdateSamples(DisplaySamples& samples);

			/*
			inline auto& Info() const { return _info; }
			inline auto& Resource() const { return _resource; }
			inline auto& Texture() const { return _texture; }	

			bool Init();
			const void UpdateSamples(const DisplaySamples& samples);
			void Release();
			*/

		private:
			void CopySamplesRect(ID3D11DeviceContext* context, DisplaySamples& samples);
			bool _UpdateSamples(DisplaySamples& samples, D3D11_MAPPED_SUBRESOURCE& map);
			bool CopyMoveRects();
			bool CopyDirtyRects();


			IDXGIOutputDuplication* const _duplicatedOutput;	
			D3D11_TEXTURE2D_DESC _stagingTextDesc;
			D3D11_TEXTURE2D_DESC _samplesDesc;
			D3D11_TEXTURE2D_DESC _deskDesc;

			ID3D11Texture2D* _stagingText;
			
			ID3D11Texture2D* _samplesText;

			
			D3D11_TEXTURE2D_DESC _sharedTextDesc;
			ID3D11Texture2D* _sharedText;
			CComPtr<IDXGIKeyedMutex> _sharedTextMutex;


			D3D11_TEXTURE2D_DESC _sharedSurfDesc;
			ID3D11Texture2D* _sharedSurf;
			CComPtr<IDXGIKeyedMutex> _sharedSurfMutex;

			ID3D11RenderTargetView *renderTarget;
			
			DXGI_OUTDUPL_FRAME_INFO _deskInfo;
			IDXGIResource* _deskResource;
			ID3D11Texture2D* _deskText;
			Buffer<BYTE> _meta;
			UINT _moveCount;
			UINT _dirtyCount;
		};


		DuplicatedOutput(std::shared_ptr<Device> device, CComPtr<IDXGIOutput> output, DXGI_OUTPUT_DESC outputDesc, DISPLAY_DEVICE outputDevice);
		~DuplicatedOutput();


		inline auto& Rect() const { return _outputDesc.DesktopCoordinates; }
		inline auto& Width() const { return _outputDesc.DesktopCoordinates.right; }
		inline auto& Height() const { return _outputDesc.DesktopCoordinates.bottom; }

		static std::shared_ptr<Device> CreateDevice(CComPtr<IDXGIAdapter1> adapter);
		std::shared_ptr<DuplicatedOutput::Frame> NewFrame();		

	private:
		std::shared_ptr<Device> _device;
		CComPtr<IDXGIOutput> _output;
		CComPtr<IDXGIOutputDuplication> _duplicatedOutput;
		DXGI_OUTPUT_DESC _outputDesc;
		DISPLAY_DEVICE _outputDevice;
	};


	

#ifndef NDEBUG
	bool SaveToPng(const char* filename, Texture* texture);
	bool SaveToPng(const char* filename, D3D11_MAPPED_SUBRESOURCE& map, D3D11_TEXTURE2D_DESC& desc);
#endif
}