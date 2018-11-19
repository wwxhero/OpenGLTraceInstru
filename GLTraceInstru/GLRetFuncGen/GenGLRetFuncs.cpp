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

#define A_LETTER(c)\
	(c > 'A'-1 && c <'Z'+1)\
	|| (c > 'a'-1 && c < 'z'+1)
#define A_NUMBER(c)\
	(c > '0'-1 && c < '9'+1)
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


#define FIX_MATCH_CONSTRU(p)\
		p, sizeof(p)-1

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



//pattern:
//GLAPI +[A-Z|a-z]+ +APIENTRY +[A-Z|a-z]+ +\([A-Z|a-z]+ +
//GLAPI GLboolean APIENTRY glIsProgram (GLuint program);
//typedef GLboolean (APIENTRY* GLISPROGRAM) (GLuint program);
//_GLTRACER_API GLboolean GLTrace_glIsProgram (GLISPROGRAM proc, GLuint program, const char* fileName, unsigned int lineNum);
// typedef struct Func_t
// {
// 	const char* retType[2];
// 	const char* callConven[2];
// 	const char* funcName[2];
// 	typedef struct Param_t
// 	{
// 		const char* paramType[2];
// 		const char* paramName[2];
// 	} Param;
// 	Param* params;
// 	unsigned int numParams;
// } Func;

class PatternMatch
{
public:
	virtual bool Match(const char*& p, const char* end)
	{
		return false;
	}
	virtual void Range(const char* r[2])
	{
		r[0] = m_range[0];
		r[1] = m_range[1];
	}
protected:
	const char* m_range[2];
};

class FixMatch : public PatternMatch
{
public:
	FixMatch(const char* pattern, unsigned int len)
	{
		m_pattern = (char*)malloc(len);
		strncpy(m_pattern, pattern, len);
		m_len = len;
	}
	~FixMatch()
	{
		free(m_pattern);
	}
	virtual bool Match(const char*& p, const char* end) override
	{
		m_range[0] = p;
		const char*& s = p;
		const char*  s_e = end;
		const char*  d_e = m_pattern + m_len;
		bool match = true;
		for ( const char*  d = m_pattern
			; d < d_e && s < s_e && match
			; d ++, s ++ )
			match = ((*d) == (*s));
		if (!match)
			s --;
		m_range[1] = s;
		return match;
	}

private:
	char* m_pattern;
	unsigned int m_len;
};


class BlankMatch : public PatternMatch
{
public:
	BlankMatch()
	{

	}
	virtual bool Match(const char*& p, const char* end) override
	{
		m_range[0] = p;
		bool match = (SPACE(*p)
					|| TAB(*p));
		if (match)
			p ++;
		m_range[1] = p;
		return match;
	}
};

template<class TMatch>
class Star : public PatternMatch
{
public:
	Star(TMatch* m) : m_pMatch(m)
	{
	}
	virtual bool Match(const char*& p, const char* end) override
	{
		m_range[0] = p;
		while(m_pMatch->Match(p, end));
		m_range[1] = p;
		return true;
	}
private:
	TMatch* m_pMatch;
};

typedef Star<BlankMatch> BlankMatchStar;

template<class TMatch>
class Plus : public PatternMatch
{
public:
	Plus(TMatch* m) : m_pMatch(m)
	{
	}
	virtual bool Match(const char*& p, const char* end) override
	{
		m_range[0] = p;
		int i = 0;
		while (m_pMatch->Match(p, end))
			i ++;
		m_range[1] = p;
		return i > 0;
	}
private:
	TMatch* m_pMatch;
};

typedef Plus<BlankMatch> BlankMatchPlus;

class Or : public PatternMatch
{
public:
	Or(PatternMatch** m, unsigned int n) : m_arr(m)
										, m_n(n)
	{
	}
	virtual bool Match(const char*& p, const char* end) override
	{
		bool match = false;
		for (unsigned int i = 0; i < m_n && !match; i ++)
		{
			match = (m_arr[i]->Match(p, end));
			if (match)
				m_arr[i]->Range(m_range);
		}
		return match;
	}
private:
	PatternMatch** m_arr;
	unsigned int m_n;
};

class And : public PatternMatch
{
public:
	And(PatternMatch** m, unsigned int n) : m_arr(m)
										, m_n(n)
	{
	}
	virtual bool Match(const char*& p, const char* end) override
	{
		bool match = true;
		m_range[0] = p;
		for (unsigned int i = 0; i < m_n && match; i ++)
		{
			match = (m_arr[i]->Match(p, end));
		}
		m_range[1] = p;
		return match;
	}
private:
	PatternMatch** m_arr;
	unsigned int m_n;
};

class LetterMatch : public PatternMatch
{
public:
	LetterMatch()
	{

	}
	virtual bool Match(const char*& p, const char* end) override
	{
		m_range[0] = p;
		bool matched = A_LETTER(*p);
		while(A_LETTER(*p)
		   && p < end)
			p ++;
		m_range[1] = p;
		return matched;
	}
};

class NameMatch : public PatternMatch
{
public:
	NameMatch()
	{

	}
	virtual bool Match(const char*& p, const char* end) override
	{
		m_range[0] = p;
		bool matched = (A_LETTER(*p)
					|| *p == '_');
		while( (A_LETTER(*p)
				|| *p == '_'
				|| A_NUMBER(*p))
		   && p < end)
			p ++;
		m_range[1] = p;
		return matched;
	}
};

class TypeMatch : public PatternMatch
{
public:
	TypeMatch()
	{

	}
	virtual bool Match(const char*& p, const char* end) override
	{
		m_range[0] = p;
		bool match = true;

		FixMatch m1(FIX_MATCH_CONSTRU("const"));
		BlankMatch m2;
		Plus<BlankMatch> m3(&m2);
		//m1 -> m2: const<blank>
		match = (!m1.Match(p, end) || m3.Match(p, end));


		NameMatch m4;
		match = (match
			  && m4.Match(p, end));

		if (match)
		{
			m_range[1] = p;
			BlankMatch m0;
			Star<BlankMatch> m1(&m0);
			FixMatch m2(FIX_MATCH_CONSTRU("*"));
			FixMatch m3(FIX_MATCH_CONSTRU("&"));
			PatternMatch* g[] = {&m2, &m3};
			Or m4(g, 2);
			Plus<Or> m5(&m4);
			Star<BlankMatch> m6(&m0);
			PatternMatch* g2[] = {&m1, &m5, &m6};
			And m_and(g2, 3);
			Plus<And> andPlus(&m_and);
			if (andPlus.Match(p, end))
				m_range[1] = p;
			else
				p = m_range[1];

		}
		return match;
	}
};






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
				 << std::string(MAK_STRING(func->params[i_param].paramName));
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
				 << std::string(MAK_STRING(func->params[i_param].paramName));
		}

		fOut << ", const char* fileName, unsigned int lineNum);" << std::endl;
	}
}
void GenFuncsImpl(const char* szPath, const MemSrc* mem, const std::list<Func*>& lstFuncs)
{
}