// GLTraceInstruInjector.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "GLTraceInjector.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <list>
#include "PatternMatch.h"


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

class Call
{
public:
	Call(const char* range[2]
		, const std::string& token
		, const std::list<std::string>& params)
	{
		m_range[0] = range[0];
		m_range[1] = range[1];
		m_token = token;
		m_params = params;
	}
	void Range(const char* range[2])
	{
		range[0] = m_range[0];
		range[1] = m_range[1];
	}
	virtual void emitt(std::string& exp) = 0;
private:
	const char* m_range[2];
protected:
	std::list<std::string> m_params;
	std::string m_token;

};

class GLCall : public Call
{
public:
	GLCall(const char* range[2]
		, const std::string& token
		, const std::list<std::string>& params
		, bool void_call)
				: Call(range, token, params)
				, m_isVoid(void_call)
	{

	}
	virtual void emitt(std::string& exp)
	{
		//GLTrace_void_n(token, param1, param2, ..., paramn)
		std::stringstream o;
		if (m_isVoid)
			o << "GLTRACE_VOID_" << m_params.size() << "(" << m_token;
		else
			o << "GLTRACE_RET_" << m_params.size() << "(" << m_token;

		for (std::list<std::string>::iterator it = m_params.begin()
				; it != m_params.end()
				; it ++)
		{
			o << ", " << *it;
		}
		o << ")";
		exp = o.str();
	}
private:
	bool m_isVoid;
};

class SyncCall : public Call
{
public:
	SyncCall(const char* range[2]
		, const std::string& token
		, const std::list<std::string>& params)
			: Call(range, token, params)
	{
	}
	virtual void emitt(std::string& exp)
	{
		//GLTrace_void_n(token, param1, param2, ..., paramn)
		std::stringstream o;
		o << "GLTRACE_SYNC_" << m_params.size() << "(" << m_token;


		for (std::list<std::string>::iterator it = m_params.begin()
				; it != m_params.end()
				; it ++)
		{
			o << ", " << *it;
		}
		o << ")";
		exp = o.str();
	}
};






unsigned int MatchGLFuncs(const std::set<std::string>& void_tokens
				, const std::set<std::string>& unvoid_tokens
				, const char* p_start
				, const char* p_end
				, std::list<Call*>& calls)
{
	//glDrawArrays(GL_TRIANGLES, 0, 3);
	unsigned int offset = 0;
	NameMatch call_name;
	BlankMatch blank_match;
	BlankMatchStar blank_star(&blank_match);
	FixMatch left(FIX_MATCH_CONSTRU("("));
	PatternMatch* g[] = {&call_name, &blank_star, &left, &blank_star};
	And g_match(g, 4);
	const char* p = p_start;
	if (g_match.Match(p, p_end))
	{
		const char* r[2] = {0};
		call_name.Range(r);
		std::string token(MAKE_STRING(r));
		bool b_void = (void_tokens.end() != void_tokens.find(token));
		bool b_unvoid = (!b_void)
						&& (unvoid_tokens.end() != unvoid_tokens.find(token));
		if (b_void || b_unvoid)
		{
			std::list<std::string> params;
			FixMatch right(FIX_MATCH_CONSTRU(")"));
			ValueMatch value;
			bool b_right = false;
			bool b_value = false;
			while (!(b_right = right.Match(p, p_end))
				&& (b_value = value.Match(p, p_end)))
			{
				value.Range(r);
				params.push_back(std::string(MAKE_STRING(r)));

				FixMatch sep(FIX_MATCH_CONSTRU(","));
				Question<FixMatch> sep_ques(&sep);
				PatternMatch* g_ignore[] = {&blank_star, &sep_ques, &blank_star};
				And and_ignore(g_ignore, 3);
				and_ignore.Match(p, p_end);
			}
			if (b_right)
			{
				Call *call = NULL;
				const char* range[] = {p_start, p};
				if (b_void)
					call = new GLCall(range, token, params, true);
				else
					call = new GLCall(range, token, params, false);
				calls.push_back(call);
				offset = p-p_start;
			}
		}
	}

	return offset;
}

unsigned int MatchSyncFuncs(const std::set<std::string>& sync_tokens
				, const char* p_start
				, const char* p_end
				, std::list<Call*>& calls)
{
	unsigned int offset = 0;
	NameMatch class_name;
	BlankMatch blank_match;
	BlankMatchStar blank_star(&blank_match);
	FixMatch cnn1(FIX_MATCH_CONSTRU("->"));
	FixMatch cnn2(FIX_MATCH_CONSTRU("."));
	PatternMatch* g_cnn[] = {&cnn1, &cnn2};
	Or or_cnn(g_cnn, 2);
	NameMatch call_name;
	FixMatch left(FIX_MATCH_CONSTRU("("));
	PatternMatch* g_cpp[] = {&class_name, &blank_star, &or_cnn, &blank_star, &call_name, &blank_star, &left, &blank_star};
	And and_cpp(g_cpp, sizeof(g_cpp)/sizeof(PatternMatch*));
	const char* p = p_start;
	bool match_cpp = and_cpp.Match(p, p_end);
	bool match_c = (!match_cpp) && call_name.Match(p, p_end);
	if (match_cpp || match_c)
	{
		const char* range_proc[2] = {0};
		call_name.Range(range_proc);
		std::string token(MAKE_STRING(range_proc));
		if (sync_tokens.end() != sync_tokens.find(token))
		{
			std::list<std::string> params;
			FixMatch right(FIX_MATCH_CONSTRU(")"));
			ValueMatch value;
			bool b_right = false;
			bool b_value = false;
			while (!(b_right = right.Match(p, p_end))
				&& (b_value = value.Match(p, p_end)))
			{
				const char* r[2] = {0};
				value.Range(r);
				params.push_back(std::string(MAKE_STRING(r)));

				FixMatch sep(FIX_MATCH_CONSTRU(","));
				Question<FixMatch> sep_ques(&sep);
				PatternMatch* g_ignore[] = {&blank_star, &sep_ques, &blank_star};
				And and_ignore(g_ignore, 3);
				and_ignore.Match(p, p_end);
			}
			if (b_right)
			{
				Call *call = NULL;
				if (match_cpp)
				{
					const char* range_class[2] = {0};
					class_name.Range(range_class);
					const char* range[] = {range_class[0], range_proc[1]};
					token = std::string(MAKE_STRING(range));
				}
				const char* range_call[] = {p_start, p};
				call = new SyncCall(range_call, token, params);
				calls.push_back(call);
				offset = p-p_start;
			}
		}

	}
	return offset;
}

void InjectInternal(const std::set<std::string>& void_tokens
	, const std::set<std::string>& unvoid_tokens
	, const std::set<std::string>& sync_tokens
	, const MemSrc* src, std::ostringstream* dst)
{
	//glDrawArrays(GL_TRIANGLES, 0, 3);
	const char* p_start = (const char*)src->p;
	const char* p_end = p_start + src->size;
	const char* p = p_start;
	std::list<Call*> calls;
	while (p < p_end)
	{
		unsigned int offset = MatchSyncFuncs(sync_tokens, p, p_end, calls);
		if (offset > 0)
			p = p + offset;
		else if ((offset = MatchGLFuncs(void_tokens, unvoid_tokens, p, p_end, calls)) > 0)
			p = p + offset;
		else
			p ++;
	}
	p = p_start;
	for (std::list<Call*>::iterator it = calls.begin()
		; it != calls.end()
		; it ++)
	{
		Call* call = *it;
		const char* range[2] = {0};
		call->Range(range);
		dst->write(p, range[0]-p);
		std::string exp;
		call->emitt(exp);
		*dst << exp;
		p = range[1];
		delete call;
	}
	dst->write(p, p_end-p);
}

void Inject(const char* filePath
	, const std::set<std::string>& void_tokens
	, const std::set<std::string>& unvoid_tokens
	, const std::set<std::string>& sync_tokens)
{
	MemSrc memSrc;
	bool loaded = LoadFile(filePath, &memSrc);
	ASSERT(loaded);
	if (loaded)
	{
		std::ostringstream memDst;
		InjectInternal(void_tokens, unvoid_tokens, sync_tokens, &memSrc, &memDst);
		UnLoad(&memSrc);
		std::ofstream fOut(filePath, std::ios::binary|std::ios::out);
		fOut << memDst.str();
	}
}