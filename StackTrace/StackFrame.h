#pragma once
#include <Windows.h>
#include <DbgHelp.h>
#include <string>
#include <vector>
#include <sstream>

#include "FunctionObject.h"

struct Node {
	struct Node* next;
	std::string nameOfFunc;
	std::string returnType;
	struct Table* table;
};

struct Table {
	std::string datatype;
	std::string name;
	std::string value;
	std::string address;
};


class StackFrame
{
	friend BOOL __stdcall EnumSymbolCallback(PSYMBOL_INFO inf, ULONG size, PVOID param);
	friend BOOL __stdcall EnumVariablesCallback(PSYMBOL_INFO info, ULONG size, PVOID param);
	HANDLE hProcess;
	STACKFRAME64& frame;

	std::vector<FunctionObject> m_parameters;
	std::vector<FunctionObject> localVariables;

	std::string m_functionName;
	FunctionObject* m_returnType;
	std::string m_callConvention;
	int isSetContext = 0;

	void LoadParameters();
	void LoadLocals();
	void LoadCConv(PSYMBOL_INFO sym);

	void ParameterEnumProc(PSYMBOL_INFO sym);
	void VariableEnumProc(PSYMBOL_INFO sym);

	StackFrame(StackFrame& other, HANDLE handle) : frame(other.frame), hProcess(handle) { }
	void operator=(StackFrame&) { }

public:
	StackFrame(STACKFRAME64& stackFrame, HANDLE handle);
	~StackFrame()
	{
		delete m_returnType;
		m_parameters.clear();
	}

	void ToString();
	int getContextFlag();
	struct Node* createDataStructure(struct Node* head);

	//abheerav
};