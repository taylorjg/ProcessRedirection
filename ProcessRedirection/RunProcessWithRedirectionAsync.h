#ifndef __RunProcessWithRedirectionAsync_h__
#define __RunProcessWithRedirectionAsync_h__

#include "RedirectionTarget.h"

class CMyOverlapped;

class CRunProcessWithRedirectionAsync
{
public:
	DWORD RunProcessAndWait (const CString& p_strExe);
	DWORD RunProcessAndWait (const CString& p_strExe, const CString& p_strArgs);

	const CString& GetRedirectedStdOutText (void) const { return m_RedirectedStdOut.GetAccumulatedText (); }
	const CString& GetRedirectedStdErrText (void) const { return m_RedirectedStdErr.GetAccumulatedText (); }

private:
	CRedirectionTarget m_RedirectedStdOut;
	CRedirectionTarget m_RedirectedStdErr;
};

#endif
