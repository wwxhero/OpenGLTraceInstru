#pragma once
#ifndef _GLTRACE_INJECTOR_H
#define _GLTRACE_INJECTOR_H
#include <set>

void Inject(const char* filePath
		, const std::set<std::string>& void_tokens
		, const std::set<std::string>& unvoid_tokens
		, const std::set<std::string>& sync_tokens);

#endif