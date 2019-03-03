// AmbilightTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>


Mambi::SampleDesc hDesc = {
	/* HPadding */ 0,
	/* VPadding */ 0,
	/* Width */    120,
	/* Height */   70,
	/* Count */    24
};

Mambi::SampleDesc vDesc = {
	/* HPadding */ 0,
	/* VPadding */ 0,
	/* Width */    70,
	/* Height */   120,
	/* Count */    10
};


void recreate()
{
	char fnAck[255];
	char fnSample[255];

	for (int i = 0; i < 10; i++)
	{
		auto ambilight = new Mambi::Ambilight("MONITOR\\GSM7727");
		ambilight->UpdateSamplesDesc(hDesc, vDesc);
		
		printf("Sleep 1000...\n");
		Sleep(1000);

		//sprintf_s(fnAck, "D:\\Workspace\\Mambi\\screenshot\\test-%d.png", i);
		//sprintf_s(fnSample, "D:\\Workspace\\Mambi\\screenshot\\sample-%d.png", i);

		ambilight->ProcessSamples();
		
		delete ambilight;
	}
}


void cached()
{
	auto ambilight = new Mambi::Ambilight("MONITOR\\GSM7727");
	ambilight->UpdateSamplesDesc(hDesc, vDesc);
	
	while (true)
	{		
		//printf("Sleep 33...\n");
		Sleep(33);

		ambilight->ProcessSamples();
	}

	delete ambilight;
}


int main()
{
	cached();
	//recreate();
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
