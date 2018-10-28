// GLTraceInstruInjector.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "GLTraceInjector.h"
#include <iostream>
#include <fstream>
#include <sstream>


struct MemSrc
{
	const unsigned char* p;
	unsigned int size;
	HANDLE hFile;
	HANDLE hMapFile;
};

bool LoadFile(const char* filePath, MemSrc* mem)
{
	mem->hFile = CreateFileA(filePath,               // file name
                       GENERIC_READ,          // open for reading
                       0,                     // do not share
                       NULL,                  // default security
                       OPEN_EXISTING,         // existing file only
                       FILE_ATTRIBUTE_NORMAL, // normal file
                       NULL);                 // no template

   	bool ok = (INVALID_HANDLE_VALUE != mem->hFile);
   	if (ok)
   	{
   		mem->size = GetFileSize(mem->hFile, 0);
   		mem->hMapFile = CreateFileMapping(
                 mem->hFile,    				// use paging file
                 NULL,                  // default security
                 PAGE_READONLY,        // read/write access
                 0,						// maximum object size (high-order DWORD)
                 mem->size,						// maximum object size (low-order DWORD)
                 NULL);                 // name of mapping object
   		ok = (mem->hMapFile != NULL);
   	}

   	if (ok)
   	{
   		mem->p = (const unsigned char*) MapViewOfFile(mem->hMapFile,   // handle to map object
                    	    				FILE_MAP_READ, // read/write permission
                    	    				0,
                    	    				0,
                    	    				mem->size);
   	}
   	else
   	{
   		mem->p = NULL;
   		mem->size = 0;
   		mem->hFile = INVALID_HANDLE_VALUE;
   		mem->hMapFile = NULL;
   	}
   	return ok;
}

void UnLoad(MemSrc* mem)
{
	UnmapViewOfFile(mem->p);

   	CloseHandle(mem->hMapFile);
   	CloseHandle(mem->hFile);
   	mem->size = 0;

}


void Inject(const char* filePath)
{
	MemSrc mem;
	bool loaded = LoadFile(filePath, &mem);
	ASSERT(loaded);		

	UnLoad(&mem);
}