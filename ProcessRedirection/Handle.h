#ifndef __Handle_h__
#define __Handle_h__

class CHandle
{
public:
	CHandle (void) : m_h (NULL) {}
	CHandle (HANDLE p_h) : m_h (p_h) {}

	~CHandle (void)
	{
		Close ();
	}

	CHandle& operator= (HANDLE p_h)
	{
		Close ();
		m_h = p_h;

		return *this;
	}

    LPHANDLE operator& (void)
    {
        Close ();
        return &m_h;
    }

	void Close (void) {
		if (m_h != NULL && m_h != INVALID_HANDLE_VALUE) {
			if (!CloseHandle (m_h)) {
				DWORD l_dwLastError = GetLastError ();
				(void) _ftprintf (stderr, _T("CloseHandle() failed with %ld.\n"), l_dwLastError);
			}
			m_h = NULL;
		}
	}

	operator HANDLE (void) const { return m_h; }

private:

	// Disallow copy constructor and assignment operator.
	CHandle (const CHandle& rhs);
	CHandle& operator= (const CHandle& rhs);

	HANDLE m_h;
};

#endif
