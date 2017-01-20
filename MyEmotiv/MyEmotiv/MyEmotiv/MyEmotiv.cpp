// MyEmotiv.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "IEmoStateDLL.h"
#include "Iedk.h"
#include "IedkErrorCode.h"
#include <conio.h>
#pragma comment(lib, "Ws2_32.lib")
///////Windows Libraries///////
#define _WIN32_WINNT 0x0600
#define _WIN32_IE 0x0700
#define _UNICODE
#define _AFXDLL

#include <afxwin.h>
#include <shobjidl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <propvarutil.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "propsys.lib")
#pragma once
/////////////////////////////////////

using namespace std;

bool isCooldown;
UINT cooldown(LPVOID pParam)
{
	int* path = (int*)pParam;
	isCooldown = true;
	Sleep(*path);
	isCooldown = false;
	return 0;
}
int main(int argc, char **argv)
{
	string receiverHost = "localhost";
	if (argc > 2) {
		cout << "Usage: " << argv[0] << " <hostname>" << endl;
		cout << "The arguments specify the host of the head model "
			"(Default: localhost)" << endl;
		return 1;
	}
	if (argc > 1) {
		receiverHost = string(argv[1]);
	}

	EmoEngineEventHandle eEvent = IEE_EmoEngineEventCreate();
	EmoStateHandle eState = IEE_EmoStateCreate();
	unsigned int userID = 0;
	string path = "../../../TextFile/status.txt";
	int cooldownTime = 2000;
	int* cooldownPtr = &cooldownTime;
	ofstream file;
	const int CONTROL_PANEL_PORT = 3008;
	try {
		if (IEE_EngineConnect() != EDK_OK) {
			throw runtime_error("Emotiv Driver start up failed.");
		}
		else {
			cout << "Emotiv Driver started!" << endl;
		}

		int startSendPort = 30000;
		cout << "Prepare to scan for winks" << endl;
		while (true) {
			int state = IEE_EngineGetNextEvent(eEvent);
			if (isCooldown) continue;
			if (state == EDK_OK) {
				IEE_Event_t eventType = IEE_EmoEngineEventGetType(eEvent);
				IEE_EmoEngineEventGetUserId(eEvent, &userID);
				switch (eventType) {
				case IEE_UserAdded:
				{
					cout << endl << "New user " << userID
						<< " added, sending FacialExpression animation to ";
					cout << receiverHost << ":" << startSendPort << "..."
						<< endl;
					++startSendPort;
					break;
				}
				case IEE_UserRemoved:
				{
					cout << endl << "User " << userID
						<< " has been removed." << endl;
					break;
				}
				case IEE_EmoStateUpdated:
				{
					IEE_EmoEngineEventGetEmoState(eEvent, eState);
					bool leftWink = IS_FacialExpressionIsLeftWink(eState);
					bool rightWink = IS_FacialExpressionIsRightWink(eState);
					if (leftWink || rightWink) {
						AfxBeginThread(cooldown, (LPVOID)cooldownPtr);
						cout << "Wink" << endl;
						file.open(path, ios::out);
						file.clear();
						file << "1";
						file.close();
					}
					break;
				}
				case IEE_FacialExpressionEvent: {}
				default:
					break;
				}
			}
			else if (state != EDK_NO_EVENT) {
				cout << endl << "Internal error in Emotiv Engine!"
					<< endl;
				break;
			}
			//Sleep(15);
		}
	}
	catch (const runtime_error& e) {
		cerr << e.what() << endl;
		cout << "Press 'Enter' to exit..." << endl;
		getchar();
	}
	system("pause");
	return 0;
}

