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
#include "Can2Doc.h"
#include "AntexPcvView.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CAntexPcvCtrl implementation

///////////////////////////////
// Overrides
can2::OglSurface::Grid CAntexPcvCtrl::getGrid() const
{
	can2::OglSurface::Grid grid;
	CCan2Doc* pd = m_pView->GetDocument();
	if (pd->m_node == nullptr)
		return grid;

	return can2::OglSurface::Grid(0, 360, 181, 0, 90, 361);

}

can2::Node* CAntexPcvCtrl::getNode() const
{
	CCan2Doc* pd = m_pView->GetDocument();
	return pd->m_node;
}

can2::Gnss::Signal CAntexPcvCtrl::getSignal() const
{
	CCan2Doc* pd = m_pView->GetDocument();
	return pd->m_signal;
}

double CAntexPcvCtrl::getData(const can2::Node* node, can2::Gnss::Signal es, double x, double y) const
{
	return node->getPcv(es, y, x) * 1000;
}

void CAntexPcvCtrl::onViewChanged()
{
	can2::Plot3d::onViewChanged();
	m_pView->onViewChanged();
}

// End of CAntexPcvCtrl implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CAntexPcvView implementation
CCan2Doc* CAntexPcvView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCan2Doc)));
	return (CCan2Doc*)m_pDocument;
}

void CAntexPcvView::regenAll()
{
	m_wndPlot.updateData();
}

void CAntexPcvView::onViewChanged()
{
	m_viewParams = m_wndPlot.getViewParams();
	UpdateData(FALSE);
}

IMPLEMENT_DYNCREATE(CAntexPcvView, CFormView)

CAntexPcvView::CAntexPcvView()
	: CFormView(CAntexPcvView::IDD), m_bNeedInit(true), m_wndPlot(this)
{

}

CAntexPcvView::~CAntexPcvView()
{
}

void CAntexPcvView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CUSTOM_PLOT, m_wndPlot);
	DDX_Control(pDX, IDC_COMBO_SIGNAL, m_cmbSignal);
	DDX_Control(pDX, IDC_COMBO_VIEW_AT, m_cmbViewAt);
	DDX_Text(pDX, IDC_EDIT_MIN, m_viewParams.zMin);
	DDX_Text(pDX, IDC_EDIT_MAX, m_viewParams.zMax);
	DDX_Text(pDX, IDC_EDIT_VALUE_STEP, m_viewParams.zStep);
	DDX_Text(pDX, IDC_EDIT_ISO_STEP, m_viewParams.isoStep);
	DDX_Text(pDX, IDC_EDIT_COLOR_STEP, m_viewParams.colorStep);

	BOOL bPolar = m_viewParams.coords == can2::Plot3dCamera::ecPolar ? TRUE : FALSE;
	DDX_Check(pDX, IDC_CHECK_POLAR, bPolar);
	if (pDX->m_bSaveAndValidate)
		m_viewParams.coords = bPolar ? can2::Plot3dCamera::ecPolar : can2::Plot3dCamera::ecDecart;

	BOOL bGrid0 = m_viewParams.bGrid0 ? TRUE : FALSE;
	DDX_Check(pDX, IDC_CHECK_0GRID, bGrid0);
	if (pDX->m_bSaveAndValidate)
		m_viewParams.bGrid0 = bGrid0 ? true : false;

	BOOL bRainbow = m_viewParams.bRainbow ? TRUE : FALSE;
	DDX_Check(pDX, IDC_CHECK_RAINBOW, bRainbow);
	if (pDX->m_bSaveAndValidate)
		m_viewParams.bRainbow = bRainbow ? true : false;

	if (pDX->m_bSaveAndValidate)
	{
		int sel = m_cmbSignal.GetCurSel();
		if (sel >= 0)
		{
			can2::Gnss::Signal es = (can2::Gnss::Signal)m_cmbSignal.GetItemData(sel);
			CCan2Doc* pDoc = GetDocument();
			pDoc->m_signal = es;
		}

		sel = m_cmbViewAt.GetCurSel();
		if (sel >= 0)
		{
			m_viewParams.viewAt = (can2::Plot3dCamera::ViewAt)m_cmbViewAt.GetItemData(sel);
		}
	}
	else
	{
		CCan2Doc* pDoc = GetDocument();
		for (int i = 0; i < m_cmbSignal.GetCount(); i++)
		{
			DWORD dw = m_cmbSignal.GetItemData(i);
			if (pDoc->m_signal == dw)
			{
				m_cmbSignal.SetCurSel(i);
				break;
			}
		}

		for (int i = 0; i < m_cmbSignal.GetCount(); i++)
		{
			if ((can2::Plot3dCamera::ViewAt)m_cmbViewAt.GetItemData(i) == m_viewParams.viewAt)
			{
				m_cmbViewAt.SetCurSel(i);
				break;
			}
		}
	}
}

BEGIN_MESSAGE_MAP(CAntexPcvView, CFormView)
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_COMBO_SIGNAL, &OnSelchangeComboSignal)
	ON_CBN_SELCHANGE(IDC_COMBO_VIEW_AT, &OnSelchangeComboViewAt)
	ON_EN_CHANGE(IDC_EDIT_ISO_STEP, &OnEnChangeEditIso)
	ON_EN_CHANGE(IDC_EDIT_MIN, OnChangeEditZMinMax)
	ON_EN_CHANGE(IDC_EDIT_MAX, OnChangeEditZMinMax)
	ON_EN_CHANGE(IDC_EDIT_VALUE_STEP, OnChangeEditZMinMax)
	ON_EN_CHANGE(IDC_EDIT_COLOR_STEP, OnEnChangeEditIso)
	ON_BN_CLICKED(IDC_CHECK_POLAR, OnPolar)
	ON_BN_CLICKED(IDC_CHECK_0GRID, OnGrid0)
	ON_BN_CLICKED(IDC_CHECK_RAINBOW, OnRainbow)
END_MESSAGE_MAP()


// CAntexPcvView message handlers


void CAntexPcvView::OnSize(UINT nType, int cx, int cy)
{
	CRect r1;
	CFormView::OnSize(nType, cx, cy);
	if (m_bNeedInit)
	{
		if (cx > 0 && cy > 0)
		{
			m_rCrt = CRect(0, 0, cx, cy);
			m_bNeedInit = false;
		}
		return;
	}

	int dx = cx - m_rCrt.Width();
	int dy = cy - m_rCrt.Height();
	GetClientRect(&m_rCrt);

	HDWP hDWP = ::BeginDeferWindowPos(5);

	for (CWnd* pChild = GetWindow(GW_CHILD);
		pChild != NULL;
		pChild = pChild->GetWindow(GW_HWNDNEXT))
	{
		int nId = pChild->GetDlgCtrlID();
		if (nId == IDC_CUSTOM_PLOT)
		{
			pChild->GetWindowRect(&r1);
			ScreenToClient(r1);
			r1.right = m_rCrt.right - 20; // += dx;
			r1.bottom = m_rCrt.bottom - 30; // += dy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, 0, 0,
				r1.Width(), r1.Height(),
				SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
		}

	}

	::EndDeferWindowPos(hDWP);

}

void CAntexPcvView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	GetParentFrame()->RecalcLayout();
	ResizeParentToFit(FALSE);
	ResizeParentToFit(TRUE);

	UpdateData(FALSE);

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
	GetFont()->GetLogFont(&lf);

	CCan2Doc* pDoc = GetDocument();
	can2::Node* node = pDoc->m_node;

	for (int s = can2::Gnss::G01; s < can2::Gnss::esigInvalid; s++)
	{
		if (!node->hasPcc(s))
			continue;
		int index = m_cmbSignal.AddString(can2::Gnss::getSignalName(s));
		m_cmbSignal.SetItemData(index, s);
		if (pDoc->m_signal == s)
			m_cmbSignal.SetCurSel(index);
	}

	int index = m_cmbViewAt.AddString("Default");
	m_cmbViewAt.SetItemData(index, can2::Plot3dCamera::evaDef);
	if (m_viewParams.viewAt == can2::Plot3dCamera::evaDef)
		m_cmbViewAt.SetCurSel(index);
	index = m_cmbViewAt.AddString("Front");
	m_cmbViewAt.SetItemData(index, can2::Plot3dCamera::evaFront);
	if (m_viewParams.viewAt == can2::Plot3dCamera::evaFront)
		m_cmbViewAt.SetCurSel(index);
	index = m_cmbViewAt.AddString("Top");
	m_cmbViewAt.SetItemData(index, can2::Plot3dCamera::evaTop);
	if (m_viewParams.viewAt == can2::Plot3dCamera::evaTop)
		m_cmbViewAt.SetCurSel(index);
	index = m_cmbViewAt.AddString("Custom");
	m_cmbViewAt.SetItemData(index, can2::Plot3dCamera::evaCustom);
	if (m_viewParams.viewAt == can2::Plot3dCamera::evaCustom)
		m_cmbViewAt.SetCurSel(index);

	UpdateData(FALSE);

	m_wndPlot.onInitialUpdate();

	m_wndPlot.updateData();
}

void CAntexPcvView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1) // Siganl
	{
		KillTimer(nIDEvent);
		UpdateData(TRUE);
		m_wndPlot.updateData();
	}
	else if (nIDEvent == 2) // ISO
	{
		KillTimer(nIDEvent);
		UpdateData(TRUE);
		if (m_viewParams.isoStep <= 0)
			m_viewParams.isoStep = 0.1;
		if (m_viewParams.isValid())
		{
			m_wndPlot.setViewParams(m_viewParams);
			m_wndPlot.updateData();
		}
	}
	else if (nIDEvent == 3) // Z min-max-step
	{
		KillTimer(nIDEvent);
		UpdateData(TRUE);
		if (m_viewParams.isValid())
		{
			m_wndPlot.setViewParams(m_viewParams);
			m_wndPlot.updateData();
		}
	}
	else if (nIDEvent == 4) // View
	{
		KillTimer(nIDEvent);
		UpdateData(TRUE);
		if (m_viewParams.isValid())
		{
			m_wndPlot.setViewParams(m_viewParams);
			m_wndPlot.updateData();
		}
	}
	else if (nIDEvent == 5) // Polar - Decart
	{
		KillTimer(nIDEvent);
		UpdateData(TRUE);
		if (m_viewParams.isValid())
		{
			m_wndPlot.setViewParams(m_viewParams);
			m_wndPlot.updateData();
		}
	}
	else if (nIDEvent == 6) // Grid0, Rainbow
	{
		KillTimer(nIDEvent);
		UpdateData(TRUE);
		if (m_viewParams.isValid())
		{
			m_wndPlot.setViewParams(m_viewParams);
			m_wndPlot.Invalidate();
		}
	}

	CFormView::OnTimer(nIDEvent);
}

void CAntexPcvView::OnSelchangeComboSignal()
{
	SetTimer(1, 1, NULL);
}

void CAntexPcvView::OnSelchangeComboViewAt()
{
	SetTimer(4, 1, NULL);
}

void CAntexPcvView::OnEnChangeEditIso()
{
	SetTimer(2, 1, NULL);
}

void CAntexPcvView::OnChangeEditZMinMax()
{
	SetTimer(3, 1, NULL);
}

void CAntexPcvView::OnPolar()
{
	SetTimer(5, 1, NULL);
}

void CAntexPcvView::OnGrid0()
{
	SetTimer(6, 1, NULL);
}

void CAntexPcvView::OnRainbow()
{
	SetTimer(6, 1, NULL);
}