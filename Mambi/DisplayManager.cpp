#include "stdafx.h"
#include "DisplayManager.h"
#include "Application.h"
#include "Console.h"


namespace Mambi
{

	bool DisplayManager::UpdateOutputs()
	{
		CComPtr<IDXGIFactory1> factory;
		if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&factory))))
		{
			throw new std::exception("CreateDXGIFactory1 failed...");
		}

		//std::vector<CComPtr<IDXGIAdapter1>> adapters;
		CComPtr<IDXGIAdapter1> adapter;
		for (UINT16 i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++)
		{
			UpdateDisplays(adapter, _displays);
			adapter.Release();
		}

		return true;
	}

	std::shared_ptr<DuplicatedOutput::Device> DisplayManager::GetDevice(CComPtr<IDXGIAdapter1> adapter)
	{
		DXGI_ADAPTER_DESC1 spec;

		if (SUCCEEDED(adapter->GetDesc1(&spec)))
		{
			if (_devices.find(spec.DeviceId) == _devices.end())
			{
				return _devices[spec.DeviceId] = DuplicatedOutput::CreateDevice(adapter);
			}
			else
			{
				return _devices[spec.DeviceId];
			}
		}

		return nullptr;
	}


	bool DisplayManager::Update()
	{
		auto& cfg = Application::Config().Data();
		std::vector<std::string> defined;

		MAMBI_CFG_IS_OBJECT(cfg, "display", "config");
		
		for (auto& el : cfg["display"].items())
		{
			auto& key = el.key();
			auto& value = el.value();

			defined.push_back(key);

			if (!_displays[key].Update(key, value))
			{
				_displays.clear();
				return false;
			}

			if (value.count("profile") == 0 || value["profile"].is_null())
			{
				Application::Profile().Activate(key, nullptr);
			}
			else if (value["profile"].is_string())
			{
				Application::Profile().Activate(key, value["profile"].get<std::string>());
			}
			else
			{
				Application::Profile().Activate(key, nullptr);
				ErrorAlert("Error", "Invalid 'profile' type in display config, must be string");
			}
		}
			

		// remove unpresented configs
		std::vector<std::string> remove;
		for (auto& el : _displays)
		{
			if (std::find(defined.begin(), defined.end(), el.first) == defined.end())
			{
				remove.push_back(el.first);
			}
		}

		for (auto& el : remove)
		{
			_displays.erase(el);
		}

		UpdateOutputs();

		return true;
	}


	bool DisplayManager::UpdateDisplays(CComPtr<IDXGIAdapter1> adapter, DisplayManager::Displays& displays)
	{
		bool res = false;

		for (UINT16 i = 0; ; i++)
		{
			CComPtr<IDXGIOutput> output;
			if (FAILED(adapter->EnumOutputs(i, &output)))
			{
				break;
			}

			DXGI_OUTPUT_DESC desc;
			if (SUCCEEDED(output->GetDesc(&desc)) && desc.AttachedToDesktop)
			{
				Console::WriteLine("Desktop %S = left=%ld, top=%ld, right=%ld, bottom=%ld"
					, desc.DeviceName
					, desc.DesktopCoordinates.left
					, desc.DesktopCoordinates.top
					, desc.DesktopCoordinates.right
					, desc.DesktopCoordinates.bottom);

				DISPLAY_DEVICE device;
				ZeroMemory(&device, sizeof(device));
				device.cb = sizeof(device);
				for (UINT16 k=0; EnumDisplayDevices(desc.DeviceName, k, &device, 0); k++)
				{
					std::wstring deviceId(device.DeviceID);
					size_t pos = deviceId.find(L"\\");
					if (pos != std::string::npos)
					{
						pos = deviceId.find(L"\\", pos + 1);
						if (pos != std::string::npos)
						{
							deviceId = deviceId.substr(0, pos);
						}
					}

					std::string displayKey(deviceId.begin(), deviceId.end());

					if (displays.find(displayKey) != displays.end())
					{
						res = true;
						auto dev = GetDevice(adapter);
						auto& display = displays[displayKey];
						if (display.DupedOutput() == NULL)
						{
							display.DupedOutput(new DuplicatedOutput(dev, output, desc, device));
						}
					}

					Console::WriteLine("Device = %S  KEY = %s", deviceId.c_str(), displayKey.c_str());
				}

				/*
				MONITORINFOEX minfo;
				GetMonitorInfo(desc.Monitor, &minfo);
				Console::WriteLine("Output left=%ld, top=%ld, right=%ld, bottom=%ld"
					, minfo.rcWork.left
					, minfo.rcWork.top
					, minfo.rcWork.right
					, minfo.rcWork.bottom);
				*/
			}
		}

		return res;
	}
}
