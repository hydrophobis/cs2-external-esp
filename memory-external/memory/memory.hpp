#ifndef _PPROCESS_HPP_
#define _PPROCESS_HPP_

#include <vector>
#include <math.h>
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <iostream>
#include <Psapi.h> 

typedef NTSTATUS(WINAPI* pNtReadVirtualMemory)(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, ULONG NumberOfBytesToRead, PULONG NumberOfBytesRead);
typedef NTSTATUS(WINAPI* pNtWriteVirtualMemory)(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, ULONG NumberOfBytesToWrite, PULONG NumberOfBytesWritten);

class pMemory {

public:
	pMemory() {
		pfnNtReadVirtualMemory = (pNtReadVirtualMemory)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtReadVirtualMemory");
		pfnNtWriteVirtualMemory = (pNtWriteVirtualMemory)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWriteVirtualMemory");
	}

	pNtReadVirtualMemory pfnNtReadVirtualMemory;
	pNtWriteVirtualMemory pfnNtWriteVirtualMemory;
};

struct ProcessModule
{
	uintptr_t base, size;
};

class pProcess
{
public:
	DWORD		  pid_; // process id
	HANDLE		  handle_; // handle to process
	HWND		  hwnd_; // window handle
	ProcessModule base_client_;

public:
	bool AttachProcess(const char* process_name);
	bool AttachProcessHj(const char* process_name);
	bool AttachWindow(const char* window_name);
	bool UpdateHWND();
	void Close();

public:
	ProcessModule GetModule(const char* module_name);
	LPVOID		  Allocate(size_t size_in_bytes);
	uintptr_t	  FindCodeCave(uint32_t length_in_bytes);
	uintptr_t     FindSignature(std::vector<uint8_t> signature);
	uintptr_t     FindSignature(ProcessModule target_module, std::vector<uint8_t> signature);

	template<class T>
	uintptr_t ReadOffsetFromSignature(std::vector<uint8_t> signature, uint8_t offset) // offset example: "FF 05 ->22628B01<-" offset is 2
	{
		uintptr_t pattern_address = this->FindSignature(signature);
		if (!pattern_address)
			return 0x0;

		T offset_value = this->read<T>(pattern_address + offset);
		return pattern_address + offset_value + offset + sizeof(T);
	}

	bool read_raw(uintptr_t address, void* buffer, size_t size)
	{
		SIZE_T bytesRead;
		pMemory cMemory;

		if (cMemory.pfnNtReadVirtualMemory(this->handle_, (PVOID)(address), buffer, static_cast<ULONG>(size), (PULONG)&bytesRead))
		{
			return bytesRead == size;
		}
		return false;
	}

	template<class T>
	T read(uintptr_t address)
	{
		T buffer{};
		pMemory cMemory;

		cMemory.pfnNtReadVirtualMemory(handle_, (void*)address, &buffer, sizeof(T), 0);
		return buffer;
	}

	template<class T>
	bool write(uintptr_t address, const T& value)
	{
		MEMORY_BASIC_INFORMATION mbi;
		if (VirtualQueryEx(handle_, (LPVOID)address, &mbi, sizeof(mbi)) == 0 || mbi.State != MEM_COMMIT)
		{
			std::cerr << "Invalid or non-committed memory region." << std::endl;
			return false;
		}

		DWORD oldProtect;
		if (!VirtualProtectEx(handle_, (LPVOID)address, sizeof(T), PAGE_READWRITE, &oldProtect))
		{
			std::cerr << "VirtualProtectEx failed with error: " << GetLastError() << std::endl;
			return false;
		}

		// Use WriteProcessMemory instead of NtWriteVirtualMemory
		SIZE_T bytesWritten = 0;
		BOOL result = WriteProcessMemory(handle_, (LPVOID)address, &value, sizeof(T), &bytesWritten);

		// Restore protection
		VirtualProtectEx(handle_, (LPVOID)address, sizeof(T), oldProtect, &oldProtect);

		if (!result || bytesWritten != sizeof(T))
		{
			std::cerr << "WriteProcessMemory failed with error: " << GetLastError() << std::endl;
			return false;
		}

		return true;
	}



	void write_bytes(uintptr_t addr, const std::vector<uint8_t>& patch)
	{
		if (patch.empty())
			return;

		pMemory cMemory;
		DWORD oldProtect;

		MEMORY_BASIC_INFORMATION mbi;
		if (VirtualQueryEx(handle_, (LPCVOID)addr, &mbi, sizeof(mbi)) == sizeof(mbi)) {
			std::cout << "Protect: " << std::hex << mbi.Protect
				<< " State: " << mbi.State
				<< " Type: " << mbi.Type << std::endl;
		}

		if (VirtualProtectEx(handle_, (LPVOID)addr, patch.size(), PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			ULONG bytesWritten = 0;
			NTSTATUS status = cMemory.pfnNtWriteVirtualMemory(handle_, (void*)addr,
				(void*)patch.data(),
				(ULONG)patch.size(),
				&bytesWritten);

			if (status != 0 || bytesWritten != patch.size())
			{
				printf("Write failed! Status: %lx, BytesWritten: %lu\n", status, bytesWritten);
			}

			// Restore protection
			VirtualProtectEx(handle_, (LPVOID)addr, patch.size(), oldProtect, &oldProtect);
		}
		else
		{
			printf("VirtualProtectEx failed on write_bytes at %p\n", (void*)addr);
		}
	}


	uintptr_t read_multi_address(uintptr_t ptr, std::vector<uintptr_t> offsets)
	{
		uintptr_t buffer = ptr;
		for (int i = 0; i < offsets.size(); i++)
			buffer = this->read<uintptr_t>(buffer + offsets[i]);

		return buffer;
	}

	template <typename T>
	T read_multi(uintptr_t base, std::vector<uintptr_t> offsets) {
		uintptr_t buffer = base;
		for (int i = 0; i < offsets.size() - 1; i++)
		{
			buffer = this->read<uintptr_t>(buffer + offsets[i]);
		}
		return this->read<T>(buffer + offsets.back());
	}

private:
	uint32_t FindProcessIdByProcessName(const char* process_name);
	uint32_t FindProcessIdByWindowName(const char* window_name);
	HWND GetWindowHandleFromProcessId(DWORD ProcessId);
};
#endif