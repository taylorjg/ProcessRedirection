#include "stdafx.h"
#include "Wait.h"

#include <stdarg.h>
#include <crtdbg.h>

//*****************************************************************************
//* Function Name: Wait
//*   Description: Wraps the Windows API function, WaitForMultipleObjectsEx().
//*                Possible outcomes :-
//*
//*                    returns TRUE
//*                      if the wait condition is satisfied within the timeout
//*
//*                    returns FALSE
//*                      if the wait condition is not satisifed within the timeout
//*                      if WaitForMultipleObjectsEx() returns an unknown value
//*
//*                    throws an exception of type DWORD
//*                       if WaitForMultipleObjectsEx() returns WAIT_FAILED
//*                       then the DWORD will be the value of GetLastError()
//*****************************************************************************
BOOL Wait (
	DWORD	p_dwTimeout,
	BOOL	p_bWaitAll,
	BOOL	p_bAlertable,
	INT		p_iNumHandles,
	...)
{
	BOOL l_bResult = FALSE;

	_ASSERTE (p_iNumHandles >= 1);
	_ASSERTE (p_iNumHandles <= MAXIMUM_WAIT_OBJECTS);

	LPHANDLE l_haHandles = new HANDLE[p_iNumHandles];

	va_list arg_ptr;
	va_start (arg_ptr, p_iNumHandles);

	for (INT i = 0; i < p_iNumHandles; i++) {
		HANDLE l_h = va_arg (arg_ptr, HANDLE);
		_ASSERTE (l_h != NULL);
		_ASSERTE (l_h != INVALID_HANDLE_VALUE);
		l_haHandles[i] = l_h;
	}

	va_end (arg_ptr);

	for (;;) {
		DWORD l_dwWaitResult = WaitForMultipleObjectsEx (
			p_iNumHandles,		// nCount
			l_haHandles,		// lpHandles
			p_bWaitAll,			// bWaitAll
			p_dwTimeout,		// dwMilliseconds
			p_bAlertable);		// bAlertable

		if (l_dwWaitResult == WAIT_FAILED) {
			DWORD l_dwLastError = GetLastError ();
			(void) _ftprintf (stderr, _T("WaitForMultipleObjectsEx() failed with %ld.\n"), l_dwLastError);
			delete[] l_haHandles;
			l_haHandles = NULL;
			throw l_dwLastError;
		}

		if (l_dwWaitResult >= WAIT_OBJECT_0 && l_dwWaitResult < (WAIT_OBJECT_0 + p_iNumHandles)) {
			l_bResult = TRUE;
			break;
		}

		if (l_dwWaitResult == WAIT_IO_COMPLETION) {
			_ASSERTE (p_bAlertable == TRUE);
			continue;
		}

		if (l_dwWaitResult == WAIT_TIMEOUT) {
			_ASSERTE (p_dwTimeout != INFINITE);
			break;
		}

		(void) _ftprintf (stderr, _T("WaitForMultipleObjectsEx() returned %ld.\n"), l_dwWaitResult);
		break;
	}

	delete[] l_haHandles;
	l_haHandles = NULL;

	return l_bResult;
}
