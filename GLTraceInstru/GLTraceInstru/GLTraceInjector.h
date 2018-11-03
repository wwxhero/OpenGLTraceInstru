#pragma once
#ifndef _GLTRACE_INJECTOR_H
#define _GLTRACE_INJECTOR_H
#include <set>

void Inject(const char* filePath, const std::set<std::string>& tokens);

#endif