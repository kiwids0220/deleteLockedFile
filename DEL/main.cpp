#include "main.h"
#include <iostream>
#include <memory>
static
HANDLE
ds_open_handle(
	LPCWSTR pwPath
)
{
	return CreateFileW(pwPath, DELETE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}

static
BOOL
ds_rename_handle(
	HANDLE hHandle
)
{
	const wchar_t* FileNameBBQ = L":wtfbbq";
	auto destFilenameLength = wcslen(FileNameBBQ);

	//LPBY_HANDLE_FILE_INFORMATION lpFileInformation = new BY_HANDLE_FILE_INFORMATION();
	 
	auto bufferSize = sizeof(FILE_RENAME_INFO) + (destFilenameLength * sizeof(wchar_t));
	auto buffer = _alloca(bufferSize);
	memset(buffer, 0, bufferSize);

	auto const fri = reinterpret_cast<FILE_RENAME_INFO*>(buffer);
	fri->ReplaceIfExists = TRUE;
	fri->RootDirectory = NULL;


	fri->FileNameLength = destFilenameLength;
	wmemcpy(fri->FileName, FileNameBBQ, sizeof(FileNameBBQ));
	//GetFileInformationByHandle(hHandle, lpFileInformation);
	BOOL res = SetFileInformationByHandle(hHandle, FileRenameInfo, fri, bufferSize);
	if (!res)
	{
		auto const err = GetLastError();
		std::cerr << "failed to rename file: " << err;
		return err;
	}
	else
		std::cout << "success";
	return 0;

}

static
BOOL
ds_deposite_handle(
	HANDLE hHandle
)
{
	// set FILE_DISPOSITION_INFO::DeleteFile to TRUE
	FILE_DISPOSITION_INFO fDelete;
	RtlSecureZeroMemory(&fDelete, sizeof(fDelete));

	fDelete.DeleteFile = TRUE;

	return SetFileInformationByHandle(hHandle, FileDispositionInfo, &fDelete, sizeof(fDelete));
}

int
main(
	int argc,
	char** argv
)
{
	const wchar_t wcPath[] = L"C:\\Users\\Labadmin\\Desktop\\delete-self-poc-main\\README.md";
	// get the path to the current running process ctx

	
	HANDLE hCurrent = CreateFileW(wcPath, DELETE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hCurrent == INVALID_HANDLE_VALUE)
	{
		DS_DEBUG_LOG(L"failed to acquire handle to current running process");
		return 0;
	}

	// rename the associated HANDLE's file name
	DS_DEBUG_LOG(L"attempting to rename file name");
	if (!ds_rename_handle(hCurrent))
	{
		DS_DEBUG_LOG(L"failed to rename to stream");
		return 0;
	}

	DS_DEBUG_LOG(L"successfully renamed file primary :$DATA ADS to specified stream, closing initial handle");
	CloseHandle(hCurrent);

	// open another handle, trigger deletion on close
	hCurrent = ds_open_handle(wcPath);
	if (hCurrent == INVALID_HANDLE_VALUE)
	{
		DS_DEBUG_LOG(L"failed to reopen current module");
		return 0;
	}

	if (!ds_deposite_handle(hCurrent))
	{
		DS_DEBUG_LOG(L"failed to set delete deposition");
		return 0;
	}

	// trigger the deletion deposition on hCurrent
	DS_DEBUG_LOG(L"closing handle to trigger deletion deposition");
	CloseHandle(hCurrent);

	// verify we've been deleted
	if (PathFileExistsW(wcPath))
	{
		DS_DEBUG_LOG(L"failed to delete copy, file still exists");
		return 0;
	}

	DS_DEBUG_LOG(L"successfully deleted self from disk");
	return 1;
}
