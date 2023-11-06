#pragma once

#include<comdef.h>
#include<string>
#include<vector>

extern "C" __declspec(dllexport) BSTR GetFunctionName(int iter);
extern "C" __declspec(dllexport) int GetFunctionCount();