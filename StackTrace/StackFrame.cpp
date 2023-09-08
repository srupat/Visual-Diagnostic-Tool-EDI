#include "StackFrame.h"
#include<iostream>
#pragma comment(lib, "Dbghelp.lib")

// Later we use loadParam to indicate if parameters of local variables should be loaded
struct SymEnumParam
{
	StackFrame* pFrame;
	bool loadParam;
};

struct SymEnumLocals
{
	StackFrame* pFrame;
	bool loadLocal;
};

StackFrame::StackFrame(STACKFRAME64& stackFrame,HANDLE handle) : frame(stackFrame), hProcess(handle)
{
	
	//std::cout << " Address offset in Stack Frame: " << frame.AddrPC.Offset << std::endl;

	LoadParameters();
	if (isSetContext == 0)
		return;
	LoadLocals();
	PSYMBOL_INFO inf = (PSYMBOL_INFO)new char[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
	inf->MaxNameLen = MAX_SYM_NAME;
	inf->SizeOfStruct = sizeof(SYMBOL_INFO);
	DWORD64 disp;
	SymFromAddr(hProcess, stackFrame.AddrPC.Offset, &disp, inf);
	m_functionName = inf->Name;

	LoadCConv(inf);

	// for a function TI_GET_TYPE returns the type of the return value as a function
	// in the end resolves to that type
	DWORD type = 0;
	SymGetTypeInfo(hProcess, inf->ModBase, inf->TypeIndex, TI_GET_TYPE, &type);

	// we dont care about the other values because in FunctionObject only the
	// ModBase and TypeIndex are used (lazy ;P)
	inf->TypeIndex = type;
	m_returnType = new FunctionObject(inf,hProcess);
	delete[] inf;
}

BOOL __stdcall EnumVariablesCallback(PSYMBOL_INFO info, ULONG size, PVOID param)
{

	SymEnumParam* sep = (SymEnumParam*)param;

	if (sep->loadParam)
		sep->pFrame->VariableEnumProc(info);
	return TRUE;	
}

BOOL __stdcall EnumSymbolCallback(PSYMBOL_INFO inf, ULONG size, PVOID param)
{
	SymEnumParam* sep = (SymEnumParam*)param;

	if (sep->loadParam)
		sep->pFrame->ParameterEnumProc(inf);
	return TRUE;
}

void StackFrame::LoadParameters()
{
	IMAGEHLP_STACK_FRAME curFrame = { 0 };
	curFrame.InstructionOffset = frame.AddrPC.Offset;
	
	SymSetContext(hProcess, &curFrame, NULL);
	if(GetLastError() != 0)
	{
		isSetContext = 0;
		//std::cout << "symSetContext failed " << GetLastError() << std::endl;
	}
	else
	{
		isSetContext = 1;
	}

	SymEnumParam param =
	{
		this,
		true
	};

	SymEnumSymbols(hProcess, 0, NULL, EnumSymbolCallback, &param);
}

void StackFrame::LoadLocals()
{
	IMAGEHLP_STACK_FRAME curFrame = { 0 };

	curFrame.InstructionOffset = frame.AddrPC.Offset;
	SymSetContext(hProcess, &curFrame, NULL);

	SymEnumParam local =
	{
		this,
		true
	};
	SymEnumSymbols(hProcess, 0, NULL, EnumVariablesCallback, &local);
}

void StackFrame::ParameterEnumProc(PSYMBOL_INFO inf)
{
	// SYMFLAG_PARAMETER not set -> no parameter
	if ((inf->Flags & SYMFLAG_PARAMETER) == 0)
		return ;

	FunctionObject fo(inf, frame, hProcess);
	m_parameters.push_back(fo);

}

void StackFrame::VariableEnumProc(PSYMBOL_INFO inf)
{
	// SYMFLAG_PARAMETER not set -> no parameter
	if (inf->Flags & SYMFLAG_LOCAL && (inf->Flags & SYMFLAG_PARAMETER) == 0)
	{
		FunctionObject fo(inf, frame, hProcess);
		localVariables.push_back(fo);
	}
}

void StackFrame::LoadCConv(PSYMBOL_INFO info)
{
	DWORD callConv = 0;
	if (!SymGetTypeInfo(hProcess, info->ModBase, info->TypeIndex, TI_GET_CALLING_CONVENTION, &callConv))
	{
		std::cout << "SymGetTypeInfo() failed" << GetLastError() <<std::endl;
	}

	switch (callConv)
	{
	case 0:
	case 1:
		m_callConvention = "__cdecl";
		break;
	case 2:
	case 3:
		m_callConvention = "__pascal";
		break;
	case 4:
	case 5:
		m_callConvention = "__fastcall";
		break;
	case 7:
	case 8:
		m_callConvention = "__stdcall";
		break;
	case 9:
	case 10:
		m_callConvention = "__syscall";
		break;
	case 11:
		m_callConvention = "__thiscall";
		break;
	default:
		m_callConvention = "__usercall"; // take that for all the "strange" stuff like __sh5call (people that own the IDA disassembler may know __usercall :D)
		break;
	}
}

void StackFrame::ToString()
{
	std::stringstream ret;
	ret << m_returnType->ToString() << " ";
	ret << m_callConvention << " ";
	ret << m_functionName << "(";
	for (UINT i = 0; i < m_parameters.size(); ++i)
	{
		if (i > 0)
			ret << ", ";
		ret << m_parameters[i].ToString();
	}
	ret << ")";
	std::cout << std::endl;
	std::stringstream ret2;
	for (UINT i = 0; i < localVariables.size(); ++i)
	{
		if (i > 0)
			ret2 << "\n ";
		ret2 << localVariables[i].ToString();
	}
	std::cout << ret.str();
	std::cout << ret2.str();

}

int StackFrame::getContextFlag()
{
	return isSetContext;
}
