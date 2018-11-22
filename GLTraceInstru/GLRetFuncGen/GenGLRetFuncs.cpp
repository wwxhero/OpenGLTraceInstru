// GLTraceInstruInjector.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "GenGLRetFuncs.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <list>
#include <assert.h>
#include <vector>
#include "PatternMatch.h"


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
   		mem->p = (const char*) MapViewOfFile(mem->hMapFile,   // handle to map object
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


void PatternMatchIntera(const char*& p, unsigned int& size, std::list<Func*>& lstFuncs)
{
	if (size > 0)
	{
		FixMatch m1(FIX_MATCH_CONSTRU("GLAPI"));
		BlankMatch m_blank;
		BlankMatchPlus m2(&m_blank);
		TypeMatch m3;
		BlankMatchStar m4(&m_blank);
		FixMatch m5(FIX_MATCH_CONSTRU("APIENTRY"));
		BlankMatchPlus m6(&m_blank);
		NameMatch m7;
		BlankMatchStar m8(&m_blank);
		const char pattern9[] = "(";
		FixMatch m9(FIX_MATCH_CONSTRU(pattern9));
		PatternMatch* m[] = {&m1, &m2, &m3, &m4, &m5, &m6, &m7, &m8, &m9};
		bool match = true;

		const char* p_matching = p;
		const char* p_end = (p + size);
		for (int i_m = 0
			; match && i_m < 9
			; i_m ++)
		{
			match = m[i_m]->Match(p_matching, p_end);
		}

		std::vector<PatternMatch> m_params;
		if (match) //parse for parameter list
		{
			BlankMatchStar m_10(&m_blank);
			FixMatch m_11(FIX_MATCH_CONSTRU("void"));
			BlankMatchStar m_12(&m_blank);
			FixMatch m_13(FIX_MATCH_CONSTRU(")"));
			PatternMatch* g_2[] = {&m_10, &m_11, &m_12, &m_13};
			And m_and(g_2, 4);
			if (!m_and.Match(p_matching, p_end))
			{
				while (match)
				{
					const char pattern[] = ")";
					FixMatch m_n(FIX_MATCH_CONSTRU(pattern));
					if (!m_n.Match(p_matching, p_end))
					{
						const char patter13[] = ",";
						FixMatch m15(FIX_MATCH_CONSTRU(patter13));
						BlankMatchStar m10(&m_blank);
						TypeMatch m11;
						BlankMatchStar m12(&m_blank);
						NameMatch m13;
						BlankMatchStar m14(&m_blank);
						std::vector<PatternMatch*> m_param;
						if (m_params.size() > 0)
							m_param.push_back(&m15);
						m_param.push_back(&m10);
						m_param.push_back(&m11);
						m_param.push_back(&m12);
						m_param.push_back(&m13);
						m_param.push_back(&m14);

						for (int i_m = 0
							; match && i_m < m_param.size()
							; i_m ++)
							match = m_param[i_m]->Match(p_matching, p_end);

						m_params.push_back(m11);
						m_params.push_back(m13);
					}
					else
						break;
				}
			}

		}

		const char *p_prime;
		unsigned int size_prime;
		if (match)
		{
			Func *func = (Func *)malloc(sizeof(Func));
			m3.Range(func->retType);
			m5.Range(func->callConven);
			m7.Range(func->funcName);
			unsigned int &n_param = func->numParams;
			n_param = m_params.size()/2;
			func->params = (Func::Param* )malloc(func->numParams * sizeof(Func::Param));

			for (int i_param = 0; i_param < n_param; i_param ++)
			{
				int i_base = (i_param << 1);
				PatternMatch m_t = m_params[i_base];
				m_t.Range(func->params[i_param].paramType);
				PatternMatch m_n = m_params[i_base + 1];
				m_n.Range(func->params[i_param].paramName);
			}
			lstFuncs.push_back(func);
			p_prime = p_matching;
			size_prime = size - (p_matching-p);
		}
		else
		{
			p_prime = p + 1;
			size_prime = size - 1;
		}
		p = p_prime;
		size = size_prime;
		//PatternMatchIntera(p_prime, size_prime, lstFuncs);
		//GLAPI +[A-Z|a-z]+ +APIENTRY +[A-Z|a-z]+ *\( *[A-Z|a-z]+ +[A-Z|a-z] * ,
		//1		2         3 4   5     6         7 8 9 10      11 12      13 14 15
		//GLAPI GLboolean APIENTRY glIsProgram (GLuint program);
		//typedef GLboolean (APIENTRY* GLISPROGRAM) (GLuint program);


	}
}


void StartParse4Funcs(const char* szPath, MemSrc* mem, std::list<Func*>& lstFuncs)
{
	bool loaded = LoadFile(szPath, mem);
	assert(loaded);
	const char* p = mem->p;
	unsigned int size = mem->size;
	const char* p_end = p + size;
	while (p < p_end)
		PatternMatchIntera(p, size, lstFuncs);

#ifdef TEST
	std::list<Func*>::iterator it = lstFuncs.begin();
	char output[1024] = {0};
	printf("const char* g_glApiNames[] = \n");
	printf("\t\t\t{\n");
	for (; it != lstFuncs.end(); it ++)
	{
		Func* func = *it;
		//strncpy(output, func->funcName[0], func->funcName[1]-func->funcName[0]);
		char* d = output;
		const char* s = func->funcName[0];
		for (
			; s < func->funcName[1]
			; s ++, d++)
			*d = *s;
		*d = '\0';

		printf("\t\t\t\"%s\"\n", output);
	}
	printf("\t\t\t}\n");
	printf ("FuncObject Generated:");

	int n_void = 0;

	it = lstFuncs.begin();
	for (; it != lstFuncs.end(); it ++)
	{
		char *out = output;
		*out = '\n';
		out ++;
		*out = '\t';
		out ++;

		Func* func = *it;
		const char *src = func->retType[0];
		for (; src < func->retType[1]; src ++, out ++)
		{
			*out = *src;
		}
		if (0 == strncmp(func->retType[0], "void", func->retType[1]-func->retType[0]))
			n_void ++;

		*out = '\t';
		out ++;

		for (src = func->callConven[0]; src< func->callConven[1]; src ++, out ++)
			*out = *src;

		*out = '\t';
		out ++;

		for (src = func->funcName[0]; src < func->funcName[1]; src ++, out ++)
			*out = *src;

		*out = '(';
		out ++;

		for (int i_param = 0; i_param < func->numParams; i_param ++)
		{
			if (i_param > 0)
			{
				*out = ',';
				out ++;
			}

			Func::Param* param = func->params + i_param;
			for (src = param->paramType[0]; src < param->paramType[1]; src ++, out ++)
				*out = *src;

			*out = '\t';
			out ++;

			for (src = param->paramName[0]; src < param->paramName[1]; src ++, out ++)
				*out = *src;

		}
		*out = ')';
		out ++;
		*out = '\n';
		out ++;
		*out = '\0';
		printf (output);
	}
	printf("\n\t%d functions in total, %d void function", lstFuncs.size(), n_void);
#endif
}

void EndParse4Funcs(struct MemSrc_t* mem, std::list<Func*>& lstFuncs)
{
	for (std::list<Func*>::iterator it = lstFuncs.begin()
		; it != lstFuncs.end()
		; it ++)
	{
		Func* f = *it;
		free(f->params);
		free(f);
	}
	UnLoad(mem);
}

void GenFuncsDecl(const char* szPath, const MemSrc* mem, const std::list<Func*>& lstFuncs)
{
#define MAK_STRING(r)\
	r[0], r[1]-r[0]

	std::ofstream fOut(szPath, std::ios::binary|std::ios::out);
	for (std::list<Func*>::const_iterator it = lstFuncs.begin()
		; it != lstFuncs.end()
		; it ++)
	{
		Func* func = *it;

		const char* p = func->retType[0];

		fOut << "typedef "
			 << std::string(MAK_STRING(func->retType))
			 << " ("
			 << std::string(MAK_STRING(func->callConven))
			 << "* FP"<< std::string(MAK_STRING(func->funcName))
			 << ")"
			 << "(";
		for (unsigned int i_param = 0; i_param < func->numParams; i_param ++)
		{
			if (i_param >  0)
				fOut <<", ";
			fOut << std::string(MAK_STRING(func->params[i_param].paramType))
				 << " "
				 << "a_" << std::string(MAK_STRING(func->params[i_param].paramName));
		}
		fOut << ");" << std::endl;

		fOut << "_GLTRACER_API "
			 << std::string(MAK_STRING(func->retType))
			 << " GLTrace_"<<std::string(MAK_STRING(func->funcName))
			 << "("
			 << "FP"<< std::string(MAK_STRING(func->funcName))<<" proc";

		for (unsigned int i_param = 0; i_param < func->numParams; i_param ++)
		{
			fOut << ", "
				 << std::string(MAK_STRING(func->params[i_param].paramType))
				 << " "
				 << "a_" << std::string(MAK_STRING(func->params[i_param].paramName));
		}

		fOut << ", const char* fileName, unsigned int lineNum);" << std::endl;
	}
#undef MAK_STRING
}
void GenFuncsImpl(const char* szPath, const MemSrc* mem, const std::list<Func*>& lstFuncs)
{
#define MAK_STRING(r)\
	r[0], r[1]-r[0]

	FixMatch voidMatch(FIX_MATCH_CONSTRU("void"));
	std::ofstream fOut(szPath, std::ios::binary|std::ios::out);
	for (std::list<Func*>::const_iterator it = lstFuncs.begin()
		; it != lstFuncs.end()
		; it ++)
	{
		Func* func = *it;

		const char* p = func->retType[0];
		bool voidRet = (voidMatch.Match(p, func->retType[1]));


		fOut << std::string(MAK_STRING(func->retType))
			 << " GLTrace_"<<std::string(MAK_STRING(func->funcName))
			 << "("
			 << "FP"<< std::string(MAK_STRING(func->funcName))<<" proc";

		for (unsigned int i_param = 0; i_param < func->numParams; i_param ++)
		{
			fOut << ", "
				 << std::string(MAK_STRING(func->params[i_param].paramType))
				 << " "
				 << "a_" << std::string(MAK_STRING(func->params[i_param].paramName));
		}

		fOut << ", const char* fileName, unsigned int lineNum)" << std::endl;


		fOut << "{" << std::endl
			 << "\tLogItem* item = FuncLogStart(\"" << std::string(MAK_STRING(func->funcName)) <<"\", fileName, lineNum);" <<std::endl;

		if (voidRet)
			fOut << "\tproc(";
		else
			fOut << "\tauto ret = proc(";
							for (unsigned int i_param = 0; i_param < func->numParams; i_param ++)
							{
								if (i_param > 0)
									fOut << ", ";
								fOut << "a_" << std::string(MAK_STRING(func->params[i_param].paramName));
							}
							fOut << "); " << std::endl;
		fOut << "\tFuncLogEnd(item);" << std::endl;

		if (!voidRet)
			fOut << "\treturn ret;" << std::endl;


		fOut << "}" << std::endl << std::endl;


	}
#undef MAK_STRING
}