////////////////////////////////////////////////////////////////////////////////////////////////////
// 
//  The CAN2 software is distributed under the following BSD 2-clause license and 
//  additional exclusive clauses. Users are permitted to develop, produce or sell their 
//  own non-commercial or commercial products utilizing, linking or including CAN2 as 
//  long as they comply with the license.BSD 2 - Clause License
// 
//  Copyright(c) 2024, TOPCON, All rights reserved.
// 
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met :
// 
//  1. Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
// 
//  2. Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and /or other materials provided with the distribution.
// 
//  3. The software package includes some companion executive binaries or shared 
//  libraries necessary to execute APs on Windows. These licenses succeed to the 
//  original ones of these software.
// 
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// 	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// 	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// 	OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// 	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include "pch.h"

#include "Tools.h"

namespace can2
{
	const char* whiteSpace = " \t\n\r\f\v";


	CSize calcDialogSize(UINT nResourceId, HINSTANCE hInstance)
	{
		CSize size;
		HRSRC hRsrc = ::FindResource(hInstance, MAKEINTRESOURCE(nResourceId), RT_DIALOG);
		ASSERT(hRsrc != NULL);
		if (hRsrc == NULL)
			return CSize(0, 0);

		HGLOBAL hTemplate = ::LoadResource(hInstance, hRsrc);
		ASSERT(hTemplate != NULL);
		if (hTemplate == nullptr)
			return CSize(0, 0);

		DLGTEMPLATE* pTemplate = (DLGTEMPLATE*)::LockResource(hTemplate);
		CDialogTemplate dlgtemplate(pTemplate);
		dlgtemplate.GetSizeInPixels(&size);
		::UnlockResource(hTemplate);

		size.cy += GetSystemMetrics(SM_CYCAPTION);

		return size;
	}

	int CTimeEx::getYearGmt() const
	{
		struct tm ttm;
		struct tm* ptm;

		ptm = GetGmtTm(&ttm);
		return ptm ? (ptm->tm_year) + 1900 : 0;
	}

	int CTimeEx::getMonthGmt() const
	{
		struct tm ttm;
		struct tm* ptm;

		ptm = GetGmtTm(&ttm);
		return ptm ? ptm->tm_mon + 1 : 0;
	}

	int CTimeEx::getDayGmt() const
	{
		struct tm ttm;
		struct tm* ptm;

		ptm = GetGmtTm(&ttm);
		return ptm ? ptm->tm_mday : 0;
	}

	int CTimeEx::getHourGmt() const
	{
		struct tm ttm;
		struct tm* ptm;

		ptm = GetGmtTm(&ttm);
		return ptm ? ptm->tm_hour : -1;
	}

}