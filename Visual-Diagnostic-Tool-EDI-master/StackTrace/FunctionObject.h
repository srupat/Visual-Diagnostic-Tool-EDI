#pragma once
#include <Windows.h>
#define _NO_CVCONST_H
#include <DbgHelp.h>
#include <string>
#include <vector>
#include <sstream>

enum BasicType;

struct Members
{
	VARIANT m_value;
	std::string m_objName;
	std::string m_typeName;
};

class FunctionObject
{
	VARIANT m_value;
	std::string m_objName;
	std::string m_typeName;
	HANDLE m_hProcess;
	std::vector<Members> UDT_membersInfo;
	std::vector<VARIANT> arrayMembers;
	enum SymTagEnum m_tag;
	std::string m_enumValue;
	bool m_isReturnObj;
	bool isUDT;

	PSYMBOL_INFO m_symbol;

	// Loads the name of the object
	void LoadName(PSYMBOL_INFO sym);

	// Loads the name of the type of that symbol
	void LoadType(STACKFRAME64 frame, PSYMBOL_INFO sym, ULONG64 frameAddress);

	void LoadType(PSYMBOL_INFO sym, ULONG64 frameAddress);

	// Try to load the actual value of that object if possible
	void LoadValue(STACKFRAME64 frame, ULONG64 address, enum SymTagEnum mem_tag);

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

	
	void extractValues(std::string& name, std::string& datatype, VARIANT& value, ULONG64 address) const
	{
		name = m_objName;
		datatype = m_typeName;
		value = m_value;
		address = m_symbol->Address; 
	}

	
};