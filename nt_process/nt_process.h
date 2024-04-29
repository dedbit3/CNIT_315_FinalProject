#pragma once
#include <windows.h>
#include <stdio.h>

#define STATUS_SUCCESS (NTSTATUS)0x00000000L
#define okay(msg, ...) printf("[+] " msg "\n", ##__VA_ARGS__)
#define info(msg, ...) printf("[i] " msg "\n", ##__VA_ARGS__)
#define warn(msg, ...) printf("[-] " msg "\n", ##__VA_ARGS__)

/* defining structures...*/

typedef struct _OBJECT_ATTRIBUTES {
	ULONG Length;
    	VOID* RootDirectory;
    	struct _UNICODE_STRING* ObjectName;
    	ULONG Attributes;
    	VOID* SecurityDescriptor;
    	VOID* SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef struct _CLIENT_ID {
	VOID* UniqueProcess;
	VOID* UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef struct _PS_ATTRIBUTE {
	ULONGLONG Attributes;
	SIZE_T Size;
	union {
		ULONG_PTR Value;
		PVOID ValuePtr;
	};
	PSIZE_T ReturnLength;
} PS_ATTRIBUTE, *PPS_ATTRIBUTES;

typedef struct _PS_ATTRIBUTE_LIST {
	SIZE_T TotalLength;
	PS_ATTRIBUTE Attributes[1];
} PS_ATTRIBUTE_LIST, *PPS_ATTRIBUTE_LIST;


/* function prototype declarations...*/

typedef NTSTATUS (NTAPI* NtOpenProcess) (
	_Out_ PHANDLE ProcessHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ POBJECT_ATTRIBUTES ObjectAttributes,
	_In_opt_ PCLIENT_ID ClientId
);

typedef NTSTATUS(NTAPI* NtClose) (
	_In_ _Post_ptr_invalid_ HANDLE Handle
);

