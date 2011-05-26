#include "stdafx.h"
#include "RunProcessWithRedirectionAsync.h"
#include "Wait.h"

//*****************************************************************************
//* Function Name: RunProcessAndWait
//*   Description: 
//*****************************************************************************
DWORD CRunProcessWithRedirectionAsync::RunProcessAndWait (const CString& p_strExe)
{
	return RunProcessAndWait(p_strExe, CString ());
}


//*****************************************************************************
//* Function Name: RunProcessAndWait
//*   Description: 
//*****************************************************************************
DWORD CRunProcessWithRedirectionAsync::RunProcessAndWait (
	const CString& p_strExe,
	const CString& p_strArgs)
{
	DWORD l_dwResult = ERROR_SUCCESS;

	m_RedirectedStdOut.Reset ();
	m_RedirectedStdErr.Reset ();

	LPTSTR l_lpszCopyOfArgs = NULL;
	LPTSTR l_lpszCommandLine = NULL;

	if (p_strArgs.GetLength () > 0) {
		int l_cchBuffer = p_strArgs.GetLength () + 1;
		l_lpszCopyOfArgs = new TCHAR[l_cchBuffer];
		_tcscpy_s (l_lpszCopyOfArgs, l_cchBuffer, p_strArgs);
		l_lpszCommandLine = l_lpszCopyOfArgs;
	}

	CHandle l_hProcess;
	CHandle l_hThread;

	try {
		m_RedirectedStdOut.Initialise (_T("StdOut"));
		m_RedirectedStdErr.Initialise (_T("StdErr"));

		// Stick that in your pipe and smoke it!
		STARTUPINFO l_StartupInfo = {0};
		l_StartupInfo.cb = sizeof (l_StartupInfo);
		l_StartupInfo.dwFlags = STARTF_USESTDHANDLES;
		l_StartupInfo.hStdInput = GetStdHandle (STD_INPUT_HANDLE);
		l_StartupInfo.hStdOutput = m_RedirectedStdOut.GetPipeWritingHandle ();
		l_StartupInfo.hStdError = m_RedirectedStdErr.GetPipeWritingHandle ();

		PROCESS_INFORMATION l_ProcessInformation = {0};
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
			throw l_dwLastError;
		}

		l_hProcess = l_ProcessInformation.hProcess;
		l_hThread = l_ProcessInformation.hThread;

		m_RedirectedStdOut.BeginReading ();
		m_RedirectedStdErr.BeginReading ();

		(void) Wait (
			INFINITE,		// p_dwTimeout
			FALSE,			// p_bWaitAll
			TRUE,			// p_bAlertable
			1,				// p_iNumHandles
			l_hProcess);	// handle 1

		// Break the pipes by closing the writing ends of the pipes.
		m_RedirectedStdOut.CloseWritingEnd ();
		m_RedirectedStdErr.CloseWritingEnd ();

		(void) Wait (
			INFINITE,										// p_dwTimeout
			TRUE,											// p_bWaitAll
			TRUE,											// p_bAlertable
			2,												// p_iNumHandles
			m_RedirectedStdOut.GetFinishedReadingEvent (),	// handle 1
			m_RedirectedStdErr.GetFinishedReadingEvent ());	// handle 2
	}
	catch (DWORD dwException) {
		l_dwResult = dwException;
	}

	l_hProcess.Close ();
	l_hThread.Close ();

	delete[] l_lpszCopyOfArgs;
	l_lpszCopyOfArgs = NULL;

	return l_dwResult;
}
