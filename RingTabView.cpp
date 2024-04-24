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

#include "Can2.h"
#include "RingDoc.h"
#include "RingTabView.h"
#include "RingFrm.h"
#include "RingPcvView.h"
#include "RingPcoView.h"
#include "RingDifView.h"
#include "RingBar.h"

// CRingTabView

IMPLEMENT_DYNCREATE(CRingTabView, CTabView)

CRingTabView::CRingTabView()
{
	m_pPcvView = nullptr;
	m_pPcoView = nullptr;
	m_pStatView = nullptr;
	m_pDifView = nullptr;
}

CRingTabView::~CRingTabView()
{
}

CRingDoc* CRingTabView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CRingDoc)));
	return (CRingDoc*)m_pDocument;
}

CRingBar* CRingTabView::getRingBar() const
{
	CRingFrame* pFrame = (CRingFrame*)GetParentFrame();
	return pFrame->m_pBar;
}

BEGIN_MESSAGE_MAP(CRingTabView, CTabView)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CRingTabView diagnostics

#ifdef _DEBUG
void CRingTabView::AssertValid() const
{
	CTabView::AssertValid();
}

#ifndef _WIN32_WCE
void CRingTabView::Dump(CDumpContext& dc) const
{
	CTabView::Dump(dc);
}
#endif
#endif //_DEBUG


// CRingTabView message handlers


int CRingTabView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTabView::OnCreate(lpCreateStruct) == -1)
		return -1;

	AddView(RUNTIME_CLASS(CRingPcvView), "PCV", 1);
	AddView(RUNTIME_CLASS(CRingPcoView), "PCO", 2);
	AddView(RUNTIME_CLASS(CRingDifView), "DIF", 3);
	//AddView(RUNTIME_CLASS(CRingStatView), "STAT", 4);

	m_pPcvView = (CRingPcvView*)m_wndTabs.GetTabWnd(0);
	m_pPcoView = (CRingPcoView*)m_wndTabs.GetTabWnd(1);
	m_pDifView = (CRingDifView*)m_wndTabs.GetTabWnd(2);
	//m_pStatView = (CRingStatView*)m_wndTabs.GetTabWnd(3);

	return 0;
}


BOOL CRingTabView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return CTabView::OnEraseBkgnd(pDC);
}


void CRingTabView::OnInitialUpdate()
{
	CTabView::OnInitialUpdate();

	getRingBar()->onCreateView(this); // m_pView = this;

	CRingDoc* pDoc = GetDocument();
	pDoc->SetTitle(pDoc->m_rn->getShortName().c_str());
}
