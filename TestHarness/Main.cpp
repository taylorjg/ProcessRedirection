#include <afx.h>

#include "RunProcessWithRedirection.h"
#include "RunProcessWithRedirectionAsync.h"

static void RunProcessWithRedirection_GivenSeparateExeAndArgs_WritesToStdOut (void);
static void RunProcessWithRedirection_GivenExeWithNoArgs_WritesToStdOut (void);
static void RunProcessWithRedirection_GivenExeThatsWritesLotsToStdOut_WritesLotsToStdOut (void);
static void RunProcessWithRedirection_GivenBogusArgs_WritesToStdErr (void);
static void RunProcessWithRedirection_GivenBogusExe_Fails (void);

static void RunProcessWithRedirectionAsync_GivenSeparateExeAndArgs_WritesToStdOut (void);
static void RunProcessWithRedirectionAsync_GivenExeWithNoArgs_WritesToStdOut (void);
static void RunProcessWithRedirectionAsync_GivenExeThatsWritesLotsToStdOut_WritesLotsToStdOut (void);
static void RunProcessWithRedirectionAsync_GivenBogusArgs_WritesToStdErr (void);
static void RunProcessWithRedirectionAsync_GivenBogusExe_Fails (void);

static CString CombineWithSystemDirectory (LPCTSTR p_lpszFileName);

//*****************************************************************************
//* Function Name: main
//*   Description: 
//*****************************************************************************
int main ()
{
	RunProcessWithRedirection_GivenSeparateExeAndArgs_WritesToStdOut ();
	RunProcessWithRedirection_GivenExeWithNoArgs_WritesToStdOut ();
	RunProcessWithRedirection_GivenExeThatsWritesLotsToStdOut_WritesLotsToStdOut ();
	RunProcessWithRedirection_GivenBogusArgs_WritesToStdErr ();
	RunProcessWithRedirection_GivenBogusExe_Fails ();

	RunProcessWithRedirectionAsync_GivenSeparateExeAndArgs_WritesToStdOut ();
	RunProcessWithRedirectionAsync_GivenExeWithNoArgs_WritesToStdOut ();
	RunProcessWithRedirectionAsync_GivenExeThatsWritesLotsToStdOut_WritesLotsToStdOut ();
	RunProcessWithRedirectionAsync_GivenBogusArgs_WritesToStdErr ();
	RunProcessWithRedirectionAsync_GivenBogusExe_Fails ();

	return 0;
}


//*****************************************************************************
//* Function Name: RunProcessWithRedirection_GivenSeparateExeAndArgs_WritesToStdOut
//*   Description: 
//*****************************************************************************
static void RunProcessWithRedirection_GivenSeparateExeAndArgs_WritesToStdOut (void)
{
	(void) _tprintf (_T("RunProcessWithRedirection_GivenSeparateExeAndArgs_WritesToStdOut\n"));

	CRunProcessWithRedirection l_SUT;
	DWORD l_dwResult = l_SUT.RunProcessAndWait (CombineWithSystemDirectory (_T("cmd.exe")), CString (_T("/c dir c:\\")));

	if (l_dwResult == ERROR_SUCCESS) {

		CString l_strStdOut = l_SUT.GetRedirectedStdOut ();
		CString l_strStdErr = l_SUT.GetRedirectedStdErr ();

		(void) _tprintf(_T("stdout :-\n"));
		(void) _tprintf(l_strStdOut);
		(void) _tprintf(_T("\n"));

		(void) _tprintf(_T("stderr :-\n"));
		(void) _tprintf(l_strStdErr);
		(void) _tprintf(_T("\n"));
	}
	else {
		(void) _tprintf (_T("RunProcessAndWait() failed with %ld.\n"), l_dwResult);
	}
}


//*****************************************************************************
//* Function Name: RunProcessWithRedirection_GivenExeWithNoArgs_WritesToStdOut
//*   Description: 
//*****************************************************************************
static void RunProcessWithRedirection_GivenExeWithNoArgs_WritesToStdOut (void)
{
	(void) _tprintf (_T("RunProcessWithRedirection_GivenExeWithNoArgs_WritesToStdOut\n"));

	CRunProcessWithRedirection l_SUT;
	DWORD l_dwResult = l_SUT.RunProcessAndWait (CombineWithSystemDirectory (_T("ipconfig.exe")));

	if (l_dwResult == ERROR_SUCCESS) {

		CString l_strStdOut = l_SUT.GetRedirectedStdOut ();
		CString l_strStdErr = l_SUT.GetRedirectedStdErr ();

		(void) _tprintf(_T("stdout :-\n"));
		(void) _tprintf(l_strStdOut);
		(void) _tprintf(_T("\n"));

		(void) _tprintf(_T("stderr :-\n"));
		(void) _tprintf(l_strStdErr);
		(void) _tprintf(_T("\n"));
	}
	else {
		(void) _tprintf (_T("RunProcessAndWait() failed with %ld.\n"), l_dwResult);
	}
}


//*****************************************************************************
//* Function Name: RunProcessWithRedirection_GivenExeThatsWritesLotsToStdOut_WritesLotsToStdOut
//*   Description: 
//*****************************************************************************
static void RunProcessWithRedirection_GivenExeThatsWritesLotsToStdOut_WritesLotsToStdOut (void)
{
	(void) _tprintf (_T("RunProcessWithRedirection_GivenExeThatsWritesLotsToStdOut_WritesLotsToStdOut\n"));

	CRunProcessWithRedirection l_SUT;
	DWORD l_dwResult = l_SUT.RunProcessAndWait (CombineWithSystemDirectory (_T("cmd.exe")), _T("/c dir /b %temp%"));

	if (l_dwResult == ERROR_SUCCESS) {

		CString l_strStdOut = l_SUT.GetRedirectedStdOut ();
		CString l_strStdErr = l_SUT.GetRedirectedStdErr ();

		(void) _tprintf(_T("stdout :-\n"));
		(void) _tprintf(l_strStdOut);
		(void) _tprintf(_T("\n"));

		(void) _tprintf(_T("stderr :-\n"));
		(void) _tprintf(l_strStdErr);
		(void) _tprintf(_T("\n"));
	}
	else {
		(void) _tprintf (_T("RunProcessAndWait() failed with %ld.\n"), l_dwResult);
	}
}


//*****************************************************************************
//* Function Name: RunProcessWithRedirection_GivenBogusArgs_WritesToStdErr
//*   Description: 
//*****************************************************************************
static void RunProcessWithRedirection_GivenBogusArgs_WritesToStdErr (void)
{
	(void) _tprintf (_T("RunProcessWithRedirection_GivenBogusArgs_WritesToStdErr\n"));

	CRunProcessWithRedirection l_SUT;
	DWORD l_dwResult = l_SUT.RunProcessAndWait (CombineWithSystemDirectory (_T("cmd.exe")), _T("/c dir c:\\this\\is\\\bogus\directory"));

	if (l_dwResult == ERROR_SUCCESS) {

		CString l_strStdOut = l_SUT.GetRedirectedStdOut ();
		CString l_strStdErr = l_SUT.GetRedirectedStdErr ();

		(void) _tprintf(_T("stdout :-\n"));
		(void) _tprintf(l_strStdOut);
		(void) _tprintf(_T("\n"));

		(void) _tprintf(_T("stderr :-\n"));
		(void) _tprintf(l_strStdErr);
		(void) _tprintf(_T("\n"));
	}
	else {
		(void) _tprintf (_T("RunProcessAndWait() failed with %ld.\n"), l_dwResult);
	}
}


//*****************************************************************************
//* Function Name: RunProcessWithRedirection_GivenBogusExe_Fails
//*   Description: 
//*****************************************************************************
static void RunProcessWithRedirection_GivenBogusExe_Fails (void)
{
	(void) _tprintf (_T("RunProcessWithRedirection_GivenBogusExe_Fails\n"));

	CRunProcessWithRedirection l_SUT;
	DWORD l_dwResult = l_SUT.RunProcessAndWait (_T("ThisIsABogusProgram.exe"));

	if (l_dwResult == ERROR_SUCCESS) {

		CString l_strStdOut = l_SUT.GetRedirectedStdOut ();
		CString l_strStdErr = l_SUT.GetRedirectedStdErr ();

		(void) _tprintf(_T("stdout :-\n"));
		(void) _tprintf(l_strStdOut);
		(void) _tprintf(_T("\n"));

		(void) _tprintf(_T("stderr :-\n"));
		(void) _tprintf(l_strStdErr);
		(void) _tprintf(_T("\n"));
	}
	else {
		(void) _tprintf (_T("RunProcessAndWait() failed with %ld.\n"), l_dwResult);
	}
}


//*****************************************************************************
//* Function Name: RunProcessWithRedirectionAsync_GivenSeparateExeAndArgs_WritesToStdOut
//*   Description: 
//*****************************************************************************
static void RunProcessWithRedirectionAsync_GivenSeparateExeAndArgs_WritesToStdOut (void)
{
	(void) _tprintf (_T("RunProcessWithRedirectionAsync_GivenSeparateExeAndArgs_WritesToStdOut\n"));

	CRunProcessWithRedirectionAsync l_SUT;
	DWORD l_dwResult = l_SUT.RunProcessAndWait (CombineWithSystemDirectory (_T("cmd.exe")), CString (_T("/c dir c:\\")));

	if (l_dwResult == ERROR_SUCCESS) {

		CString l_strStdOut = l_SUT.GetRedirectedStdOut ();
		CString l_strStdErr = l_SUT.GetRedirectedStdErr ();

		(void) _tprintf(_T("stdout :-\n"));
		(void) _tprintf(l_strStdOut);
		(void) _tprintf(_T("\n"));

		(void) _tprintf(_T("stderr :-\n"));
		(void) _tprintf(l_strStdErr);
		(void) _tprintf(_T("\n"));
	}
	else {
		(void) _tprintf (_T("RunProcessAndWait() failed with %ld.\n"), l_dwResult);
	}
}


//*****************************************************************************
//* Function Name: RunProcessWithRedirectionAsync_GivenExeWithNoArgs_WritesToStdOut
//*   Description: 
//*****************************************************************************
static void RunProcessWithRedirectionAsync_GivenExeWithNoArgs_WritesToStdOut (void)
{
	(void) _tprintf (_T("RunProcessWithRedirectionAsync_GivenExeWithNoArgs_WritesToStdOut\n"));

	CRunProcessWithRedirectionAsync l_SUT;
	DWORD l_dwResult = l_SUT.RunProcessAndWait (CombineWithSystemDirectory (_T("ipconfig.exe")));

	if (l_dwResult == ERROR_SUCCESS) {

		CString l_strStdOut = l_SUT.GetRedirectedStdOut ();
		CString l_strStdErr = l_SUT.GetRedirectedStdErr ();

		(void) _tprintf(_T("stdout :-\n"));
		(void) _tprintf(l_strStdOut);
		(void) _tprintf(_T("\n"));

		(void) _tprintf(_T("stderr :-\n"));
		(void) _tprintf(l_strStdErr);
		(void) _tprintf(_T("\n"));
	}
	else {
		(void) _tprintf (_T("RunProcessAndWait() failed with %ld.\n"), l_dwResult);
	}
}


//*****************************************************************************
//* Function Name: RunProcessWithRedirectionAsync_GivenExeThatsWritesLotsToStdOut_WritesLotsToStdOut
//*   Description: 
//*****************************************************************************
static void RunProcessWithRedirectionAsync_GivenExeThatsWritesLotsToStdOut_WritesLotsToStdOut (void)
{
	(void) _tprintf (_T("RunProcessWithRedirectionAsync_GivenExeThatsWritesLotsToStdOut_WritesLotsToStdOut\n"));

	CRunProcessWithRedirection l_SUT;
	DWORD l_dwResult = l_SUT.RunProcessAndWait (CombineWithSystemDirectory (_T("cmd.exe")), _T("/c dir /b %temp%"));

	if (l_dwResult == ERROR_SUCCESS) {

		CString l_strStdOut = l_SUT.GetRedirectedStdOut ();
		CString l_strStdErr = l_SUT.GetRedirectedStdErr ();

		(void) _tprintf(_T("stdout :-\n"));
		(void) _tprintf(l_strStdOut);
		(void) _tprintf(_T("\n"));

		(void) _tprintf(_T("stderr :-\n"));
		(void) _tprintf(l_strStdErr);
		(void) _tprintf(_T("\n"));
	}
	else {
		(void) _tprintf (_T("RunProcessAndWait() failed with %ld.\n"), l_dwResult);
	}
}


//*****************************************************************************
//* Function Name: RunProcessWithRedirectionAsync_GivenBogusArgs_WritesToStdErr
//*   Description: 
//*****************************************************************************
static void RunProcessWithRedirectionAsync_GivenBogusArgs_WritesToStdErr (void)
{
	(void) _tprintf (_T("RunProcessWithRedirectionAsync_GivenBogusArgs_WritesToStdErr\n"));

	CRunProcessWithRedirectionAsync l_SUT;
	DWORD l_dwResult = l_SUT.RunProcessAndWait (CombineWithSystemDirectory (_T("cmd.exe")), _T("/c dir c:\\this\\is\\\bogus\directory"));

	if (l_dwResult == ERROR_SUCCESS) {

		CString l_strStdOut = l_SUT.GetRedirectedStdOut ();
		CString l_strStdErr = l_SUT.GetRedirectedStdErr ();

		(void) _tprintf(_T("stdout :-\n"));
		(void) _tprintf(l_strStdOut);
		(void) _tprintf(_T("\n"));

		(void) _tprintf(_T("stderr :-\n"));
		(void) _tprintf(l_strStdErr);
		(void) _tprintf(_T("\n"));
	}
	else {
		(void) _tprintf (_T("RunProcessAndWait() failed with %ld.\n"), l_dwResult);
	}
}


//*****************************************************************************
//* Function Name: RunProcessWithRedirectionAsync_GivenBogusExe_Fails
//*   Description: 
//*****************************************************************************
static void RunProcessWithRedirectionAsync_GivenBogusExe_Fails (void)
{
	(void) _tprintf (_T("RunProcessWithRedirectionAsync_GivenBogusExe_Fails\n"));

	CRunProcessWithRedirectionAsync l_SUT;
	DWORD l_dwResult = l_SUT.RunProcessAndWait (_T("ThisIsABogusProgram.exe"));

	if (l_dwResult == ERROR_SUCCESS) {

		CString l_strStdOut = l_SUT.GetRedirectedStdOut ();
		CString l_strStdErr = l_SUT.GetRedirectedStdErr ();

		(void) _tprintf(_T("stdout :-\n"));
		(void) _tprintf(l_strStdOut);
		(void) _tprintf(_T("\n"));

		(void) _tprintf(_T("stderr :-\n"));
		(void) _tprintf(l_strStdErr);
		(void) _tprintf(_T("\n"));
	}
	else {
		(void) _tprintf (_T("RunProcessAndWait() failed with %ld.\n"), l_dwResult);
	}
}


//*****************************************************************************
//* Function Name: CombineWithSystemDirectory
//*   Description: 
//*****************************************************************************
static CString CombineWithSystemDirectory (LPCTSTR p_lpszFileName)
{
	TCHAR l_szSystemDirectory[_MAX_PATH + 1] = {0};
	GetSystemDirectory (l_szSystemDirectory, _MAX_PATH);
	CString l_strSystemDirectory (l_szSystemDirectory);
	return l_strSystemDirectory + _T("\\") + p_lpszFileName;
}
