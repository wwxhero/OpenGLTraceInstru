#pragma once
#ifndef _GLTRACE_INJECTOR_H
#define _GLTRACE_INJECTOR_H
#include <set>
#include <list>
//GLAPI GLboolean APIENTRY glIsProgram (GLuint program);
//typedef GLboolean (APIENTRY* GLISPROGRAM) (GLuint program);
//_GLTRACER_API GLboolean GLTrace_glIsProgram (GLISPROGRAM proc, GLuint program, const char* fileName, unsigned int lineNum);
typedef struct Func_t
{
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

void StartParse4Funcs(const char* szPath, MemSrc* mem, std::list<Func*>& lstFuncs);
void GenFuncsDecl(const char* szPath, const MemSrc* mem, const std::list<Func*>& lstFuncs);
void GenFuncsImpl(const char* szPath, const MemSrc* mem, const std::list<Func*>& lstFuncs);
void EndParse4Funcs(MemSrc* mem);

#endif