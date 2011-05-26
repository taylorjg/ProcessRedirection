#include "stdafx.h"
#include "RedirectionTarget.h"

const static DWORD BUFFER_SIZE = 4096;

/*
 * Each time we call ReadFileEx(), we need to pass an OVERLAPPED
 * data structure and a data buffer into which to read. Also, the
 * completion routine passed to ReadFileEx() is a static member function of
 * class CRedirectionTarget. Being a static member function,
 * the completion routine will not have any sort of link back to the instance
 * of CRedirectionTarget to which it pertains. Taking all these
 * things into consideration, an elegant design is to derive a class from
 * OVERLAPPED. This effectively allows us to add additional state to the
 * OVERLAPPED structure. In our case, we add a pointer to the instance of
 * CRedirectionTarget and the data buffer too. We then
 * allocate an instance of this derived class for each call to
 * ReadFileEx(). Thus, inside the completion routine, we can cast
 * the LPOVERLAPPED parameter to a pointer to our derived class and we
 * then have access to all the state that we need.
 *
 * REFERENCE:
 *
 * Pattern-Oriented Software Architecture, Volume 2.
 * - Patterns for Concurrent and Networked Objects
 *
 * By :-
 *
 *	Douglas Schmidt
 *	Michael Stal
 *	Hans Rohnert
 *	Frank Buschmann
 *
 */
class CMyOverlapped : public OVERLAPPED
{
public:
	CMyOverlapped (CRedirectionTarget& p_rThis) :
		m_rThis (p_rThis)
	{
		// Zero-out the OVERLAPPED part of this instance.
		// OVERLAPPED is a typedef struct so it does not
		// have its own constructor.
		ZeroMemory (this, sizeof (OVERLAPPED));

		// Zero-out the buffer.
		ZeroMemory (m_Buffer, sizeof (m_Buffer));
	}

	LPVOID GetBufferAddress (void) { return m_Buffer; }
	LPCVOID GetBufferAddress (void) const { return m_Buffer; }
	DWORD GetBufferSize (void) const { return BUFFER_SIZE; }
	CRedirectionTarget& GetThis (void) { return m_rThis; }

private:
	CRedirectionTarget& m_rThis;

	// ANSI char buffer. Includes an extra byte for the NULL terminator.
	CHAR m_Buffer[BUFFER_SIZE + 1];
};


//*****************************************************************************
//* Function Name: Initialise
//*   Description: Perform the following steps :-
//*                    Create the server (reading) end of the named pipe
//*                    Create the client (writing) end of the named pipe
//*                    Create the finished reading event
//*****************************************************************************
void CRedirectionTarget::Initialise (const CString& p_strName)
{
	m_strName = p_strName;

	TCHAR l_szPipeName[256] = {0};
	(void) _stprintf_s (
		l_szPipeName,
		256,
		_T("\\\\.\\pipe\\RedirectionTarget_%08X_%s"),
		GetCurrentProcessId (),
		GetName ());

	DWORD dwOpenMode =
		PIPE_ACCESS_INBOUND |
		FILE_FLAG_FIRST_PIPE_INSTANCE |
		FILE_FLAG_OVERLAPPED;

	DWORD dwPipeMode =
		PIPE_TYPE_BYTE |
		PIPE_READMODE_BYTE |
		PIPE_WAIT;

	m_hPipeRead = CreateNamedPipe (
		l_szPipeName,				// lpName
		dwOpenMode,					// dwOpenMode
		dwPipeMode,					// dwPipeMode
		1,							// nMaxInstances
		BUFFER_SIZE,				// nOutBufferSize
		BUFFER_SIZE,				// nInBufferSize
		INFINITE,					// nDefaultTimeOut
		NULL);						// lpSecurityAttributes

	if (m_hPipeRead == INVALID_HANDLE_VALUE) {
		DWORD l_dwLastError = GetLastError ();
		(void) _ftprintf (stderr, _T("CreateNamedPipe(%s) failed with %ld.\n"), GetName (), l_dwLastError);
		throw l_dwLastError;
	}

	SECURITY_ATTRIBUTES l_SecurityAttributes = {0};
	l_SecurityAttributes.nLength = sizeof (l_SecurityAttributes);
	l_SecurityAttributes.lpSecurityDescriptor = NULL;
	l_SecurityAttributes.bInheritHandle = TRUE;

	m_hPipeWrite = CreateFile (
		l_szPipeName,				// lpFileName
		FILE_WRITE_DATA,			// dwDesiredAccess
		0,							// dwShareMode
		&l_SecurityAttributes,		// lpSecurityAttributes,
		OPEN_EXISTING,				// dwCreationDisposition
		0,							// dwFlagsAndAttributes
		NULL);						// hTemplateFile

	if (m_hPipeWrite == INVALID_HANDLE_VALUE) {
		DWORD l_dwLastError = GetLastError ();
		(void) _ftprintf (stderr, _T("CreateFile(%s) failed with %ld.\n"), GetName (), l_dwLastError);
		throw l_dwLastError;
	}

	m_hFinishedReadingEvent = CreateEvent (NULL, TRUE, FALSE, NULL);

	if (m_hFinishedReadingEvent == NULL) {
		DWORD l_dwLastError = GetLastError ();
		(void) _ftprintf (stderr, _T("CreateEvent(%s) failed with %ld.\n"), GetName (), l_dwLastError);
		throw l_dwLastError;
	}
}


//*****************************************************************************
//* Function Name: AccumulateText
//*   Description: 
//*****************************************************************************
void CRedirectionTarget::AccumulateText (LPCTSTR p_lpszText)
{
	m_strAccumulatedText += p_lpszText;
}


//*****************************************************************************
//* Function Name: StartAsyncRead
//*   Description: 
//*****************************************************************************
DWORD CRedirectionTarget::StartAsyncRead (void)
{
	DWORD l_dwResult = ERROR_SUCCESS;

	CMyOverlapped* l_pMyOverlapped = new CMyOverlapped (*this);

	if (ReadFileEx (
		GetPipeReadingHandle (),
		l_pMyOverlapped->GetBufferAddress (),
		l_pMyOverlapped->GetBufferSize (),
		l_pMyOverlapped,
		&CRedirectionTarget::StaticReadCompletionRoutine))
	{
		//(void) _tprintf (_T("Async read started successfully (%s).\n"), GetName ());
	}
	else {
		DWORD l_dwLastError = GetLastError ();

		if (l_dwLastError != ERROR_BROKEN_PIPE) {
			(void) _ftprintf (stderr, _T("ReadFileEx(%s) failed with %ld.\n"), GetName (), l_dwLastError);
		}

		delete l_pMyOverlapped;
		l_pMyOverlapped = NULL;

		l_dwResult = l_dwLastError;
	}

	return l_dwResult;
}


//*****************************************************************************
//* Function Name: StaticReadCompletionRoutine
//*   Description: 
//*****************************************************************************
void CRedirectionTarget::StaticReadCompletionRoutine (
	DWORD			p_dwErrorCode,
	DWORD			p_dwNumberOfBytesTransferred,
	LPOVERLAPPED	p_pOverlapped)
{
	CMyOverlapped* l_pMyOverlapped = static_cast<CMyOverlapped*>(p_pOverlapped);

	l_pMyOverlapped->GetThis ().InstanceReadCompletionRoutine (
		p_dwErrorCode,
		p_dwNumberOfBytesTransferred,
		l_pMyOverlapped);
}


//*****************************************************************************
//* Function Name: InstanceReadCompletionRoutine
//*   Description: 
//*****************************************************************************
void CRedirectionTarget::InstanceReadCompletionRoutine (
	DWORD			p_dwErrorCode,
	DWORD			p_dwNumberOfBytesTransferred,
	CMyOverlapped*	p_pMyOverlapped)
{
	if (p_dwErrorCode != ERROR_SUCCESS && p_dwErrorCode != ERROR_BROKEN_PIPE) {
		(void) _tprintf (_T("p_dwErrorCode = %ld (%s).\n"), p_dwErrorCode, GetName ());
	}

	//(void) _tprintf (_T("p_dwNumberOfBytesTransferred = %ld (%s).\n"), p_dwNumberOfBytesTransferred, GetName ());

	BOOL l_bSuccess = FALSE;

	if (p_dwErrorCode == ERROR_SUCCESS && p_dwNumberOfBytesTransferred > 0) {

		// Get a pointer to the ANSI buffer.
		LPCSTR l_lpszTextA = static_cast<LPCSTR>(p_pMyOverlapped->GetBufferAddress ());

		// Construct a smart BSTR from the ANSI buffer.
		_bstr_t l_sbstrText (l_lpszTextA);

		// Pass the smart BSTR to AccumulateText() which deals in TCHARs.
		AccumulateText (l_sbstrText);

		// Now try to initiate another async read.
		if (StartAsyncRead () == ERROR_SUCCESS) {
			l_bSuccess = TRUE;
		}
	}

	delete p_pMyOverlapped;
	p_pMyOverlapped = NULL;

	if (!l_bSuccess) {
		//(void) _tprintf (_T("Setting the finished reading event (%s)...\n"), GetName ());
		if (!SetEvent (GetFinishedReadingEvent ())) {
			DWORD l_dwLastError = GetLastError ();
			(void) _ftprintf (stderr, _T("SetEvent(%s) failed with %ld.\n"), GetName (), l_dwLastError);
		}
	}
}
