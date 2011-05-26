#ifndef __RedirectionData_h__
#define __RedirectionData_h__

#include "Handle.h"

class CMyOverlapped;

class CRedirectionTarget
{
public:
	void Initialise (const CString& p_strName);
	const CString& GetName (void) const { return m_strName; }
	const CString& GetAccumulatedText (void) const { return m_strAccumulatedText; }
	const CHandle& GetPipeReadingHandle (void) const { return m_hPipeRead; }
	const CHandle& GetPipeWritingHandle (void) const { return m_hPipeWrite; }
	const CHandle& GetFinishedReadingEvent (void) const { return m_hFinishedReadingEvent; }

	void ClearAccumulatedText (void) { m_strAccumulatedText.Empty (); }
	void CloseReadingEnd (void) { m_hPipeRead.Close (); }
	void CloseWritingEnd (void) { m_hPipeWrite.Close (); }
	void Reset (void)
	{
		ClearAccumulatedText ();
		CloseReadingEnd ();
		CloseWritingEnd ();
		m_hFinishedReadingEvent.Close ();
	}
	DWORD BeginReading (void) { return StartAsyncRead (); }

private:

	CString m_strName;
	void AccumulateText (LPCTSTR p_lpszText);

	DWORD StartAsyncRead (void);

	static void CALLBACK StaticReadCompletionRoutine (
		DWORD			p_dwErrorCode,
		DWORD			p_dwNumberOfBytesTransferred,
		LPOVERLAPPED	p_pOverlapped);

	void InstanceReadCompletionRoutine (
		DWORD			p_dwErrorCode,
		DWORD			p_dwNumberOfBytesTransferred,
		CMyOverlapped*	p_pMyOverlapped);

	// Text read from the child process is accumulated in this field.
	CString m_strAccumulatedText;

	// This handle is used by the parent process to read text from the child process.
	CHandle m_hPipeRead;

	// This handle is inherited by the child process.
	CHandle m_hPipeWrite;

	// We set this event when :-
	//	- an error occurs reading from the pipe
	// or
	//	- we read 0 bytes
	CHandle	m_hFinishedReadingEvent;
};

#endif
