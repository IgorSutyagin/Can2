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
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Can2.h"
#endif

#include "Can2Doc.h"
#include "Can2View.h"
#include "AntexPcvView.h"
#include "PcoView.h"
#include "PcoTableView.h"
#include "AntexSourceView.h"
#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCan2View

IMPLEMENT_DYNCREATE(CCan2View, CTabView)

BEGIN_MESSAGE_MAP(CCan2View, CTabView)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// CCan2View construction/destruction

CCan2View::CCan2View() noexcept
{
	m_pPcvView = nullptr;
	m_pPcoView = nullptr;
	m_pSrcView = nullptr;
	m_pPcoTableView = nullptr;
}

CCan2View::~CCan2View()
{
}


// CCan2View diagnostics

#ifdef _DEBUG
void CCan2View::AssertValid() const
{
	CTabView::AssertValid();
}

void CCan2View::Dump(CDumpContext& dc) const
{
	CTabView::Dump(dc);
}

CCan2Doc* CCan2View::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCan2Doc)));
	return (CCan2Doc*)m_pDocument;
}
#endif //_DEBUG

void CCan2View::onSignalChanged()
{
	m_pPcvView->regenAll();
	m_pPcoView->updateCurves();
}

// CCan2View message handlers

int CCan2View::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTabView::OnCreate(lpCreateStruct) == -1)
		return -1;

	AddView(RUNTIME_CLASS(CAntexPcvView), "PCV", 1);
	AddView(RUNTIME_CLASS(CPcoView), "PCO", 2);
	AddView(RUNTIME_CLASS(CPcoTableView), "PCO (table)", 3);
	AddView(RUNTIME_CLASS(CAntexSourceView), "SRC", 4);

	m_pPcvView = (CAntexPcvView*)m_wndTabs.GetTabWnd(0);
	m_pPcoView = (CPcoView*)m_wndTabs.GetTabWnd(1);
	m_pPcoTableView = (CPcoTableView*)m_wndTabs.GetTabWnd(2);
	m_pSrcView = (CAntexSourceView*)m_wndTabs.GetTabWnd(3);

	return 0;
}

BOOL CCan2View::OnEraseBkgnd(CDC* pDC)
{
	return CTabView::OnEraseBkgnd(pDC);
}

void CCan2View::OnInitialUpdate()
{
	CTabView::OnInitialUpdate();

	CCan2Doc* pDoc = GetDocument();
	pDoc->SetTitle(pDoc->m_node->getName().c_str());
}
