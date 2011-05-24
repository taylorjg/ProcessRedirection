#include "stdafx.h"
#include "RunProcessWithRedirection.h"
#include "Wait.h"

const static DWORD BUFFER_SIZE = 4096;

//*****************************************************************************
//* Function Name: RunProcessAndWait
//*   Description: 
//*****************************************************************************
DWORD CRunProcessWithRedirection::RunProcessAndWait (const CString& p_strExe)
{
	return RunProcessAndWait(p_strExe, CString ());
}


//*****************************************************************************
//* Function Name: RunProcessAndWait
//*   Description: 
//*****************************************************************************
DWORD CRunProcessWithRedirection::RunProcessAndWait (
	const CString& p_strExe,
	const CString& p_strArgs)
{
	DWORD l_dwResult = ERROR_SUCCESS;

	m_strRedirectedStdOut.Empty ();
	m_strRedirectedStdErr.Empty ();

	LPTSTR l_lpszCopyOfArgs = NULL;
	LPTSTR l_lpszCommandLine = NULL;

	if (p_strArgs.GetLength () > 0) {
		int l_cchBuffer = p_strArgs.GetLength () + 1;
		l_lpszCopyOfArgs = new TCHAR[l_cchBuffer];
		_tcscpy_s (l_lpszCopyOfArgs, l_cchBuffer, p_strArgs);
		l_lpszCommandLine = l_lpszCopyOfArgs;
	}

	STARTUPINFO l_StartupInfo = {0};
	PROCESS_INFORMATION l_ProcessInformation = {0};
	CHandle l_hProcess;
	CHandle l_hThread;

	CHandle l_hStdOutReadThread;
	CHandle l_hStdErrReadThread;
	DWORD l_dwStdOutReadThreadId = 0;
	DWORD l_dwStdErrReadThreadId = 0;

	try {
		m_hInitSucceededEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
		if (m_hInitSucceededEvent == NULL) {
			DWORD l_dwLastError = GetLastError ();
			(void) _ftprintf (stderr, _T("CreateEvent(m_hInitSucceededEvent) failed with %ld.\n"), l_dwLastError);
			throw l_dwLastError;
		}

		m_hInitFailedEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
		if (m_hInitFailedEvent == NULL) {
			DWORD l_dwLastError = GetLastError ();
			(void) _ftprintf (stderr, _T("CreateEvent(m_hInitFailedEvent) failed with %ld.\n"), l_dwLastError);
			throw l_dwLastError;
		}

		SECURITY_ATTRIBUTES l_SecurityAttributes = {0};
		l_SecurityAttributes.nLength = sizeof (l_SecurityAttributes);
		l_SecurityAttributes.lpSecurityDescriptor = NULL;
		l_SecurityAttributes.bInheritHandle = TRUE;

		if (!CreatePipe (&m_hStdOutPipeRead, &m_hStdOutPipeRedirect, &l_SecurityAttributes, BUFFER_SIZE)) {
			DWORD l_dwLastError = GetLastError ();
			(void) _ftprintf (stderr, _T("CreatePipe(stdout) failed with %ld.\n"), l_dwLastError);
			throw l_dwLastError;
		}

		if (!SetHandleInformation (m_hStdOutPipeRead, HANDLE_FLAG_INHERIT, 0)) {
			DWORD l_dwLastError = GetLastError ();
			(void) _ftprintf (stderr, _T("SetHandleInformation(m_hStdOutPipeRead) failed with %ld.\n"), l_dwLastError);
			throw l_dwLastError;
		}

		if (!CreatePipe (&m_hStdErrPipeRead, &m_hStdErrPipeRedirect, &l_SecurityAttributes, BUFFER_SIZE)) {
			DWORD l_dwLastError = GetLastError ();
			(void) _ftprintf (stderr, _T("CreatePipe(stderr) failed with %ld.\n"), l_dwLastError);
			throw l_dwLastError;
		}

		if (!SetHandleInformation (m_hStdErrPipeRead, HANDLE_FLAG_INHERIT, 0)) {
			DWORD l_dwLastError = GetLastError ();
			(void) _ftprintf (stderr, _T("SetHandleInformation(m_hStdErrPipeRead) failed with %ld.\n"), l_dwLastError);
			throw l_dwLastError;
		}

		l_hStdOutReadThread = CreateThread (NULL, 0, &StdOutReadThreadStartRoutineStatic, this, 0, &l_dwStdOutReadThreadId);
		if (l_hStdOutReadThread == NULL) {
			DWORD l_dwLastError = GetLastError ();
			(void) _ftprintf (stderr, _T("CreateThread(stdout) failed with %ld.\n"), l_dwLastError);
			SetEvent (m_hInitFailedEvent);
			throw l_dwLastError;
		}

		l_hStdErrReadThread = CreateThread (NULL, 0, &StdErrReadThreadStartRoutineStatic, this, 0, &l_dwStdErrReadThreadId);
		if (l_hStdErrReadThread == NULL) {
			DWORD l_dwLastError = GetLastError ();
			(void) _ftprintf (stderr, _T("CreateThread(stderr) failed with %ld.\n"), l_dwLastError);
			SetEvent (m_hInitFailedEvent);
			throw l_dwLastError;
		}

		// Stick that in your pipe and smoke it!
		l_StartupInfo.cb = sizeof (l_StartupInfo);
		l_StartupInfo.dwFlags = STARTF_USESTDHANDLES;
		l_StartupInfo.hStdInput = GetStdHandle (STD_INPUT_HANDLE);
		l_StartupInfo.hStdOutput = m_hStdOutPipeRedirect;
		l_StartupInfo.hStdError = m_hStdErrPipeRedirect;

		if (!CreateProcess (
			p_strExe,					// lpApplicationName
			l_lpszCommandLine,			// lpCommandLine
			NULL,						// lpProcessAttributes
			NULL,						// lpThreadAttributes
			TRUE,						// bInheritHandles
			0,							// dwCreationFlags
			NULL,						// lpEnvironment
			NULL,						// lpCurrentDirectory
			&l_StartupInfo,				// lpStartupInfo
			&l_ProcessInformation))		// lpProcessInformation
		{
			DWORD l_dwLastError = GetLastError ();
			(void) _ftprintf (stderr, _T("CreateProcess() failed with %ld.\n"), l_dwLastError);
			SetEvent (m_hInitFailedEvent);
			throw l_dwLastError;
		}

		l_hProcess = l_ProcessInformation.hProcess;
		l_hThread = l_ProcessInformation.hThread;

		if (!SetEvent (m_hInitSucceededEvent)) {
			DWORD l_dwLastError = GetLastError ();
			(void) _ftprintf (stderr, _T("SetEvent(m_hInitSucceededEvent) failed with %ld.\n"), l_dwLastError);
			throw l_dwLastError;
		}

		(void) Wait (
			INFINITE,		// p_dwTimeout
			FALSE,			// p_bWaitAll
			FALSE,			// p_bAlertable
			1,				// 1
			l_hProcess);	// handle 1

		// Break the pipes by closing the write ends of the pipes.
		m_hStdOutPipeRedirect.Close ();
		m_hStdErrPipeRedirect.Close ();
	}
	catch (DWORD dwException) {
		l_dwResult = dwException;
	}

	int l_iNumHandles = 0;
	HANDLE l_haHandles[2] = { NULL, NULL };

	if (l_hStdOutReadThread != NULL) {
		l_haHandles[l_iNumHandles++] = l_hStdOutReadThread;
	}
	if (l_hStdErrReadThread != NULL) {
		l_haHandles[l_iNumHandles++] = l_hStdErrReadThread;
	}

	if (l_iNumHandles > 0) {
		DWORD l_dwWaitResult = WaitForMultipleObjects (l_iNumHandles, l_haHandles, TRUE, INFINITE);
		if (l_dwWaitResult == WAIT_FAILED) {
			DWORD l_dwLastError = GetLastError ();
			(void) _ftprintf (stderr, _T("WaitForMultipleObjects(threads) failed with %ld.\n"), l_dwLastError);
			throw l_dwLastError;
		}
	}

	m_hInitSucceededEvent.Close ();
	m_hInitFailedEvent.Close ();

	m_hStdOutPipeRead.Close ();
	m_hStdErrPipeRead.Close ();
	m_hStdOutPipeRedirect.Close ();
	m_hStdErrPipeRedirect.Close ();

	l_hProcess.Close ();
	l_hThread.Close ();

	delete[] l_lpszCopyOfArgs;
	l_lpszCopyOfArgs = NULL;

	return l_dwResult;
}


//*****************************************************************************
//* Function Name: StdOutReadThreadStartRoutineStatic
//*   Description: 
//*****************************************************************************
DWORD WINAPI CRunProcessWithRedirection::StdOutReadThreadStartRoutineStatic (LPVOID lpvClosure)
{
	CRunProcessWithRedirection* l_pThis = static_cast<CRunProcessWithRedirection*>(lpvClosure);
	return l_pThis->StdOutReadThreadStartRoutine ();
}


//*****************************************************************************
//* Function Name: StdErrReadThreadStartRoutineStatic
//*   Description: 
//*****************************************************************************
DWORD WINAPI CRunProcessWithRedirection::StdErrReadThreadStartRoutineStatic (LPVOID lpvClosure)
{
	CRunProcessWithRedirection* l_pThis = static_cast<CRunProcessWithRedirection*>(lpvClosure);
	return l_pThis->StdErrReadThreadStartRoutine ();
}


//*****************************************************************************
//* Function Name: StdOutReadThreadStartRoutine
//*   Description: 
//*****************************************************************************
DWORD CRunProcessWithRedirection::StdOutReadThreadStartRoutine ()
{
	DWORD l_dwResult = ERROR_SUCCESS;

	HANDLE l_haHandles[2] = { m_hInitSucceededEvent, m_hInitFailedEvent };

	DWORD l_dwWaitResult = WaitForMultipleObjects (2, l_haHandles, FALSE, INFINITE);
	if (l_dwWaitResult == WAIT_FAILED) {
		DWORD l_dwLastError = GetLastError ();
		(void) _ftprintf (stderr, _T("WaitForMultipleObjects(events) failed with %ld.\n"), l_dwLastError);
		return l_dwLastError;
	}
	if (l_dwWaitResult == (WAIT_OBJECT_0 + 1)) {
		(void) _ftprintf (stderr, _T("m_hInitFailedEvent set - StdOutReadThreadStartRoutine quitting\n"));
		return l_dwResult;
	}

	// Keep reading from stdout pipe until error/EOF/broken pipe etc.
	for (;;) {

		// ANSI buffer.
		CHAR l_szBuffer[BUFFER_SIZE + 1] = {0};
		DWORD l_dwNumberOfBytesRead = 0;

		if (ReadFile (m_hStdOutPipeRead, l_szBuffer, BUFFER_SIZE, &l_dwNumberOfBytesRead, NULL)) {

			if (l_dwNumberOfBytesRead == 0) {
				break;
			}

			// Convert ANSI buffer to TCHAR buffer via _bstr_t.
			_bstr_t l_sbstrBuffer (l_szBuffer);
			m_strRedirectedStdOut += static_cast<LPCTSTR>(l_sbstrBuffer);
		}
		else {
			DWORD l_dwLastError = GetLastError ();
			if (l_dwLastError != ERROR_BROKEN_PIPE) {
				(void) _ftprintf (stderr, _T("ReadFile(stdout) failed with %ld.\n"), l_dwLastError);
				l_dwResult = l_dwLastError;
			}
			break;
		}
	}

	return l_dwResult;
}


//*****************************************************************************
//* Function Name: StdErrReadThreadStartRoutine
//*   Description: 
//*****************************************************************************
DWORD CRunProcessWithRedirection::StdErrReadThreadStartRoutine ()
{
	DWORD l_dwResult = ERROR_SUCCESS;

	HANDLE l_haHandles[2] = { m_hInitSucceededEvent, m_hInitFailedEvent };

	DWORD l_dwWaitResult = WaitForMultipleObjects (2, l_haHandles, FALSE, INFINITE);
	if (l_dwWaitResult == WAIT_FAILED) {
		DWORD l_dwLastError = GetLastError ();
		(void) _ftprintf (stderr, _T("WaitForMultipleObjects(events) failed with %ld.\n"), l_dwLastError);
		return l_dwLastError;
	}
	if (l_dwWaitResult == (WAIT_OBJECT_0 + 1)) {
		(void) _ftprintf (stderr, _T("m_hInitFailedEvent set - StdErrReadThreadStartRoutine quitting\n"));
		return l_dwResult;
	}

	// Keep reading from stderr pipe until error/EOF/broken pipe etc.
	for (;;) {

		// ANSI buffer.
		CHAR l_szBuffer[BUFFER_SIZE + 1] = {0};
		DWORD l_dwNumberOfBytesRead = 0;

		if (ReadFile (m_hStdErrPipeRead, l_szBuffer, BUFFER_SIZE, &l_dwNumberOfBytesRead, NULL)) {

			if (l_dwNumberOfBytesRead == 0) {
				break;
			}

			// Convert ANSI buffer to TCHAR buffer via _bstr_t.
			_bstr_t l_sbstrBuffer (l_szBuffer);
			m_strRedirectedStdErr += static_cast<LPCTSTR>(l_sbstrBuffer);
		}
		else {
			DWORD l_dwLastError = GetLastError ();
			if (l_dwLastError != ERROR_BROKEN_PIPE) {
				(void) _ftprintf (stderr, _T("ReadFile(stderr) failed with %ld.\n"), l_dwLastError);
				l_dwResult = l_dwLastError;
			}
			break;
		}
	}

	return l_dwResult;
}
