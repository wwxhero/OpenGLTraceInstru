// GLRetFuncGen.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "GenGLRetFuncs.h"

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 4)
	{
		printf ("GLRetFuncGen <glHeader> <header generated> <source generated>\n");
		return -1;
	}
	const char* srcPath = argv[1];
	const char* declarePath = argv[2];
	const char* definiPath = argv[3];
	MemSrc mem;
	std::list<Func*> lstFuncs;
	StartParse4Funcs(srcPath, &mem, lstFuncs);
	GenFuncsDecl(declarePath, &mem, lstFuncs);
	GenFuncsImpl(definiPath, &mem, lstFuncs);
	EndParse4Funcs(&mem, lstFuncs);
	return 0;
}

