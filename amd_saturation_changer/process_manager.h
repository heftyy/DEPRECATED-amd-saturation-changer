#pragma once
#include <cstdio>
#include <windows.h>
#include <tlhelp32.h>
#include <string>

class process_manager
{
private:
	PROCESSENTRY32 entry;
	HANDLE snapshot;
public:
	process_manager() 
	{
		entry.dwSize = sizeof(PROCESSENTRY32);

		snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	}

	boolean is_process_running(std::string process_name)
	{
		bool process_found = false;
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

		if (Process32First(snapshot, &entry) == TRUE)
		{
			while (Process32Next(snapshot, &entry) == TRUE)
			{
				char DefChar = ' ';
				char iter_process_name[260];
				WideCharToMultiByte(CP_ACP, 0, entry.szExeFile, -1, iter_process_name, 260, &DefChar, NULL);

				if (stricmp(iter_process_name, process_name.c_str()) == 0)
				{
					process_found = true;
					break;
				}
			}
		}

		CloseHandle(snapshot);

		return process_found;
	}
};