/*
	Screen monitoring utility - written in 2017 by Joseph J. Keller 
	Monitors a given program window by making a bitmap of its contents, hashing the bitmap,
	and comparing new and old hashes to see if the window contents have changed during a
	given period.

	By default it'll check the whole window region, but you can pass it the x,y of the top
	left corner of the region you wish to check, along with the width and height of that
	region. Not useful under normal circumstances, but when used with another application
	or a scripting engine like AutoHotKey, it can be automated to keep this program looking
	at the correct screen region.

	A lot of this code is borrowed and modified from various people across the internet.
	Many thanks to their help. I wish I could attribute them all but I spent far too much
	time writing this before I had that thought...
*/

#include <Windows.h>
#include <iostream>
#include <sstream>
#include <string>
#include <math.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <boost\functional\hash.hpp>
#include <tclap\cmdline.h>
#using <System.dll>
using namespace std;
using namespace TCLAP;
using namespace System;
using namespace System::Diagnostics;
using namespace System::ComponentModel;

//from here

DWORD FindProcessId(const std::wstring& processName);

struct handle_data {
	unsigned long process_id;
	HWND best_handle;
};

BOOL is_main_window(HWND handle)
{
	return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}

BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam)
{
	handle_data& data = *(handle_data*)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	if (data.process_id != process_id || !is_main_window(handle)) {
		return TRUE;
	}
	data.best_handle = handle;
	return FALSE;
}

HWND find_main_window(unsigned long process_id)
{
	handle_data data;
	data.process_id = process_id;
	data.best_handle = 0;
	EnumWindows(enum_windows_callback, (LPARAM)&data);
	return data.best_handle;
}

//to here is all for finding the PID of a process by process name...

HBITMAP GetScreenBmp(HDC hdc, HWND hwnd, int x, int y, int h, int w) { //grab the bitmap of the specified window
	// Get window dimensions
	if ((x & y & h & w) == 0) { //checking if x, y, h, and w actually have non-default values

	//if not, just default to the whole window region instead
		RECT rect;
		if (GetWindowRect(hwnd, &rect))
		{
			w = rect.right - rect.left;
			h = rect.bottom - rect.top;
		}
	}

	// Create compatible DC, create a compatible bitmap and copy the screen using BitBlt()
	HDC hCaptureDC = CreateCompatibleDC(hdc);
	HBITMAP hBitmap = CreateCompatibleBitmap(hdc, w, h);
	HGDIOBJ hOld = SelectObject(hCaptureDC, hBitmap);
	BOOL bOK = BitBlt(hCaptureDC, 0, 0, w, h, hdc, x, y, SRCCOPY | CAPTUREBLT);

	SelectObject(hCaptureDC, hOld); // always select the previously selected object once done
	DeleteDC(hCaptureDC);
	return hBitmap;
}

int getScreenHash(HWND hwnd, int x, int y, int h, int w, bool verboseTrue) { //grab the screen hash for a specified window
	HDC hdc = GetDC(hwnd);

	HBITMAP hBitmap = GetScreenBmp(hdc, hwnd, x, y, h, w); //pass the hdc and hwnd to the bitmap grabber

	BITMAPINFO MyBMInfo = { 0 };
	MyBMInfo.bmiHeader.biSize = sizeof(MyBMInfo.bmiHeader);

	// Get the BITMAPINFO structure from the bitmap
	if (0 == GetDIBits(hdc, hBitmap, 0, 0, NULL, &MyBMInfo, DIB_RGB_COLORS)) {
		if (verboseTrue) {
			cout << "Did you close the target application?" << endl;
			cout << "ScreenMon will now exit." << endl;
		}
		return 2;
	}

	// create the bitmap buffer
	BYTE* lpPixels = new BYTE[MyBMInfo.bmiHeader.biSizeImage];

	// Better do this here - the original bitmap might have BI_BITFILEDS, which makes it
	// necessary to read the color table - you might not want this.
	MyBMInfo.bmiHeader.biCompression = BI_RGB;

	int size = MyBMInfo.bmiHeader.biSizeImage; //grab the size of the bitmap buffer

	// get the actual bitmap buffer
	if (0 == GetDIBits(hdc, hBitmap, 0, MyBMInfo.bmiHeader.biHeight, (LPVOID)lpPixels, &MyBMInfo, DIB_RGB_COLORS)) {
		cout << "error2" << endl;
	}

	if (verboseTrue)
		cout << size << endl;
	int screenHash = NULL; 
	for (int i = 0; i < size; i++) { //take the bitmap buffer, convert the bytes (UChar) to ints, and append them to a new variable
		screenHash = screenHash + (int)lpPixels[i];
	}

	// cout << "Debug: " << endl;
	// for (int i = 0; i < size; i++) {
	//	cout << (int)lpPixels[i];
	//}
	//cout << endl;

	DeleteObject(hBitmap);
	ReleaseDC(NULL, hdc);
	delete[] lpPixels;
	return screenHash;
}

int main(int argc, char** argv) {
	wstring processName;
	bool verboseTrue;
	string tempInterval = "1000";
	int checkInterval = NULL;
	HWND hwnd = NULL;
	int x = 0;
	int y = 0;
	int h = 0;
	int w = 0;

	try {
		//setting up the command line switches
		string programVersion = "0.1";
		CmdLine cmd("ProgramMonitor; monitors a screen region to see if it changes.", ' ', programVersion, false); //help text, delimiter, version number, switch override

		//arguments section
		ValueArg<string> progArg("p", "program", "program to monitor", true, "obs64.exe", "string"); //arg char, short name, long name, whether the switch can be empty, arg, val type
		ValueArg<string> checkArg("i", "interval", "check interval in ms; default is 1000", false, "1000", "int");
		ValueArg<string> xArg("x", "topleftx", "X coordinate of top left of screen region", false, "1", "int");
		ValueArg<string> yArg("y", "toplefty", "y coordinate of top left of screen region", false, "1", "int");
		ValueArg<string> hArg("h", "height", "height of the screen region you wish to select", false, "1", "int");
		ValueArg<string> wArg("w", "width", "width of the screen region you wish to select", false, "1", "int");
		cmd.add(progArg); //add the arguments
		cmd.add(checkArg);
		cmd.add(xArg);
		cmd.add(yArg);
		cmd.add(hArg);
		cmd.add(wArg);

		//switches section
		SwitchArg verboseSwitch("", "verbose", "print more debug data than normal", cmd, false); //arg char, short name, long name, call to cmd array, whether switch val can be empty

		cmd.parse(argc,argv); //parse the array when you're done

		//get the argument values
		processName.assign((progArg.getValue()).begin(), (progArg.getValue()).end()); //what program to monitor; have to do string -> wstring conversion for ~reasons~
		verboseTrue = verboseSwitch.getValue(); //verboseness
		if (checkArg.getValue() != "")
			tempInterval = checkArg.getValue();
		int checkInterval = stoi(tempInterval);

		if (verboseTrue)
			cout << "checkInterval is: " << checkInterval << endl;

		//now onto the main program

		DWORD pid = FindProcessId(processName); //call FindProcessID to grab the PID of the program
		string processNameTemp;
		processNameTemp.assign(processName.begin(), processName.end());

		if (pid == 0) { //exit with a code of 2 if the target application isn't found
			if (verboseTrue) {
				cout << "The target application " << processNameTemp << " could not be found, or isn't running!" << endl;
				cout << "ScreenMon will now exit." << endl;
			}
			return 2;
		}

		hwnd = find_main_window(pid); //get hwnd of the main window of the program

		if (verboseTrue) { //print the pid and hwnd out for debug
			cout << "The PID of " << processNameTemp << " is: " << pid << endl;
			cout << "And the hwnd is: " << hwnd << endl;
		}

		int oldScreenHash = NULL;
		cout << "Starting up..." << endl;

		while (true) {
			if (IsIconic(hwnd)) { //check if the program is minimized
				ShowWindow(hwnd, 9); //restore it if it is
			}

			int screenHash = getScreenHash(hwnd, x, y, h, w, verboseTrue); //get the screenhash variable from the hashing function, passing the hwnd to it

			if (verboseTrue)
				cout << oldScreenHash << " " << screenHash << endl;

			if (oldScreenHash != NULL) { //compare the hashes to see if the window region has changed
				
				bool isHung = IsHungAppWindow(hwnd);
				if (isHung) {
					return 3; //if the target window is hung (according to windows), exit with a code of 3.
				}
				if (oldScreenHash != screenHash) {
					cout << true;
					if (verboseTrue)
						cout << " I WORK" << endl; //as if the 0's and 1's flooding the console aren't obvious.
				}
				else if (oldScreenHash == screenHash) {
					cout << false;
					if (verboseTrue)
						cout << " I DON'T WORK" << endl;
				}
				else {
					cout << 3;
					if (verboseTrue)
						cout << " I'm broken?" << endl;
				}
			}

			oldScreenHash = screenHash; //set the old hash equal to the new hash for the next iteration of the loop

			Sleep(checkInterval);
		}
	}
	catch (ArgException &e)  //catch argument exceptions
	{
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
	}

}

DWORD FindProcessId(const std::wstring& processName) //FindProcessID function
{
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processesSnapshot == INVALID_HANDLE_VALUE)
		return 0;

	Process32First(processesSnapshot, &processInfo);
	if (!processName.compare(processInfo.szExeFile))
	{
		CloseHandle(processesSnapshot);
		return processInfo.th32ProcessID;
	}

	while (Process32Next(processesSnapshot, &processInfo))
	{
		if (!processName.compare(processInfo.szExeFile))
		{
			CloseHandle(processesSnapshot);
			return processInfo.th32ProcessID;
		}
	}

	CloseHandle(processesSnapshot);
	return 0;
}