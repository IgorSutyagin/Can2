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

#include "Can2Doc.h"
#include "AntexFile.h"
#include "AntexAntenna.h"
#include "AntexSourceView.h"

CAntexSourceView::CAntexSourceView() noexcept
{
}

CAntexSourceView::~CAntexSourceView()
{
}

CCan2Doc* CAntexSourceView::GetDocument() const
{
	return reinterpret_cast<CCan2Doc*>(m_pDocument);
}

IMPLEMENT_DYNCREATE(CAntexSourceView, CEditView)

BEGIN_MESSAGE_MAP(CAntexSourceView, CEditView)
END_MESSAGE_MAP()

void CAntexSourceView::OnInitialUpdate()
{
	CEditView::OnInitialUpdate();

	CCan2Doc* pDoc = GetDocument();

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
	CFont* pFont = GetFont();
	if (pFont != NULL)
	{
		pFont->GetLogFont(&lf);
	}
	else
	{
		NONCLIENTMETRICS ncm;
		ncm.cbSize = sizeof(NONCLIENTMETRICS);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
		memmove(&lf, &ncm.lfMessageFont, sizeof(LOGFONT));
	}


	lf.lfWeight = FW_NORMAL;
	strncpy_s(lf.lfFaceName, "Courier", LF_FACESIZE);
	m_font.CreateFontIndirect(&lf);
	SetFont(&m_font);

	can2::AntexAntenna* pa = (can2::AntexAntenna*)pDoc->m_node;
	if (!pa->isAntenna())
		return;

	if (pa->m_parent == nullptr || !pa->m_parent->isAntexFile())
		return;

	std::string file = ((can2::AntexFile*)pa->m_parent)->m_sourceFile;

	CStdioFile f;
	if (!f.Open(file.c_str(), CFile::modeRead))
	{
		return;
	}

	CString strText;
	CString str;
	while (f.ReadString(str))
	{
		strText += str + "\r\n";
	}
	SetWindowText(strText);
}

BOOL CAntexSourceView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CEditView::PreCreateWindow(cs);
}
