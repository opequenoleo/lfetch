#include <windows.h>
#include <stdio.h>
#include <ctype.h>
#define MAX_LINES 6
#define MAX_LEN 128

char lines[MAX_LINES][MAX_LEN];

const char *pato[] = {
"      ,~~.",
" ,   (  0 )>",
" )`~~'   (",
"(  .__)   )",
" `-.____,'",
"~^~^~^`- ~~"
};

typedef struct {
	// MEMORY
	unsigned long long totalMemory;
	unsigned long long avaiableMemory;
	unsigned long long usedMemory;

	double totalMemoryGb;
	double usedMemoryGb;

	// UPTIME
	unsigned long long uptimeMs;
	unsigned long long seconds;
	unsigned long long hours;
	unsigned long long minutes;

	// USER
	char username[256];
	char hostname[256];

	unsigned long long totalStorage;
	unsigned long long usedStorage;

	char cpuName[256];

	char displayVersion[256];
	char productName[256];
} SystemInfo;

SystemInfo info;

bool enableAnsiSupport() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    if (!GetConsoleMode(hConsole, &mode)) return false;

    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    return SetConsoleMode(hConsole, mode) != 0;
};

int lowercase(char *str){
	for (int i = 0; str[i]; i++) {
		str[i] = tolower((unsigned char)str[i]);
	};
};

int printInfo(SystemInfo *info){
	snprintf(lines[0], MAX_LEN, "\e[36m%s@%s\e[0m", info->username, info->hostname);
	snprintf(lines[1], MAX_LEN, "\e[33m%-8s\e[0m %s %s", "os", info->productName, info->displayVersion);
	snprintf(lines[2], MAX_LEN, "\e[33m%-8s\e[0m %.20s", "cpu", info->cpuName);
	snprintf(lines[3], MAX_LEN, "\e[33m%-8s\e[0m %llu h %llu min", "uptime", info->hours, info->minutes);
	snprintf(lines[4], MAX_LEN, "\e[33m%-8s\e[0m %.2f GB / %.2f GB", "memory", info->usedMemoryGb, info->totalMemoryGb);
	snprintf(lines[5], MAX_LEN, "\e[33m%-8s\e[0m %llu GB / %llu GB", "storage", info->usedStorage, info->totalStorage);

	for (int i = 0; i < 6; i++) {
		if (i < 5)
			printf("\e[33m%-20s\e[0m", pato[i]);
		else
			printf("\e[36m%-20s\e[0m", pato[i]);
 
		printf(" %s", lines[i]);
		printf("\n");
	};
};

int getInfo() {
	MEMORYSTATUSEX mem;
	mem.dwLength = sizeof(mem);
	GlobalMemoryStatusEx(&mem);

	info.totalMemory = mem.ullTotalPhys;
	info.avaiableMemory = mem.ullAvailPhys;
	info.usedMemory = info.totalMemory - info.avaiableMemory;

	info.totalMemoryGb = info.totalMemory / (1024.0 * 1024 * 1024);
	info.usedMemoryGb = info.usedMemory / (1024.0 * 1024 * 1024);

	info.uptimeMs = GetTickCount64();
	info.seconds = info.uptimeMs / 1000;
	info.hours = info.seconds / 3600;
	info.minutes = (info.seconds % 3600) / 60;

	unsigned long usize = sizeof(info.username);
	GetUserNameA(info.username, &usize);
	lowercase(info.username);

	unsigned long hsize = sizeof(info.hostname);
	GetComputerNameA(info.hostname, &hsize);
	lowercase(info.hostname);

	ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFree;

	GetDiskFreeSpaceExA(
			"C:\\",
			&freeBytesAvailable,
			&totalBytes,
			&totalFree
	);

	info.totalStorage = totalBytes.QuadPart / (1024.0 * 1024 * 1024);
	info.usedStorage = (totalBytes.QuadPart - totalFree.QuadPart) / (1024.0 * 1024 * 1024);

	HKEY hKey;
	unsigned long size = sizeof(info.cpuName);

	RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
			"HARDWARE\\DESCRIPTION\\SYSTEM\\CentralProcessor\\0",
			0, KEY_READ, &hKey);

	RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL, (LPBYTE)info.cpuName, &size);

	RegCloseKey(hKey);
	hKey = NULL;

	RegOpenKeyExA(HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
			0,
			KEY_READ,
			&hKey
	);
	
	size = sizeof(info.productName);
	RegQueryValueExA(hKey, "ProductName", NULL, NULL, (LPBYTE)info.productName, &size);

	size = sizeof(info.displayVersion);
	RegQueryValueExA(hKey, "DisplayVersion", NULL, NULL, (LPBYTE)info.displayVersion, &size);
};

int main(){
	if (!enableAnsiSupport()) {
        printf("ANSI not supported.\n");
        return 1;
    }
	getInfo();
	printInfo(&info);
};
