#pragma once
//#include"pch.h"

#include <vector>
#include "StackFrame.h"

class StackTrace
{
	std::vector<StackFrame*> m_frames;
	friend std::ostream& operator << (std::ostream& strm, const StackTrace& trace);
	HANDLE mPprocHandle;
	HANDLE hThread;
	HANDLE hThreadSnap;

	// We have a non trivial destructor -> mark copy constructor and operator =
	// as private or implement them so that copying is done correctly
	StackTrace(StackTrace&) { }
	void operator=(StackTrace&) { }

public:
	StackTrace();
	~StackTrace();
	void printStackTrace(HANDLE hProcess);
	std::vector<std::string> strFunctionNames;
};

std::ostream& operator << (std::ostream& strm, const StackTrace& trace);