#pragma once
#ifndef _GENGLRET_FUNCS_H
#define _GENGLRET_FUNCS_H
#include <list>
#ifndef GLFUNCS_EXPORTS
#define FUNCS_API __declspec(dllimport)
#else
#define FUNCS_API __declspec(dllexport)
#endif
typedef struct Func_t
{
	const char* version[2];
	const char* retType[2];
	const char* callConven[2];
	const char* funcName[2];
	typedef struct Param_t
	{
		const char* paramType[2];
		const char* paramName[2];
	} Param;
	Param* params;
	unsigned int numParams;
} Func;

typedef struct MemSrc_t
{
	const char* p;
	unsigned int size;
	HANDLE hFile;
	HANDLE hMapFile;
} MemSrc;

FUNCS_API void StartParse4Funcs(const char* szPath, MemSrc* mem, std::list<Func*>& lstFuncs);
FUNCS_API void GenFuncsDecl(const char* szPath, const MemSrc* mem, const std::list<Func*>& lstFuncs);
FUNCS_API void GenFuncsImpl(const char* szPath, const MemSrc* mem, const std::list<Func*>& lstFuncs);
FUNCS_API void EndParse4Funcs(MemSrc* mem, std::list<Func*>& lstFuncs);


#endif