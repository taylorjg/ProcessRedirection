#ifndef __RunProcessWithRedirection_h__
#define __RunProcessWithRedirection_h__

#include "Handle.h"

class CRunProcessWithRedirection
{
public:
	DWORD RunProcessAndWait (const CString& p_strExe);
	DWORD RunProcessAndWait (const CString& p_strExe, const CString& p_strArgs);

	CString GetRedirectedStdOut (void) const { return m_strRedirectedStdOut; }
	CString GetRedirectedStdErr (void) const { return m_strRedirectedStdErr; }

private:

	static DWORD WINAPI StdOutReadThreadStartRoutineStatic (LPVOID lpvClosure);
	static DWORD WINAPI StdErrReadThreadStartRoutineStatic (LPVOID lpvClosure);

	DWORD StdOutReadThreadStartRoutine ();
	DWORD StdErrReadThreadStartRoutine ();

	CString m_strRedirectedStdOut;
	CString m_strRedirectedStdErr;

	CHandle m_hInitSucceededEvent;
	CHandle	m_hInitFailedEvent;

	CHandle m_hStdOutPipeRead;	// Used by the parent process to read stdout from the child process
	CHandle m_hStdErrPipeRead;	// Used by the parent process to read stderr from the child process

	CHandle m_hStdOutPipeRedirect;	// Child process activity to stdout is redirected via this handle
	CHandle m_hStdErrPipeRedirect;	// Child process activity to stderr is redirected via this handle
};

#endif
