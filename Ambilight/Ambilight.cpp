// Ambilight.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <assert.h>
#include "Ambilight.h"


#if _DEBUG
#  define SAVE_SCREENSHOTS 0
#else
#  define SAVE_SCREENSHOTS 0
#endif


namespace Mambi 
{

#pragma region AmbilightFactory
	
	D3D_DRIVER_TYPE DriverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

	D3D_FEATURE_LEVEL FeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_1
	};
	UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);


	bool SelectDisplay(Output* output, const char* hardwareId)
	{
		IDXGIOutput* dxgiOutput = NULL;
		for (UINT16 i = 0; ; i++)
		{
			if (FAILED(output->Adapter->EnumOutputs(i, &dxgiOutput)))
			{
				break;
			}

			output->Instance.Release();
			HRESULT hr = dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&output->Instance));
			dxgiOutput->Release();
			if (FAILED(hr))
			{
				continue;
			}

			if (SUCCEEDED(output->Instance->GetDesc(&output->Desc)) && output->Desc.AttachedToDesktop)
			{
				for (UINT16 k = 0; EnumDisplayDevices(output->Desc.DeviceName, k, &output->Display, 0); k++)
				{
					std::wstring deviceId(output->Display.DeviceID);
					size_t pos = deviceId.find(L"\\");
					if (pos != std::string::npos)
					{
						pos = deviceId.find(L"\\", pos + 1);
						if (pos != std::string::npos)
						{
							deviceId = deviceId.substr(0, pos);
						}
					}

					output->HardwereId = std::string(deviceId.begin(), deviceId.end());

					if (output->HardwereId == hardwareId)
					{
						output->Top = output->Desc.DesktopCoordinates.top;
						output->Left = output->Desc.DesktopCoordinates.left;
						output->Width = output->Desc.DesktopCoordinates.right - output->Desc.DesktopCoordinates.left;
						output->Height = output->Desc.DesktopCoordinates.bottom - output->Desc.DesktopCoordinates.top;
						return true;
					}
				}
			}
		}

		return false;
	}	


	Ambilight::Ambilight(const char* hardwareId)
		: HardwareId(hardwareId), _samples(NULL), _output(NULL), _frame(NULL)
	{	
	}


	Ambilight::~Ambilight()	
	{
		//printf("Ambilight::~Ambilight\n");
		ResetOutput();
	}
	
#pragma endregion



#pragma region AmbilightClass

	AMBILIGHT_API bool Ambilight::UpdateSamplesDesc(const SampleDesc& horizontal, const SampleDesc& vertical, int8_t rotate)
	{
		DisplaySampleFactory<SampleOrient::Horizontal> hFactory(horizontal);
		DisplaySampleFactory<SampleOrient::Vertical> vFactory(vertical);

		bool changed = false;
		if (_samples == NULL 
			|| _samples->HFactory() != hFactory 
			|| _samples->VFactory() != vFactory)
		{
			changed = true;

			if (_samples == NULL)
			{
				_samples = std::make_shared<DisplaySamples>(hFactory, vFactory, rotate);
			}
			else
			{
				_samples->_hFactory = hFactory;
				_samples->_vFactory = vFactory;
			}
		}

		if (changed && _output != NULL) 
		{
			UpdateSamples();
		}

		return changed;
	}


	AMBILIGHT_API bool Ambilight::ProcessSamples()
	{
#if SAVE_SCREENSHOTS
		static int screenshotCounter = 0;
#endif

		if (CheckOutput())
		{
			switch (_frame->Acquire())
			{
			case S_OK:
				if (_frame->UpdateSamplesText())
				{
					_processor.UpdateAvarage(_samples.get(), _frame->SamplesData(), _frame->SamplesPitch());

#if SAVE_SCREENSHOTS
					char fnSample[255];
					sprintf_s(fnSample, "D:\\Workspace\\Mambi\\screenshot\\sample-%d.png", screenshotCounter);
					++screenshotCounter;
					_frame->SaveSamplesToPng(fnSample);
#endif
					return true;
				}
				else
				{
					return false;
				}

			case DXGI_ERROR_ACCESS_LOST:
				ResetOutput();
				return false;

			case DXGI_ERROR_WAIT_TIMEOUT:
				return false;

			default:
				throw new std::exception("Unexpected error occured.");
			}
		}
		else
		{
			ResetOutput();
			InitOutput();

			if (CheckOutput())
			{
				InitFrame();
			}
			else
			{
				ResetOutput();				
			}

			return false;
		}
	}


	AMBILIGHT_API bool Ambilight::Available()
	{
		if (CheckOutput())
		{
			return true;
		}
		else
		{
			InitOutput();
			InitFrame();
			return CheckOutput();
		}
	}


	bool Ambilight::CheckOutput()
	{
		if (_output != NULL)
		{
			switch (_output->Device->GetDeviceRemovedReason())
			{
			case S_OK:
				return true;

			case DXGI_ERROR_DEVICE_REMOVED:
			case DXGI_ERROR_DEVICE_RESET:
			case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
			case E_OUTOFMEMORY:
				return false;

			default:
				throw new std::exception("Unexpected result");
			}
		}
		else
		{
			return false;
		}
	}


	void Ambilight::ResetOutput()
	{
		if (_frame != NULL) 
		{
			delete _frame;
			_frame = NULL;
		}

		if (_output != NULL)
		{
			delete _output;
			_output = NULL;
		}
	}


	bool Ambilight::InitOutput()
	{
		assert(_output == NULL);
		_output = new Mambi::Output();

		HRESULT hr = S_OK;

		for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
		{
			hr = D3D11CreateDevice(
				nullptr,
				DriverTypes[DriverTypeIndex],
				nullptr,
#if _DEBUG
				D3D11_CREATE_DEVICE_DEBUG,
#else
				0,
#endif
				FeatureLevels,
				NumFeatureLevels,
				D3D11_SDK_VERSION,
				&_output->Device,
				&_output->Feature,
				&_output->Context);

			if (SUCCEEDED(hr))
			{
				break;
			}
		}

		if (FAILED(hr))
		{
			throw new std::exception("Failed to create device");
		}

		IDXGIDevice* DxgiDevice = NULL;
		hr = _output->Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&DxgiDevice));
		if (FAILED(hr))
		{
			throw new std::exception("Failed to QI for DXGI Device");
		}

		hr = DxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&_output->Adapter));
		DxgiDevice->Release();
		if (FAILED(hr))
		{
			throw new std::exception("Failed to get parent DXGI Adapter");
		}

		if (!SelectDisplay(_output, HardwareId))
		{
			throw new std::exception("Failed to get specified output");
		}

		UpdateSamples();
		return true;
	}
	

	void Ambilight::InitFrame()
	{
		assert(_frame == NULL);
		_frame = new Mambi::Frame(_output, _samples);
	}


	void Ambilight::UpdateSamples()
	{
		if (_samples != NULL)
		{
			_samples->Update(_output->Width, _output->Height, _output->Width, _output->Height);
		}
	}

#pragma endregion


}

