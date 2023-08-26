#pragma once
#include <Windows.h>
#define _NO_CVCONST_H
#include <DbgHelp.h>
#include <string>
#include <vector>
#include <sstream>

enum BasicType;

class FunctionObject
{
	VARIANT m_value;
	
	HANDLE m_hProcess;
	enum SymTagEnum m_tag;
	std::string m_enumValue;
	bool m_isReturnObj;

	PSYMBOL_INFO m_symbol;

	// Loads the name of the object
	void LoadName(PSYMBOL_INFO sym);

	// Loads the name of the type of that symbol
	void LoadType(PSYMBOL_INFO sym, ULONG64 frameAddress, HANDLE hProcess);

	// Try to load the actual value of that object if possible
	void LoadValue(STACKFRAME64 frame, ULONG64 address,HANDLE hProcess);

	// Tries to match an enum's members to a given value
	void LoadEnumValue(int value);

	// Loads a basic type (int, float, char, ...)
	void LoadBasicType(BasicType bt, ULONG64 byteSize);

	// Loads the type pointed to
	void LoadPointerType(PSYMBOL_INFO sym, DWORD subType);

	// Returns a string that represents the value
	std::string GetValueString();

	BOOL __stdcall EnumSymbolCallbackStructures(PSYMBOL_INFO inf, ULONG size, PVOID param);

public:
	// With the stack frame its interpreted as a local variable or a parameter
	FunctionObject(PSYMBOL_INFO symbol, STACKFRAME64 frame, HANDLE hProcess);

	// Without the stack frame its interpreted as a return value
	FunctionObject(PSYMBOL_INFO symbol,HANDLE hProcess);

	// Returns the current function object represented as a string
	std::string ToString();

	std::string m_objName;
	std::string m_typeName;
};