#include "FunctionObject.h"
#include <iostream>
#include<Windows.h>

#pragma comment(lib, "Dbghelp.lib")

/* Extracted from cvconst.h */
enum BasicType
{
	btNoType = 0,
	btVoid = 1,
	btChar = 2,
	btWChar = 3,
	btInt = 6,
	btUInt = 7,
	btFloat = 8,
	btBCD = 9,
	btBool = 10,
	btLong = 13,
	btULong = 14,
	btCurrency = 25,
	btDate = 26,
	btVariant = 27,
	btComplex = 28,
	btBit = 29,
	btBSTR = 30,
	btHresult = 31,
};


void FunctionObject::LoadBasicType(BasicType bt, ULONG64 byteSize)
{
	// To represent what kind of object we have i use a VARIANT which
	// can hold a lot of different types.
	switch (bt)
	{
	case btNoType:
		m_typeName = "Unknown";
		m_value.vt = VT_UNKNOWN;
		break;
	case btVoid:
		m_typeName = "void";
		m_value.vt = VT_UNKNOWN; // void cannot be evaluated so we set it as VT_UNKNOWN
		break;
	case btChar:
		m_value.vt = VT_I1 | 0x1000; // its a 'special' 1 byte integer, so we add 0x1000
		m_typeName = "char";
		break;
	case btWChar:
		m_value.vt = VT_I1 | 0x8000; // its actually not a 1 byte integer, so we add 0x8000
		m_typeName = "wchar_t";
		break;
	case btInt:
	{
		m_typeName = "signed __int32";
		m_value.vt = VT_I4; // btInt is set for every type of integer. We need to determine the real type by the byte size
		switch (byteSize)
		{
			case sizeof(__int8) :
				m_typeName = "signed __int8";
				m_value.vt = VT_I1;
				break;
				case sizeof(__int16) :
					m_typeName = "signed __int16";
					m_value.vt = VT_I2;
					break;
					case sizeof(__int32) :
						m_typeName = "signed __int32";
						m_value.vt = VT_I4;
						break;
						case sizeof(__int64) :
							m_typeName = "signed __int64";
							m_value.vt = VT_I8;
							break;
		}
		break;
	}
	case btUInt:
	{
		m_typeName = "unsigned __int32";
		m_value.vt = VT_UI4;
		switch (byteSize)
		{
			case sizeof(__int8) :
				m_typeName = "unsigned __int8";
				m_value.vt = VT_UI1;
				break;
				case sizeof(__int16) :
					m_typeName = "unsigned __int16";
					m_value.vt = VT_UI2;
					break;
					case sizeof(__int32) :
						m_typeName = "unsigned __int32";
						m_value.vt = VT_UI4;
						break;
						case sizeof(__int64) :
							m_typeName = "unsigned __int64";
							m_value.vt = VT_UI8;
							break;
		}
		break;
	}
	case btFloat:
		m_typeName = "float";
		m_value.vt = VT_R4;
		break;
	case btBCD:
		m_typeName = "BCD";
		m_value.vt = VT_UNKNOWN;
		break;
	case btBool:
		m_typeName = "bool";
		m_value.vt = VT_BOOL;
		break;
	case btLong:
		m_typeName = "long int";
		m_value.vt = VT_I4;
		break;
	case btULong:
		m_typeName = "unsigned long int";
		m_value.vt = VT_UI4;
		break;
	case btCurrency:
		m_typeName = "currency";
		m_value.vt = VT_UNKNOWN;
		break;
	case btDate:
		m_typeName = "DATE";
		m_value.vt = VT_UNKNOWN;
		break;
	case btComplex:
		m_typeName = "complex";
		m_value.vt = VT_UNKNOWN;
		break;
	case btVariant:
		m_typeName = "VARIANT";
		m_value.vt = VT_UNKNOWN;
		break;
	case btBit:
		m_typeName = "bit";
		m_value.vt = VT_UNKNOWN;
		break;
	case btBSTR:
		m_typeName = "OLESTR";
		m_value.vt = VT_BSTR;
		break;
	case btHresult:
		m_typeName = "HRESULT";
		m_value.vt = VT_BLOB;
		break;

	default:
		m_typeName = "Unknown";
		m_value.vt = VT_UNKNOWN;
		break;
	}
}

FunctionObject::FunctionObject(PSYMBOL_INFO sym, STACKFRAME64 frame,HANDLE hProcess) : m_isReturnObj(false)
{
	m_symbol = sym;
	m_hProcess = hProcess;
	ULONG64 address = frame.AddrFrame.Offset;
	VariantInit(&m_value);
	LoadName(sym);
	LoadType(sym, address, hProcess);
	LoadValue(frame, sym->Address,hProcess);
}

FunctionObject::FunctionObject(PSYMBOL_INFO sym,HANDLE hProcess) : m_isReturnObj(true)
{
	m_symbol = sym;
	m_hProcess = hProcess;
	LoadType(sym,NULL,hProcess);
}

void FunctionObject::LoadName(PSYMBOL_INFO sym)
{
	// The name is already contained in the symbol -> not much to do ;)
	m_objName = sym->Name;
}

BOOL CALLBACK EnumerateMembersCallback(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID typeIndex)
{

	if (pSymInfo->Tag == SymTagData)
	{
		std::cout << pSymInfo->Name << std::endl;
	}

	return TRUE; // Continue enumeration
}

void FunctionObject::LoadType(PSYMBOL_INFO sym, ULONG64 frameAddress,HANDLE hProcess)
{
	// Sadly loading the actual name of the type must be done in several different
	// ways for different types. To see which one we need to follow we first take
	// the tag of the symbol.
	
	m_typeName = "Unknown";
	m_tag = (enum SymTagEnum)0;
	SymGetTypeInfo(m_hProcess, sym->ModBase, sym->TypeIndex, TI_GET_SYMTAG, &m_tag);
	switch (m_tag)
	{
		// Base types are types like int, float, void, char, ...
		// They are not stored directly in the PDB because there aren't any big information
		// that need to be stored for them. We can use TI_GET_BASETYPE to fill a BasicType
		// which will describe the basic type and were done!
	case SymTagBaseType:
	{
		BasicType bt = (BasicType)0;
		SymGetTypeInfo(m_hProcess, sym->ModBase, sym->TypeIndex, TI_GET_BASETYPE, &bt);
		ULONG64 length = 0;
		SymGetTypeInfo(m_hProcess, sym->ModBase, sym->TypeIndex, TI_GET_LENGTH, &length);
		LoadBasicType(bt, length);
		return;
	}

	// For pointer types we just need to take the type it points to and then load that type
	case SymTagPointerType:
	{
		DWORD subType;
		SymGetTypeInfo(m_hProcess, sym->ModBase, sym->TypeIndex, TI_GET_TYPE, &subType);
		LoadPointerType(sym, subType);
		// We need to override the possible type set by LoadPointerType because it should
		// only take an address when we evaluate it.
		m_value.vt = VT_BLOB;
		return;
	}

	// Now most of the rest can be simply parsed by getting the SYMNAME because they are complex
	// types and therefore have an entry in the PDB.
	default:
	{
		TI_FINDCHILDREN_PARAMS* pChildren = NULL;
		SYMBOL_INFO memInfo{ 0 };
		memInfo.SizeOfStruct = sizeof(SYMBOL_INFO);
		memInfo.MaxNameLen = 255;
		DWORD ChildCount = 0;
		BOOL bResult = FALSE;

		bResult = SymGetTypeInfo(m_hProcess, sym->ModBase, sym->TypeIndex, TI_GET_CHILDRENCOUNT, &ChildCount);
		if (!bResult)
		{
			std::cout << "SymGetTypeInfo failed with error :" << GetLastError() << std::endl;
		}

		pChildren = (TI_FINDCHILDREN_PARAMS*)malloc(sizeof(TI_FINDCHILDREN_PARAMS) + ChildCount * sizeof(ULONG));
		if (!pChildren)
		{
			// Handle error
		}

		pChildren->Count = ChildCount;
		pChildren->Start = 0;

		bResult = SymGetTypeInfo(m_hProcess, sym->ModBase, sym->TypeIndex, TI_FINDCHILDREN, pChildren);
		if (!bResult)
		{
			std::cout << "SymGetTypeInfo failed with error :" << GetLastError() << std::endl;
		}

		for (ULONG i = 0; i < ChildCount; i++) {
			ULONG ChildId = pChildren->ChildId[i];
			WCHAR* symbolName = NULL;
			DWORD type = 0;
			ULONG64 length = 0;
			BasicType bt = (BasicType)0;
			if (!SymGetTypeInfo(m_hProcess, sym->ModBase, ChildId, TI_GET_SYMNAME, &symbolName))
			{
				std::cout << "SymGetTypeInfo failed with error :" << GetLastError() << std::endl;
			}
			if (!SymGetTypeInfo(m_hProcess, sym->ModBase, ChildId, TI_GET_TYPE, &type))
			{
				std::cout << "SymGetTypeInfo failed with error :" << GetLastError() << std::endl;
			}
			SymGetTypeInfo(m_hProcess, sym->ModBase, type, TI_GET_BASETYPE, &bt);
			SymGetTypeInfo(m_hProcess, sym->ModBase, type, TI_GET_LENGTH, &length);
			LoadBasicType(bt, length);
		}
	}
	}
}


std::string FunctionObject::GetValueString()
{
	// Things here should be pretty simple, its just parsing the variant type

	if (m_tag == SymTagEnum)
		return m_enumValue;

	std::stringstream ret;
	switch (m_value.vt)
	{
	case VT_UNKNOWN:
		return "not evaluated";
	case VT_BLOB: // <- Pointers and HRESULT's
		ret << std::hex << "0x" << m_value.uintVal;
		break;

	case VT_I8:
		ret << m_value.llVal;
		break;

	case VT_I4:
	case VT_I2:
	case VT_I1:
		ret << m_value.intVal;
		break;

	case VT_UI8:
		ret << m_value.ullVal;
		break;

	case VT_UI4:
	case VT_UI2:
	case VT_UI1:
		ret << m_value.uintVal;
		break;

	case VT_I1 | 0x1000:
		ret << (char)m_value.intVal;
		break;

	case VT_I1 | 0x8000:
		ret << (wchar_t)m_value.intVal;
		break;

	case VT_R4:
		ret << (float)m_value.fltVal;
		break;

	default:
		return "not evaluated";
	}
	return ret.str();
}

std::string FunctionObject::ToString()
{
	if (m_isReturnObj)
		return m_typeName;

	std::stringstream ret;
	ret << m_typeName << " " << m_objName << " = " << GetValueString();
	return ret.str();
}

void FunctionObject::LoadPointerType(PSYMBOL_INFO sym, DWORD type)
{
	// This function does exactly the same as LoadType does except
	// that it adds a * to the type name
	enum SymTagEnum tag = (enum SymTagEnum)0;
	SymGetTypeInfo(m_hProcess, sym->ModBase, type, TI_GET_SYMTAG, &tag);
	switch (tag)
	{
	case SymTagBaseType:
	{
		BasicType bt = (BasicType)0;
		SymGetTypeInfo(m_hProcess, sym->ModBase, type, TI_GET_BASETYPE, &bt);
		ULONG64 length = 0;
		SymGetTypeInfo(m_hProcess, sym->ModBase, type, TI_GET_LENGTH, &length);
		LoadBasicType(bt, length);
		m_typeName += '*';
		return;
	}

	case SymTagPointerType:
	{
		DWORD subType;
		SymGetTypeInfo(m_hProcess, sym->ModBase, type, TI_GET_TYPE, &subType);
		// We recursively call ourselfs until we dont have a PointerType anymore
		// for example in char*** we need to call it 3 times
		LoadPointerType(sym, subType);
		m_typeName += '*';
		return;
	}

	default:
	{
		LPWSTR symName = NULL;
		BOOL ret = SymGetTypeInfo(m_hProcess, sym->ModBase, sym->TypeIndex, TI_GET_SYMNAME, &symName);
		{
			std::cout << "SymEnumTypes function failed " << std::endl;
		}
		if (ret == TRUE)
		{
			std::vector<char> ansi(wcslen(symName) + 1);
			WideCharToMultiByte(CP_ACP, 0, symName, ansi.size(), &ansi[0], ansi.size(), NULL, NULL);
			m_typeName = &ansi[0];
			m_typeName += '*';
			LocalFree(symName);
		}
		break;
	}
	}
}

void FunctionObject::LoadValue(STACKFRAME64 frame, ULONG64 addr,HANDLE hProcess)
{
	
	 //AddFrame holds the address of the current frame on the stack where locale variables
	 //and parameters are stored. the symbol address actually holds the offset on the 
	 //stack so we get a pointer to the data by adding the offset to the frame pointer
	
	LPVOID mem = (LPVOID)(frame.AddrFrame.Offset + addr);
	

	if (m_tag == SymTagEnum)
	{
		// This is not 100% correct because the enum could only
		// be a 1-byte value and therefore we take 3 additional
		// bytes which may be just random data but it would bloat
		// things up to much if we first determine the size of the
		// enum...
		int value = *(int*)mem;
		LoadEnumValue(value);
		return;
	}

	switch (m_value.vt)
	{
	case VT_UI4:
	case VT_BLOB:
	{
		LPDWORD* buffer = (LPDWORD*)malloc(sizeof(int));
		bool a = ReadProcessMemory(hProcess, mem, (LPVOID)buffer, sizeof(int), NULL);
		m_value.uintVal = *(LPDWORD)buffer;
		break;
	}
	case VT_I4:
	{
		
		int* buffer = (int*)malloc(sizeof(int));
		bool a = ReadProcessMemory(hProcess, mem, (LPVOID)buffer, sizeof(int), NULL);
		m_value.intVal = *(int*)buffer;
		break;
	}
	case VT_R4:
	{
		float* buffer = (float*)malloc(sizeof(float));
		bool a = ReadProcessMemory(hProcess, mem, (LPVOID)buffer, sizeof(float), NULL);
		m_value.fltVal = *(FLOAT*)buffer;
		break;
	}
	case VT_BOOL:
	{
		m_value.boolVal = *(bool*)mem;
		break;
	}
	case VT_I1 | 0x1000:
	case VT_I1:
	{
		char* buffer = (char*)malloc(sizeof(char));	
		bool a = ReadProcessMemory(hProcess, (LPVOID)mem, buffer, sizeof(char), NULL);
		m_value.intVal = *(signed __int8*)buffer;
		
		break;
	}
	case VT_I1 | 0x8000:
	case VT_I2:
	{
		m_value.intVal = *(signed __int16*)mem;
		break;
	}
	case VT_I8:
	{
		m_value.llVal = *(signed __int64*)mem;
		break;
	}
	case VT_UI1:
	{
		m_value.uintVal = *(unsigned __int8*)mem;
		break;
	}
	case VT_UI2:
	{
		m_value.uintVal = *(unsigned __int16*)mem;
		break;
	}
	case VT_UI8:
	{
		m_value.ullVal = *(unsigned __int64*)mem;
		break;
	}
	}
}

void FunctionObject::LoadEnumValue(int value)
{
	// The values of an enum type are stored as its children
	// We can access them using TI_FINDCHILDREN but first we
	// need to know how many children it has using TI_GET_CHILDRENCOUNT
	DWORD numChildren = 0;
	SymGetTypeInfo(m_hProcess, m_symbol->ModBase, m_symbol->TypeIndex, TI_GET_CHILDRENCOUNT, &numChildren);
	if (numChildren == 0)
	{
		// If the enum has no children we return something like
		// "(TheEnumType)0"
		std::stringstream strm;
		strm << "(" << m_typeName << ")" << value;
		m_enumValue = strm.str();
		return;
	}

	// we are responsible for allocating enough space to hold numChildren values
	TI_FINDCHILDREN_PARAMS* pParams = (TI_FINDCHILDREN_PARAMS*)new char[sizeof(TI_FINDCHILDREN_PARAMS) + numChildren * sizeof(ULONG)];
	pParams->Count = numChildren;
	pParams->Start = 0;

	SymGetTypeInfo(m_hProcess, m_symbol->ModBase, m_symbol->TypeIndex, TI_FINDCHILDREN, pParams);
	for (DWORD i = 0; i < numChildren; ++i)
	{
		ULONG curChild = pParams->ChildId[i];
		VARIANT vValue;
		VariantInit(&vValue);

		// For constant expressions TI_GET_VALUE can return the value of the expression
		SymGetTypeInfo(m_hProcess, m_symbol->ModBase, curChild, TI_GET_VALUE, &vValue);
		if (vValue.intVal == value)
		{
			LPWSTR symName = NULL;
			BOOL ret = SymGetTypeInfo(m_hProcess, m_symbol->ModBase, curChild, TI_GET_SYMNAME, &symName);
			if (ret == TRUE)
			{
				std::vector<char> ansi(wcslen(symName) + 1);
				WideCharToMultiByte(CP_ACP, 0, symName, ansi.size(), &ansi[0], ansi.size(), NULL, NULL);
				m_enumValue = &ansi[0];
				LocalFree(symName);
				delete[] pParams;
				return;
			}
		}
	}

	// If we reach that point that either means that no enum value had the same
	// value as the one requested or there was no possibility to get the name
	// of the correct value. So we return like there was no value at all.
	delete[] pParams;
	std::stringstream strm;
	strm << "(" << m_typeName << ")" << value;
	m_enumValue = strm.str();
}