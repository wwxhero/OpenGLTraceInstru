// GLRetFuncGen.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "GenGLRetFuncs.h"

int _tmain(int argc, _TCHAR* argv[])
{
	const char* srcPath = argv[1];
	const char* declarePath = argv[2];
	const char* definiPath = argv[3];
	//printf("Source: %s\n"
	//	   "Delcaration: %s\n"
	//	   "Definition: %s\n", srcPath, declarePath, definiPath);
	//void StartParse4Funcs(const char* szPath, MemSrc* mem, std::list<Func*>& lstFuncs);
	MemSrc mem;
	std::list<Func*> lstFuncs;
	StartParse4Funcs(srcPath, &mem, lstFuncs);
	//void GenFuncsDecl(const char* szPath, const MemSrc* mem, const std::list<Func*>& lstFuncs);
	GenFuncsDecl(declarePath, &mem, lstFuncs);
	//void GenFuncsImpl(const char* szPath, const MemSrc* mem, const std::list<Func*>& lstFuncs);
	//GenFuncsImpl(definiPath, &mem, lstFuncs);
	//void EndParse4Funcs(MemSrc* mem);
	EndParse4Funcs(&mem, lstFuncs);
	return 0;
}

