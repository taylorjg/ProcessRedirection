#ifndef __RunProcessWithRedirectionAsync_h__
#define __RunProcessWithRedirectionAsync_h__

#include "Handle.h"

class CMyOverlapped;

class CRunProcessWithRedirectionAsync
{
public:
	DWORD RunProcessAndWait (const CString& p_strExe);
	DWORD RunProcessAndWait (const CString& p_strExe, const CString& p_strArgs);

	CString GetRedirectedStdOut (void) const { return m_strRedirectedStdOut; }
	CString GetRedirectedStdErr (void) const { return m_strRedirectedStdErr; }

private:

	DWORD StartAsyncReadOnStdOut (void);
	DWORD StartAsyncReadOnStdErr (void);

	static void CALLBACK StdOutReadCompletionRoutineStatic (
		DWORD			p_dwErrorCode,
		DWORD			p_dwNumberOfBytesTransferred,
		LPOVERLAPPED	p_pOverlapped);

	static void CALLBACK StdErrReadCompletionRoutineStatic (
		DWORD			p_dwErrorCode,
		DWORD			p_dwNumberOfBytesTransferred,
		LPOVERLAPPED	p_pOverlapped);

	void StdOutReadCompletionRoutine (
		DWORD			p_dwErrorCode,
		DWORD			p_dwNumberOfBytesTransferred,
		CMyOverlapped*	p_pMyOverlapped);

	void StdErrReadCompletionRoutine (
		DWORD			p_dwErrorCode,
		DWORD			p_dwNumberOfBytesTransferred,
		CMyOverlapped*	p_pMyOverlapped);

	CString m_strRedirectedStdOut;
	CString m_strRedirectedStdErr;

	CHandle	m_hFinishedReadingStdOutEvent;
	CHandle	m_hFinishedReadingStdErrEvent;

	CHandle m_hStdOutPipeRead;	// Used by the parent process to read stdout from the child process
	CHandle m_hStdErrPipeRead;	// Used by the parent process to read stderr from the child process

	CHandle m_hStdOutPipeRedirect;	// Child process activity to stdout is redirected via this handle
	CHandle m_hStdErrPipeRedirect;	// Child process activity to stderr is redirected via this handle
};

#endif
