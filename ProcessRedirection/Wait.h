#ifndef __Wait_h__
#define __Wait_h__

extern BOOL Wait (
	DWORD	p_dwTimeout,
	BOOL	p_bWaitAll,
	BOOL	p_bAlertable,
	INT		p_iNumHandles,
	...);

#endif
