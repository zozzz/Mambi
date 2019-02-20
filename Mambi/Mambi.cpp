#include "stdafx.h"
#include "Mambi.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	if (!Mambi::Application::Instance().Init(hInstance))
	{
		return FALSE;
	}

	return Mambi::Application::Instance().Run();
}

