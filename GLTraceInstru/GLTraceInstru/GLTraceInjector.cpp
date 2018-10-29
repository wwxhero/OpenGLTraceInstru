// GLTraceInstruInjector.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "GLTraceInjector.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <list>


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

class Expression
{
public:
	void push_param(const std::string& param)
	{
		m_params.push_back(param);
	}
	void emitt(std::string& exp)
	{
		if (m_reference.empty())
		{
			//GLTrace_void_n(token, param1, param2, ..., paramn)
			std::stringstream o;
			o << "GLTRACE_VOID_" << m_params.size() << "(" << m_token;
			for (std::list<std::string>::iterator it = m_params.begin()
				; it != m_params.end()
				; it ++)
			{
				o << ", " << *it;
			}
			o << ")";
			exp = o.str();
		}
		reset();
	}
	void reset()
	{
		m_token.clear();
		m_reference.clear();
		m_params.clear();
	}
	const char* m_src;
	std::string m_token;
	std::string m_reference;
private:
	std::list<std::string> m_params;
};

void InjectInternal(const std::set<std::string>& tokens, const MemSrc* src, std::ostringstream* dst)
{
	const char* p_start = (const char*)src->p;
	const char* p_end = (const char*)(src->p + src->size);
	const char* p = p_start;
	enum {ready = 0, parsing_G, parsing_P} s = ready;
	std::string token;
	std::string param;
	Expression exp;
#define A_LETTER(c)\
	(c > 'A'-1 && c <'Z'+1)\
	|| (c > 'a'-1 && c < 'z'+1)
#define LEFT_PARENTHESIS(c)\
	('(' == c)
#define RIGHT_PARENTHESIS(c)\
	(')' == c)
#define COMMA(c)\
	(',' == c)
#define SPACE(c)\
	(' ' == c)
#define TAB(c)\
	('\t' == c)
#define LINEBR(p)\
	((*p == '\n')\
	|| (*p == '\r' && *(p+1) == '\n'))

	while (p < p_end)
	{
		switch (s)
		{
			case ready:
			{
				if (A_LETTER(*p))
				{
					token = *p;
					exp.m_src = p;
					s = parsing_G;
				}
				p ++;
				break;
			}
			case parsing_G:
			{
				if (LINEBR(p))
				{
					do
					{
						p++;
					}while (SPACE(*p) || TAB(*p) || LINEBR(p));
					if (!LEFT_PARENTHESIS(*p))
					{
						token.clear();
						exp.reset();
						s = ready;
						p--;
						break;
					}
				}

				if (LEFT_PARENTHESIS(*p))
				{
					if (tokens.end() != tokens.find(token))
					{
						exp.m_token = token;
						token.clear();
						s = parsing_P;
						param.clear();
					}
					else
					{
						token.clear();
						exp.reset();
						s = ready;
					}
				}
				else if(A_LETTER(*p))
				{
					token += *p;
				}

				else if(!SPACE(*p)
					&& !TAB(*p))
				{
					token.clear();
					exp.reset();
					s = ready;
				}
				p ++;
				break;
			}
			case parsing_P:
			{
				if (RIGHT_PARENTHESIS(*p))
				{
					if (!param.empty())
						exp.push_param(param);
					param.clear();
					dst->write(p_start, exp.m_src-p_start);
					p_start = p + 1;
					std::string exp_t;
					exp.emitt(exp_t);
					*dst << exp_t;
					s = ready;
				}
				else if (COMMA(*p))
				{
					exp.push_param(param);
					param.clear();
				}
				else
				{
					param += *p;
				}
				p ++;
				break;
			}
		}
	}
	dst->write(p_start, p-p_start);
#undef LINEBR
#undef SPACE
#undef TAB
#undef COMMA
#undef RIGHT_PARENTHESIS
#undef LEFT_PARENTHESIS
#undef A_LETTER
}

void Inject(const char* filePath)
{
	MemSrc memSrc;
	bool loaded = LoadFile(filePath, &memSrc);
	ASSERT(loaded);
	std::ostringstream memDst;
///for testing
	std::set<std::string> tokens;
	tokens.insert("glDrawArrays");
	tokens.insert("glClear");
/////////
	InjectInternal(tokens, &memSrc, &memDst);
	UnLoad(&memSrc);
	std::ofstream fOut(filePath, std::ios::binary|std::ios::out);
	fOut << memDst.str();
}