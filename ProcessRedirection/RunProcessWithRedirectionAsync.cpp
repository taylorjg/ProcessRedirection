#include "stdafx.h"
#include "RunProcessWithRedirectionAsync.h"
#include "Wait.h"

const static DWORD BUFFER_SIZE = 4096;

class CMyOverlapped : public OVERLAPPED
{
public:
	CMyOverlapped (CRunProcessWithRedirectionAsync* p_pThis) :
		m_pThis (p_pThis)
	{
		// Zero-out the OVERLAPPED part of this instance.
		ZeroMemory (this, sizeof (OVERLAPPED));

		// Zero-out the buffer.
		ZeroMemory (m_Buffer, sizeof (m_Buffer));
	}

	LPVOID GetBufferAddress (void) { return m_Buffer; }
	LPCVOID GetBufferAddress (void) const { return m_Buffer; }
	DWORD GetBufferSize (void) const { return BUFFER_SIZE; }

	CRunProcessWithRedirectionAsync* GetThis (void) { return m_pThis; }

private:
	CRunProcessWithRedirectionAsync* m_pThis;
	CHAR m_Buffer[BUFFER_SIZE + 1];
};


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

	try {
		m_hFinishedReadingStdOutEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
		m_hFinishedReadingStdErrEvent = CreateEvent (NULL, TRUE, FALSE, NULL);

		SECURITY_ATTRIBUTES l_SecurityAttributes = {0};
		l_SecurityAttributes.nLength = sizeof (l_SecurityAttributes);
		l_SecurityAttributes.lpSecurityDescriptor = NULL;
		l_SecurityAttributes.bInheritHandle = TRUE;

		TCHAR l_szStdOutPipeName[256] = {0};
		(void) _stprintf_s (l_szStdOutPipeName, 256, _T("\\\\.\\pipe\\ProcessRedirection_%08X_StdOut"), GetCurrentProcessId ());

		TCHAR l_szStdErrPipeName[256] = {0};
		(void) _stprintf_s (l_szStdErrPipeName, 256, _T("\\\\.\\pipe\\ProcessRedirection_%08X_StdErr"), GetCurrentProcessId ());

		DWORD dwOpenMode = PIPE_ACCESS_INBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE | FILE_FLAG_OVERLAPPED;
		DWORD dwPipeMode = PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT;

		m_hStdOutPipeRead = CreateNamedPipe (
			l_szStdOutPipeName,			// lpName
			dwOpenMode,					// dwOpenMode
			dwPipeMode,					// dwPipeMode
			1,							// nMaxInstances
			BUFFER_SIZE,				// nOutBufferSize
			BUFFER_SIZE,				// nInBufferSize
			INFINITE,					// nDefaultTimeOut
			NULL);						// lpSecurityAttributes
		if (m_hStdOutPipeRead == INVALID_HANDLE_VALUE) {
			DWORD l_dwLastError = GetLastError ();
			(void) _ftprintf (stderr, _T("CreateNamedPipe(stdout) failed with %ld.\n"), l_dwLastError);
			throw l_dwLastError;
		}

		m_hStdOutPipeRedirect = CreateFile (
			l_szStdOutPipeName,			// lpFileName
			GENERIC_WRITE,				// dwDesiredAccess
			0,							// dwShareMode
			&l_SecurityAttributes,		// lpSecurityAttributes,
			OPEN_EXISTING,				// dwCreationDisposition
			0,							// dwFlagsAndAttributes
			NULL);						// hTemplateFile
		if (m_hStdOutPipeRedirect == INVALID_HANDLE_VALUE) {
			DWORD l_dwLastError = GetLastError ();
			(void) _ftprintf (stderr, _T("CreateFile(stdout) failed with %ld.\n"), l_dwLastError);
			throw l_dwLastError;
		}

		m_hStdErrPipeRead = CreateNamedPipe (
			l_szStdErrPipeName,			// lpName
			dwOpenMode,					// dwOpenMode
			dwPipeMode,					// dwPipeMode
			1,							// nMaxInstances
			BUFFER_SIZE,				// nOutBufferSize
			BUFFER_SIZE,				// nInBufferSize
			INFINITE,					// nDefaultTimeOut
			NULL);						// lpSecurityAttributes
		if (m_hStdErrPipeRead == INVALID_HANDLE_VALUE) {
			DWORD l_dwLastError = GetLastError ();
			(void) _ftprintf (stderr, _T("CreateNamedPipe(stderr) failed with %ld.\n"), l_dwLastError);
			throw l_dwLastError;
		}

		m_hStdErrPipeRedirect = CreateFile (
			l_szStdErrPipeName,			// lpFileName
			GENERIC_WRITE,				// dwDesiredAccess
			0,							// dwShareMode
			&l_SecurityAttributes,		// lpSecurityAttributes,
			OPEN_EXISTING,				// dwCreationDisposition
			0,							// dwFlagsAndAttributes
			NULL);						// hTemplateFile
		if (m_hStdErrPipeRedirect == INVALID_HANDLE_VALUE) {
			DWORD l_dwLastError = GetLastError ();
			(void) _ftprintf (stderr, _T("CreateFile(stderr) failed with %ld.\n"), l_dwLastError);
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
			throw l_dwLastError;
		}

		l_hProcess = l_ProcessInformation.hProcess;
		l_hThread = l_ProcessInformation.hThread;

		StartAsyncReadOnStdOut ();
		StartAsyncReadOnStdErr ();

		(void) Wait (
			INFINITE,		// p_dwTimeout
			FALSE,			// p_bWaitAll
			TRUE,			// p_bAlertable
			1,				// p_iNumHandles
			l_hProcess);	// handle 1

		// Break the pipes by closing the write ends of the pipes.
		m_hStdOutPipeRedirect.Close ();
		m_hStdErrPipeRedirect.Close ();

		(void) Wait (
			INFINITE,						// p_dwTimeout
			TRUE,							// p_bWaitAll
			TRUE,							// p_bAlertable
			2,								// p_iNumHandles
			m_hFinishedReadingStdOutEvent,	// handle 1
			m_hFinishedReadingStdErrEvent);	// handle 2
	}
	catch (DWORD dwException) {
		l_dwResult = dwException;
	}

	m_hStdOutPipeRead.Close ();
	m_hStdErrPipeRead.Close ();
	m_hStdOutPipeRedirect.Close ();
	m_hStdErrPipeRedirect.Close ();

	l_hProcess.Close ();
	l_hThread.Close ();

	m_hFinishedReadingStdOutEvent.Close ();
	m_hFinishedReadingStdErrEvent.Close ();

	delete[] l_lpszCopyOfArgs;
	l_lpszCopyOfArgs = NULL;

	return l_dwResult;
}


//*****************************************************************************
//* Function Name: StartAsyncReadOnStdOut
//*   Description: 
//*****************************************************************************
DWORD CRunProcessWithRedirectionAsync::StartAsyncReadOnStdOut (void)
{
	DWORD l_dwResult = ERROR_SUCCESS;

	CMyOverlapped* l_pMyOverlapped = new CMyOverlapped (this);

	if (ReadFileEx (
		m_hStdOutPipeRead,
		l_pMyOverlapped->GetBufferAddress (),
		l_pMyOverlapped->GetBufferSize (),
		l_pMyOverlapped,
		&StdOutReadCompletionRoutineStatic))
	{
		(void) _tprintf (_T("Async read started successfully on stdout\n"));
	}
	else {
		DWORD l_dwLastError = GetLastError ();
		(void) _ftprintf (stderr, _T("ReadFileEx(stdout) failed with %ld.\n"), l_dwLastError);
		delete l_pMyOverlapped;
		l_pMyOverlapped = NULL;
		l_dwResult = l_dwLastError;
	}

	return l_dwResult;
}


//*****************************************************************************
//* Function Name: StartAsyncReadOnStdErr
//*   Description: 
//*****************************************************************************
DWORD CRunProcessWithRedirectionAsync::StartAsyncReadOnStdErr (void)
{
	DWORD l_dwResult = ERROR_SUCCESS;

	CMyOverlapped* l_pMyOverlapped = new CMyOverlapped (this);

	if (ReadFileEx (
		m_hStdErrPipeRead,
		l_pMyOverlapped->GetBufferAddress (),
		l_pMyOverlapped->GetBufferSize (),
		l_pMyOverlapped,
		&StdErrReadCompletionRoutineStatic))
	{
		(void) _tprintf (_T("Async read started successfully on stderr\n"));
	}
	else {
		DWORD l_dwLastError = GetLastError ();
		(void) _ftprintf (stderr, _T("ReadFileEx(stderr) failed with %ld.\n"), l_dwLastError);
		delete l_pMyOverlapped;
		l_pMyOverlapped = NULL;
		l_dwResult = l_dwLastError;
	}

	return l_dwResult;
}


//*****************************************************************************
//* Function Name: StdOutReadCompletionRoutineStatic
//*   Description: 
//*****************************************************************************
void CRunProcessWithRedirectionAsync::StdOutReadCompletionRoutineStatic (
		DWORD			p_dwErrorCode,
		DWORD			p_dwNumberOfBytesTransferred,
		LPOVERLAPPED	p_pOverlapped)
{
	(void) _tprintf (_T("Inside StdOutReadCompletionRoutineStatic ()\n"));

	CMyOverlapped* l_pMyOverlapped = static_cast<CMyOverlapped*>(p_pOverlapped);

	l_pMyOverlapped->GetThis ()->StdOutReadCompletionRoutine (
		p_dwErrorCode,
		p_dwNumberOfBytesTransferred,
		l_pMyOverlapped);
}


//*****************************************************************************
//* Function Name: StdErrReadCompletionRoutineStatic
//*   Description: 
//*****************************************************************************
void CRunProcessWithRedirectionAsync::StdErrReadCompletionRoutineStatic (
		DWORD			p_dwErrorCode,
		DWORD			p_dwNumberOfBytesTransferred,
		LPOVERLAPPED	p_pOverlapped)
{
	(void) _tprintf (_T("Inside StdErrReadCompletionRoutineStatic ()\n"));

	CMyOverlapped* l_pMyOverlapped = static_cast<CMyOverlapped*>(p_pOverlapped);

	l_pMyOverlapped->GetThis ()->StdErrReadCompletionRoutine (
		p_dwErrorCode,
		p_dwNumberOfBytesTransferred,
		l_pMyOverlapped);
}


//*****************************************************************************
//* Function Name: StdOutReadCompletionRoutine
//*   Description: 
//*****************************************************************************
void CRunProcessWithRedirectionAsync::StdOutReadCompletionRoutine (
		DWORD			p_dwErrorCode,
		DWORD			p_dwNumberOfBytesTransferred,
		CMyOverlapped*	p_pMyOverlapped)
{
	(void) _tprintf (_T("Inside StdOutReadCompletionRoutine ()\n"));

	(void) _tprintf (_T("p_dwErrorCode = %ld\n"), p_dwErrorCode);
	(void) _tprintf (_T("p_dwNumberOfBytesTransferred = %ld\n"), p_dwNumberOfBytesTransferred);

	BOOL l_bSuccess = FALSE;

	if (p_dwErrorCode == ERROR_SUCCESS) {

		_bstr_t l_sbstrBuffer (static_cast<LPCSTR>(p_pMyOverlapped->GetBufferAddress ()));
		m_strRedirectedStdOut += static_cast<LPCTSTR>(l_sbstrBuffer);

		if (p_dwNumberOfBytesTransferred > 0) {
			if (StartAsyncReadOnStdOut () == ERROR_SUCCESS) {
				l_bSuccess = TRUE;
			}
		}
	}

	delete p_pMyOverlapped;
	p_pMyOverlapped = NULL;

	if (!l_bSuccess) {
		(void) _tprintf (_T("Setting m_hFinishedReadingStdOutEvent...\n"));
		SetEvent (m_hFinishedReadingStdOutEvent);
	}
}


//*****************************************************************************
//* Function Name: StdErrReadCompletionRoutine
//*   Description: 
//*****************************************************************************
void CRunProcessWithRedirectionAsync::StdErrReadCompletionRoutine (
		DWORD			p_dwErrorCode,
		DWORD			p_dwNumberOfBytesTransferred,
		CMyOverlapped*	p_pMyOverlapped)
{
	(void) _tprintf (_T("Inside StdErrReadCompletionRoutine ()\n"));

	(void) _tprintf (_T("p_dwErrorCode = %ld\n"), p_dwErrorCode);
	(void) _tprintf (_T("p_dwNumberOfBytesTransferred = %ld\n"), p_dwNumberOfBytesTransferred);

	BOOL l_bSuccess = FALSE;

	if (p_dwErrorCode == ERROR_SUCCESS) {

		_bstr_t l_sbstrBuffer (static_cast<LPCSTR>(p_pMyOverlapped->GetBufferAddress ()));
		m_strRedirectedStdErr += static_cast<LPCTSTR>(l_sbstrBuffer);

		if (p_dwNumberOfBytesTransferred > 0) {
			if (StartAsyncReadOnStdErr () == ERROR_SUCCESS) {
				l_bSuccess = TRUE;
			}
		}
	}

	delete p_pMyOverlapped;
	p_pMyOverlapped = NULL;

	if (!l_bSuccess) {
		(void) _tprintf (_T("Setting m_hFinishedReadingStdErrEvent...\n"));
		SetEvent (m_hFinishedReadingStdErrEvent);
	}
}
