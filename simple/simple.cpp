#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "../zip.h"
#include "../unzip.h"
#include <string>
#include "shlwapi.h"
#pragma comment(lib,"Shlwapi.lib")

DWORD PackFolderToZip(HZIP ZipFile, const std::string& DestFile,const std::string& SourceFile)
{
	WIN32_FIND_DATAA file_dta;
	std::string filePath = SourceFile;
	std::string DestFilePath = DestFile;
	CHAR szFullFilePath[MAX_PATH];
	DWORD dwEerror = NULL;
	CHAR* findStr = nullptr;
	int lens = NULL;
	ZeroMemory(&file_dta, sizeof(WIN32_FIND_DATAA));
	ZeroMemory(szFullFilePath, MAX_PATH);
	lstrcpyA(szFullFilePath, filePath.c_str());
	findStr = StrRStrIA(filePath.c_str(), NULL, "\\");
	if (!findStr)
		return dwEerror;
	lens = lstrlenA(findStr);
	if (lens > 1)
	{
		lstrcatA(szFullFilePath, "\\*.*");
	}
	else if (lens == 1)
	{
		lstrcatA(szFullFilePath, "*.*");
	}
	else
	{
		return dwEerror;
	}
	HANDLE hFile = INVALID_HANDLE_VALUE;
	hFile = FindFirstFileA(szFullFilePath, &file_dta);
	dwEerror = ::GetLastError();
	if (INVALID_HANDLE_VALUE == hFile)
		return dwEerror;
	while (::FindNextFileA(hFile, &file_dta))
	{
		if (!lstrcmpiA(file_dta.cFileName, ".") || !lstrcmpiA(file_dta.cFileName, ".."))
			continue;
		sprintf_s(szFullFilePath, "%s\\%s", filePath.c_str(), file_dta.cFileName);
		if (file_dta.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{

			//ZipAddFolder(ZipFile, file_dta.cFileName);
			DestFilePath = DestFile +"\\"+file_dta.cFileName;
			PackFolderToZip(ZipFile, DestFilePath,szFullFilePath);
			continue;
		}
		else
		{
			std::string temp_file  = DestFile + "\\"+file_dta.cFileName;
			ZipAdd(ZipFile, temp_file.c_str(), szFullFilePath);
		}
	}
	::FindClose(hFile);
	return dwEerror;
}

bool CompressToPackageZip(const std::string& SourceFile, const std::string& DestFile, const std::string& PassWord, BOOL IsFolder = TRUE)
{
	bool bResult = false;
	HZIP zipFile = CreateZip(DestFile.c_str(), PassWord.c_str());
	//创建一个zip压缩包文件 失败返回null
	if (IsFolder)
	{
		bResult = PackFolderToZip(zipFile,"", SourceFile);
	}
	else
	{
		std::string dest_file_name = SourceFile.substr(SourceFile.find_last_of("/\\") + 1);
		ZipAdd(zipFile, dest_file_name.c_str(), SourceFile.c_str()); //向压缩包中添加
	}
	CloseZip(zipFile);
	return bResult;
}

bool DecompressToPackage(const std::string& SourceFile, const std::string& DestFile, const std::string& PassWord)
{
	bool bResult = false;
	HZIP hz;
	hz = OpenZip(SourceFile.c_str(), PassWord.c_str());
	SetUnzipBaseDir(hz, DestFile.c_str());
	ZIPENTRY ze;
	GetZipItem(hz, -1, &ze);
	int numitems = ze.index;
	for (int zi = 0; zi < numitems; zi++)
	{
		GetZipItem(hz, zi, &ze);
		UnzipItem(hz, zi, ze.name);
	}
	CloseZip(hz);
	bResult = true;

	return bResult;
}

bool DecompressToBuffer(const std::string& SourceFile, std::string* pbuf, const std::string& PassWord)
{
	bool bResult = false;
	do 
	{
		HZIP hz = OpenZip(SourceFile.c_str(), PassWord.c_str());
		ZIPENTRY ze;
		int i = 0;
		if (GetZipItem(hz, 0, &ze) != ZR_OK)//获取第一项文件内容
		{
			printf("GetZipItem 解压到内存失败");
			break;
		}
		char* fbuf = new char[ze.unc_size];
		if (UnzipItem(hz, i, fbuf, ze.unc_size) != ZR_OK)
		{
			printf("GetZipItem 解压到内存失败");
			break;
		}
		*pbuf = fbuf;
		CloseZip(hz);
		bResult = true;

	} while (FALSE);

	return bResult;
}

const char* compress_file_name = "CompressFile.txt";
const char* compressed_file_name = "CompressFile.zip";
const char* decompress_file_name = "decompressFile";


int main()
{

	char current_path[MAX_PATH] = { 0 };
	char compress_file_path[MAX_PATH] = { 0 };
	char compressed_file_path[MAX_PATH] = { 0 };

	GetModuleFileNameA(NULL, current_path, MAX_PATH);
	PathRemoveFileSpecA(current_path);

	strcpy_s(compress_file_path, current_path);
	PathAppendA(compress_file_path, compress_file_name);
	//输入待压缩文件
	printf("待加密文件:%s", compress_file_path);

	
	strcpy_s(compressed_file_path, current_path);
	PathAppendA(compressed_file_path, compressed_file_name);
	//输出压缩后的文件
	printf("待加密文件:%s", compressed_file_path);

	// EXAMPLE 1 - create a zipfile from existing files
	CompressToPackageZip(compress_file_path, compressed_file_path, "12345",FALSE);

	// EXAMPLE 2 - unzip it with the names suggested in the zip
	std::string dest_file = std::string(current_path) + "\\"+decompress_file_name;
	DecompressToPackage(compressed_file_path, dest_file, "12345"); //deCompressFile

	//解压单个文件到内存 注意调用者清除内存
	std::string* pbuf = new std::string;
	DecompressToBuffer(compressed_file_path, pbuf, "12345"); //deCompressFile
	pbuf->clear();


	return 0;
}


