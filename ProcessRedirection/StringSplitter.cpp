#include "stdafx.h"
#include "StringSplitter.h"

//*****************************************************************************
//* Function Name: SplitStringIntoLines
//*   Description: Split a string into an array of separate lines.
//*****************************************************************************
void CStringSplitter::SplitStringIntoLines (const CString& p_strString, CStringArray& p_rstraLines)
{
	const TCHAR LINE_FEED		= _T('\n');
	const TCHAR CARRIAGE_RETURN	= _T('\r');

	p_rstraLines.RemoveAll ();
	int l_iLineStart = 0;

	for (;;) {

		int l_iLineFeed = p_strString.Find (LINE_FEED, l_iLineStart);

		if (l_iLineFeed >= 0) {

			int l_cchLine = l_iLineFeed - l_iLineStart;

			// Do we have a carriage return before the line feed ?
			if (l_iLineFeed > 0) {
				if (p_strString[l_iLineFeed - 1] == CARRIAGE_RETURN) {
					// Yes - so don't include it.
					l_cchLine--;
				}
			}

			CString l_strLine = p_strString.Mid (l_iLineStart, l_cchLine);
			p_rstraLines.Add (l_strLine);
			l_iLineStart = l_iLineFeed + 1;
		}
		else {
			CString l_strLine = p_strString.Mid (l_iLineStart);
			p_rstraLines.Add (l_strLine);
			break;
		}
	}
}
