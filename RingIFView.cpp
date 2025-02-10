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
#include "AntexAntenna.h"
#include "RingIFView.h"
#include "PromptTitleDlg.h"
#include "Settings.h"
#include "CurveFormatPg.h"
#include "Tex2Csv.h"

// CRingIFView
CRingDoc* CRingIFView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CRingDoc)));
	return (CRingDoc*)m_pDocument;
}


IMPLEMENT_DYNCREATE(CRingIFView, CFormView)

CRingIFView::CRingIFView()
	: CFormView(CRingIFView::IDD)
	//, m_nCoord(0)
	, m_yMax(1)
	, m_yMin(0)
	, m_xMin(0)
	, m_xMax(1)
	, m_byAuto(TRUE)
	, m_bxAuto(TRUE)
	, m_yDivs(10)
	, m_xDivs(10)
	, m_bOffsetFromAntex(FALSE)
	, m_bMarker(FALSE)
{
	m_bNeedInit = true;
	m_eleMask = 0.0;
}

CRingIFView::~CRingIFView()
{
}

void CRingIFView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CUSTOM_PLOT, m_wndPlot);
	//DDX_Radio(pDX, IDC_RADIO_PCC, m_nCoord);
	DDX_Check(pDX, IDC_CHECK_PCC, m_show.pccS);
	DDX_Check(pDX, IDC_CHECK_PCC2, m_show.pccC);
	DDX_Check(pDX, IDC_CHECK_PCO, m_show.pco);
	DDX_Check(pDX, IDC_CHECK_U, m_show.u);
	DDX_Check(pDX, IDC_CHECK_N, m_show.n);
	DDX_Check(pDX, IDC_CHECK_E, m_show.e);
	DDX_Check(pDX, IDC_CHECK_H, m_show.h);
	DDX_Check(pDX, IDC_CHECK_PCV, m_show.pcv);
	DDX_Check(pDX, IDC_CHECK_NA_PCV, m_show.naPcv);
	DDX_Text(pDX, IDC_EDIT_ELE_MASK, m_eleMask);
	DDX_Control(pDX, IDC_COMBO_REF, m_cmbReference);
	DDX_Control(pDX, IDC_COMBO_COMBINATION, m_cmbSignal);
	//DDX_Check(pDX, IDC_CHECK_OFFSET_FROM_ANTEX, m_bOffsetFromAntex);
	//DDX_Check(pDX, IDC_CHECK_MARKER, m_bMarker);

	DDX_Text(pDX, IDC_EDIT_Y_MAX, m_yMax);
	DDX_Text(pDX, IDC_EDIT_Y_MIN, m_yMin);
	DDX_Text(pDX, IDC_EDIT_X_MIN, m_xMin);
	DDX_Text(pDX, IDC_EDIT_X_MAX, m_xMax);
	DDX_Check(pDX, IDC_CHECK_Y_AUTO, m_byAuto);
	DDX_Check(pDX, IDC_CHECK_X_AUTO, m_bxAuto);
	DDX_Text(pDX, IDC_EDIT_Y_DIVS, m_yDivs);
	DDX_Text(pDX, IDC_EDIT_X_DIVS, m_xDivs);
	DDX_Check(pDX, IDC_CHECK_OVERLAY, m_bOverlay);
	DDX_Text(pDX, IDC_EDIT_OVERLAY, m_strOverlay);
	DDX_Control(pDX, IDC_COMBO_PCV_MODE, m_cmbPcvMode);
}

BEGIN_MESSAGE_MAP(CRingIFView, CFormView)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_CHECK_PCC, &CRingIFView::OnClickedRadioPCC)
	ON_BN_CLICKED(IDC_CHECK_PCC2, &CRingIFView::OnClickedRadioPCC)
	ON_BN_CLICKED(IDC_CHECK_PCO, &CRingIFView::OnClickedRadioPCC)
	ON_BN_CLICKED(IDC_CHECK_U, &CRingIFView::OnClickedRadioPCC)
	ON_BN_CLICKED(IDC_CHECK_N, &CRingIFView::OnClickedRadioPCC)
	ON_BN_CLICKED(IDC_CHECK_E, &CRingIFView::OnClickedRadioPCC)
	ON_BN_CLICKED(IDC_CHECK_H, &CRingIFView::OnClickedRadioPCC)
	ON_BN_CLICKED(IDC_CHECK_PCV, &CRingIFView::OnClickedRadioPCC)
	ON_BN_CLICKED(IDC_CHECK_NA_PCV, &CRingIFView::OnClickedRadioPCC)
	//ON_BN_CLICKED(IDC_RADIO_PCC, &CRingIFView::OnClickedRadioPCC)
	//ON_BN_CLICKED(IDC_RADIO_PCO2, &CRingIFView::OnClickedRadioPCC)
	//ON_BN_CLICKED(IDC_RADIO_PCV, &CRingIFView::OnClickedRadioPCC)
	//ON_BN_CLICKED(IDC_RADIO_NOAPCV, &CRingIFView::OnClickedRadioPCC)
	ON_EN_CHANGE(IDC_EDIT_ELE_MASK, &CRingIFView::OnChangeEditEleMask)
	ON_WM_TIMER()

	ON_BN_CLICKED(IDC_CHECK_Y_AUTO, &CRingIFView::OnBnClickedCheckAuto)
	ON_BN_CLICKED(IDC_CHECK_X_AUTO, &CRingIFView::OnBnClickedCheckAuto)
	ON_EN_CHANGE(IDC_EDIT_X_MAX, &CRingIFView::OnChangeEditXMinMax)
	ON_EN_CHANGE(IDC_EDIT_X_MIN, &CRingIFView::OnChangeEditXMinMax)
	ON_EN_CHANGE(IDC_EDIT_X_DIVS, &CRingIFView::OnChangeEditXMinMax)
	ON_EN_CHANGE(IDC_EDIT_Y_MAX, &CRingIFView::OnChangeEditYMinMax)
	ON_EN_CHANGE(IDC_EDIT_Y_MIN, &CRingIFView::OnChangeEditYMinMax)
	ON_EN_CHANGE(IDC_EDIT_Y_DIVS, &CRingIFView::OnChangeEditYMinMax)
	ON_BN_CLICKED(IDC_BUTTON_ADD_COMMENT, &CRingIFView::OnBnClickedButtonAddComment)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_COMMENTS, &CRingIFView::OnBnClickedButtonRemoveComments)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CRingIFView::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, &CRingIFView::OnBnClickedButtonLoad)
	ON_BN_CLICKED(IDC_CHECK_OVERLAY, &CRingIFView::OnBnClickedCheckOverlay)
	//ON_BN_CLICKED(IDC_CHECK_USE_RO, &CRingIFView::OnBnClickedCheckOverlay)

	ON_BN_CLICKED(IDC_BUTTON_SET_PLOT_TITLE, &CRingIFView::OnBnClickedButtonSetPlotTitle)
	ON_CBN_SELCHANGE(IDC_COMBO_COMBINATION, &OnSelchangeComboSignal)
	ON_CBN_SELCHANGE(IDC_COMBO_REF, &OnSelchangeComboRef)
	ON_CBN_SELCHANGE(IDC_COMBO_PCV_MODE, &OnSelchangeComboRef)

	ON_COMMAND (ID_PLOT2D_CURVEFORMAT, &OnCurveFormat)
END_MESSAGE_MAP()


// CRingIFView diagnostics

#ifdef _DEBUG
void CRingIFView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CRingIFView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

void CRingIFView::enableControls()
{
	BOOL bxAuto = m_bxAuto;
	BOOL byAuto = m_byAuto;

	double xMin = m_xMin;
	double xMax = m_xMax;
	int xDivs = m_xDivs;
	if (bxAuto)
	{
		xMin = m_wndPlot.m_xAxis.m_min;
		xMax = m_wndPlot.m_xAxis.m_max;
		xDivs = m_wndPlot.m_xAxis.m_divs;
	}

	double yMin = m_yMin;
	double yMax = m_yMax;
	int yDivs = m_yDivs;
	if (byAuto)
	{
		yMin = m_wndPlot.m_yAxis.m_min;
		yMax = m_wndPlot.m_yAxis.m_max;
		yDivs = m_wndPlot.m_yAxis.m_divs;
	}

	UpdateData(TRUE);

	m_xMin = xMin;
	m_xMax = xMax;
	m_xDivs = xDivs;

	m_yMin = yMin;
	m_yMax = yMax;
	m_yDivs = yDivs;

	//GetDlgItem(IDC_CHECK_X_AUTO)->EnableWindow(m_bTimeXAxis ? FALSE : TRUE);
	GetDlgItem(IDC_EDIT_X_MIN)->EnableWindow(!m_bxAuto);
	GetDlgItem(IDC_EDIT_X_MAX)->EnableWindow(!m_bxAuto);
	GetDlgItem(IDC_EDIT_Y_MIN)->EnableWindow(!m_byAuto);
	GetDlgItem(IDC_EDIT_Y_MAX)->EnableWindow(!m_byAuto);

	m_wndPlot.autoScale(m_bxAuto, m_byAuto, TRUE);

	//m_cmbDataType.ShowWindow(m_bShowDataType ? SW_SHOW : SW_HIDE);

	UpdateData(FALSE);
}


// CRingIFView message handlers


void CRingIFView::OnSize(UINT nType, int cx, int cy)
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

	CRect rectPlot;
	for (CWnd* pChild = GetWindow(GW_CHILD);
		pChild != NULL;
		pChild = pChild->GetWindow(GW_HWNDNEXT))
	{
		int nId = pChild->GetDlgCtrlID();
		if (nId == IDC_RADIO_EAST || nId == IDC_RADIO_NORTH || nId == IDC_RADIO_UP)
		{
		}
		else if (nId == IDC_CUSTOM_PLOT)
		{
			pChild->GetWindowRect(&r1);
			ScreenToClient(r1);
			r1.right = m_rCrt.right - 20; // += dx;
			r1.bottom = m_rCrt.bottom - 60; // += dy;
			rectPlot = r1;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, 0, 0,
				r1.Width(), r1.Height(),
				SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
		}

	}

	for (CWnd* pChild = GetWindow(GW_CHILD);
		pChild != NULL;
		pChild = pChild->GetWindow(GW_HWNDNEXT))
	{
		int nID = pChild->GetDlgCtrlID();
		pChild->GetWindowRect(&r1); ScreenToClient(&r1);

		if (nID == IDC_EDIT_X_MIN)
		{
			CRect r = r1;
			r.left = rectPlot.left;
			r.right = rectPlot.left + r1.Size().cx;
			r.top = rectPlot.bottom + 5;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (nID == IDC_EDIT_X_MAX)
		{
			CRect r = r1;
			r.left = rectPlot.right - r1.Size().cx;
			r.right = rectPlot.right;
			r.top = rectPlot.bottom + 5;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (nID == IDC_CHECK_X_AUTO)
		{
			CRect r = r1;
			r.left = (rectPlot.right + rectPlot.left) / 2 - r1.Size().cx / 2;
			r.right = (rectPlot.right + rectPlot.left) / 2 + r1.Size().cx / 2;
			r.top = rectPlot.bottom + 5;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		if (nID == IDC_EDIT_Y_MIN)
		{
			CRect r = r1;
			r.top = rectPlot.bottom - r1.Size().cy;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (nID == IDC_EDIT_Y_MAX)
		{
			CRect r = r1;
			r.top = rectPlot.top;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (nID == IDC_CHECK_Y_AUTO)
		{
			CRect r = r1;
			r.top = (rectPlot.top + rectPlot.bottom) / 2 - r1.Size().cy / 2;
			r.bottom = (rectPlot.top + rectPlot.bottom) / 2 + r1.Size().cy / 2;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (nID == IDC_EDIT_Y_DIVS)
		{
			CRect r = r1;
			r.top = (rectPlot.top + rectPlot.bottom) / 2 + (rectPlot.bottom - rectPlot.top) / 4 - r1.Size().cy / 2;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (nID == IDC_EDIT_X_DIVS)
		{
			CRect r = r1;
			r.left = (rectPlot.left + rectPlot.right) / 2 - (rectPlot.right - rectPlot.left) / 4 - r1.Size().cx / 2 - 15 - r1.Size().cx;
			r.right = r.left + r1.Size().cx;
			r.top = rectPlot.bottom + 5;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (nID == IDC_BUTTON_SAVE)
		{
			CRect r = r1;
			r.left = (rectPlot.left + rectPlot.right) / 2 - (rectPlot.right - rectPlot.left) / 4 - r1.Size().cx / 2;
			r.right = r.left + r1.Size().cx;
			r.top = rectPlot.bottom + 5;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (nID == IDC_BUTTON_LOAD)
		{
			CRect r = r1;
			r.left = (rectPlot.left + rectPlot.right) / 2 - (rectPlot.right - rectPlot.left) / 4 + r1.Size().cx / 2 + 15;
			r.right = r.left + r1.Size().cx;
			r.top = rectPlot.bottom + 5;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (nID == IDC_CHECK_OVERLAY)
		{
			CRect r = r1;
			r.left = (rectPlot.left + rectPlot.right) / 2 + (rectPlot.right - rectPlot.left) / 10 - r1.Size().cx - 5;
			r.right = r.left + r1.Size().cx;
			r.top = rectPlot.bottom + 5;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (nID == IDC_EDIT_OVERLAY)
		{
			CRect r = r1;
			r.left = (rectPlot.left + rectPlot.right) / 2 + (rectPlot.right - rectPlot.left) / 10 - 5;
			r.right = r.left + r1.Size().cx;
			r.top = rectPlot.bottom + 5;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (nID == IDC_BUTTON_ADD_COMMENT)
		{
			CRect r = r1;
			r.left = (rectPlot.left + rectPlot.right) / 2 + (rectPlot.right - rectPlot.left) / 4 + 5; // -r1.Size().cx - 5;
			r.right = r.left + r1.Size().cx;
			r.top = rectPlot.bottom + 5;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		/*
		else if (nID == IDC_BUTTON_REMOVE_COMMENTS)
		{
			CRect r = r1;
			r.left = (rectPlot.left + rectPlot.right) / 2 + (rectPlot.right - rectPlot.left) / 4 + 5;
			r.right = r.left + r1.Size().cx;
			r.top = rectPlot.bottom + 5;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		*/

	}


	::EndDeferWindowPos(hDWP);

}


void CRingIFView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	GetParentFrame()->RecalcLayout();
	ResizeParentToFit(FALSE);
	ResizeParentToFit(TRUE);

	UpdateData(FALSE);

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
	GetFont()->GetLogFont(&lf);

	lf.lfHeight = lf.lfHeight * 3 / 2;
	{
		HDC hdc = ::GetDC(NULL);
		int xLogPx = GetDeviceCaps(hdc, LOGPIXELSX);
		lf.lfHeight = (LONG)ceil(18.0 * xLogPx / 72.0);

		::ReleaseDC(NULL, hdc);
	}
	m_font.CreateFontIndirect(&lf);

	m_wndPlot.initControl(this);
	m_wndPlot.setLegendFont(&m_font);
	m_wndPlot.setMenuIDs(IDR_POPUP, 0, NULL);

	m_wndPlot.setAxisPrecision(TRUE, 0);
	m_wndPlot.setAxisPrecision(FALSE, 0);

	m_wndPlot.setTitleFont(lf);

	fillReferences();
	fillSignals();
	fillPcvMode();

	updateCurves();
}

can2::Gnss::Signal CRingIFView::getSignal()
{
	int nSel = m_cmbSignal.GetCurSel();
	if (nSel < 0)
		return can2::Gnss::esigInvalid;
	return (can2::Gnss::Signal)m_cmbSignal.GetItemData(nSel);
}

void CRingIFView::fillSignals()
{
	m_cmbSignal.ResetContent();
	for (int i = 0; i <= can2::Gnss::esigMax; i++)
	{
		can2::Gnss::Signal es = (can2::Gnss::Signal)i;
		LPCTSTR szSignal = es == can2::Gnss::esigInvalid ? "All W/O combinations" :
			es == can2::Gnss::esigMax ? "All with Combinations" : can2::Gnss::getSignalName(es);
		int index = m_cmbSignal.AddString(szSignal);
		m_cmbSignal.SetItemData(index, es);
		if (es == can2::Gnss::GIFL2)
			m_cmbSignal.SetCurSel(index);
	}
	/*
	int index = m_cmbSignal.AddString("GPS IF L1-L2");
	m_cmbSignal.SetItemData(index, can2::Gnss::GIFL2);
	m_cmbSignal.SetCurSel(index);
	index = m_cmbSignal.AddString("GLO IF L1-L2");
	m_cmbSignal.SetItemData(index, can2::Gnss::RIFL2);
	index = m_cmbSignal.AddString("GAL IF L1-L5a");
	m_cmbSignal.SetItemData(index, can2::Gnss::EIFL5a);
	index = m_cmbSignal.AddString("GAL IF L1-L5ab");
	m_cmbSignal.SetItemData(index, can2::Gnss::EIFL5ab);
	index = m_cmbSignal.AddString("GAL IF L1-L5b");
	m_cmbSignal.SetItemData(index, can2::Gnss::EIFL5b);
	*/
}

CRingIFView::Reference CRingIFView::getReference()
{
	int nSel = m_cmbReference.GetCurSel();
	if (nSel < 0)
		return Reference(ertNone, 0);
	return m_refs[nSel];
}

can2::Node::PcvMode CRingIFView::getPcvMode()
{
	int nSel = m_cmbPcvMode.GetCurSel();
	if (nSel < 0)
		return can2::Node::epm0;
	return (can2::Node::PcvMode)m_cmbPcvMode.GetItemData(nSel);
}

void CRingIFView::fillReferences()
{
	CRingDoc* pDoc = GetDocument();
	can2::RingNode* prn = pDoc->m_rn;

	m_cmbReference.ResetContent();
	m_refs.clear();

	m_refs.push_back(Reference(ertNone, 0));
	int index = m_cmbReference.AddString("None");
	m_cmbReference.SetItemData(index, 0);
	m_cmbReference.SetCurSel(0);

	for (int nc = 0; nc < (int)prn->m_cls.size(); nc++)
	{
		if (prn->m_cls[nc].ants.size() <= 1)
			continue;

		std::string str = prn->m_cls[nc].atm->getName();
		int index = m_cmbReference.AddString(str.c_str());
		m_cmbReference.SetItemData(index, m_refs.size());
		m_refs.push_back(Reference (ertCluster, nc));
	}

	for (int na = 0; na < (int)prn->m_ants.size(); na++)
	{
		std::string str = prn->m_ants[na]->getName();
		int index = m_cmbReference.AddString(str.c_str());
		m_cmbReference.SetItemData(index, m_refs.size());
		m_refs.push_back(Reference(ertAntenna, na));
	}
}

void CRingIFView::fillPcvMode()
{
	m_cmbPcvMode.ResetContent();
	int index = m_cmbPcvMode.AddString("0 at zenith");
	m_cmbPcvMode.SetItemData(index, can2::Node::epm0);
	index = m_cmbPcvMode.AddString("Remove ro");
	m_cmbPcvMode.SetItemData(index, can2::Node::epmRo);
	index = m_cmbPcvMode.AddString("MinMax");
	m_cmbPcvMode.SetItemData(index, can2::Node::epmMinMax);
	m_cmbPcvMode.SetCurSel(index);
}

void CRingIFView::updateCurves()
{
	updateCurves2();

	/*
	CWaitCursor wc;

	if (!IsWindow(m_wndPlot.GetSafeHwnd()))
		return;

	UpdateData(TRUE);
	CRingDoc* pDoc = GetDocument();
	can2::RingNode* prn = pDoc->m_rn;
	can2::Gnss::Signal sigSel = getSignal();
	Reference ref = getReference();
	can2::Node::PcvMode epmMode = getPcvMode();

	//COLORREF clrs[9] = { RGB(255, 0, 0), RGB(0, 200, 0), RGB(0, 0, 255),
	//	RGB(200, 0, 200), RGB(125, 125, 0), RGB(0, 255, 255),
	//	RGB(120, 0, 0), RGB(0, 120, 0), RGB(255, 128, 120) };

	m_wndPlot.removeAllCurves();
	m_wndPlot.removeAllCircles();
	m_wndPlot.clearMarkers();
	m_wndPlot.removeHistGroup();

	m_wndPlot.setAxisTitle(TRUE, "", FALSE);
	m_wndPlot.setAxisTitle(FALSE, "(mm)", FALSE);
	m_wndPlot.resetInitArea();

	can2::RingAntenna* pref = nullptr;
	if (ref.ert == ertCluster)
		pref = prn->m_cls[ref.n].atm.get();
	else if (ref.ert == ertAntenna)
		pref = prn->m_ants[ref.n].get();
	else
		return;

	can2::HistGroup hg;

	hg.name = "SCM";

	COLORREF clrs[4] = { RGB(240, 106, 46), RGB(91, 155, 213), RGB(255, 192, 0), RGB(64, 191, 70) };
	COLORREF clrsPpp[4] = { RGB(200, 80, 26), RGB(70, 120, 180), RGB(180, 160, 0), RGB(0, 0, 0) };

	if (m_nCoord <= 1)
	{
		const char* valNames[4] = { "East", "North", "Up", "Sigma" };
		std::string antType;
		std::vector<std::string> groupNames;
		for (int na = 0; na < (int)prn->m_ants.size(); na++)
		{
			can2::RingAntenna* pa = prn->m_ants[na].get();
			std::shared_ptr<can2::Node> pd(pref->subtract(pa));
			if (pa == pref)
				continue;
			can2::AntexAntenna* pda = (can2::AntexAntenna*)pd.get();
			if (!pda->isAntexAntenna())
				continue;

			antType = pa->m_type;

			double ro = 0;
			can2::Point3d pco(NAN, NAN, NAN);
			double dpcc = NAN;
			if (sigSel == can2::Gnss::esigInvalid || sigSel == can2::Gnss::esigMax)
			{
				can2::Gnss::Signal sigMax = sigSel;
				for (int i = 0; i < sigMax; i++)
				{
					if (!pda->hasPcc(i))
						continue;
					can2::Point3d p = pda->calcOffset((can2::Gnss::Signal)i, m_eleMask, can2::Node::eSinAndCos, &ro) * 1000;
					double d = pda->calcNorm((can2::Gnss::Signal)i, can2::Node::eSinAndCos, true) * 1000;
					for (int j = 0; j < 3; j++)
					{
						if (_isnan(pco[j]))
							pco[j] = p[j];
						else if (fabs(p[j]) > fabs(pco[j]))
							pco[j] = p[j];
					}
					if (_isnan(dpcc))
						dpcc = d;
					else if (fabs(dpcc) < fabs(d))
						dpcc = d;
				}
			}
			else
			{
				pco = pda->calcOffset(sigSel, m_eleMask, can2::Node::eSinAndCos, &ro) * 1000;
				dpcc = pda->calcNorm(sigSel, can2::Node::eSinAndCos, true) * 1000;
			}
			double vals[4];
			for (int i = 0; i < 3; i++)
				vals[i] = pco[i];
			vals[3] = dpcc;
			hg.addGroup(pa->getName().c_str(), vals, valNames, clrs, m_nCoord == 0 ? 4 : 3);
			groupNames.push_back(pa->getName());
		}

		m_wndPlot.addHistGroup(hg);

		std::vector<PppItem> pppItems;
		if (m_bOverlay && selectPpp(antType.c_str(), sigSel, pppItems))
		{
			can2::HistGroup hg;

			hg.name = "PPP";

			for (int na = 0; na < (int)groupNames.size(); na++)
			{
				std::vector<PppItem>::iterator it = std::find_if(pppItems.begin(), pppItems.end(), [&](const PppItem& item) {
					return item.lab == groupNames[na];
				});

				PppItem item = it == pppItems.end() ? PppItem() : *it;
				item.lab = groupNames[na];

				double vals[4];
				for (int i = 0; i < 3; i++)
					vals[i] = item.enu[i];
				vals[3] = NAN;
				hg.addGroup(groupNames[na].c_str(), vals, valNames, clrsPpp, m_nCoord == 0 ? 4 : 3);
			}

			m_wndPlot.addHistGroup(hg);
		}

		LPCTSTR szSubTitle = m_nCoord == 0 ? "MAX DPCO and Sigma" : "MAX DPCO";
		LPCTSTR szSigs = sigSel == can2::Gnss::esigInvalid ? "for all signals" : sigSel == can2::Gnss::esigMax ? "for all sig. with comb." : can2::Gnss::getSignalName(sigSel);
		std::string strTitle = can2::stringFormat("%s, %s %s, ref=%s", prn->m_title.c_str(), szSubTitle, szSigs, pref->getName().c_str());
		m_wndPlot.addPlotTitle(strTitle.c_str());
	}
	else // m_nCoord == 2 (PCV)
	{
		std::string antType;
		const char* valNames[4] = { "ele > 10", "ele<=10" };
		std::vector<std::string> groupNames;
		for (int na = 0; na < (int)prn->m_ants.size(); na++)
		{
			can2::RingAntenna* pa = prn->m_ants[na].get();
			std::shared_ptr<can2::Node> pd(pref->subtract(pa));
			if (pa == pref)
				continue;

			antType = pa->m_type;

			std::pair<double, double> r = pd->getMaxPcc(sigSel, m_nCoord == 3, 10.0, epmMode, can2::Node::eSinAndCos);
			double vals[2] = { r.first*1000, r.second*1000 };
			hg.addGroup(pa->getName().c_str(), vals, valNames, clrs, 2);
			groupNames.push_back(pa->getName());
		}

		m_wndPlot.addHistGroup(hg);
		LPCTSTR szSigs = sigSel == can2::Gnss::esigInvalid ? "for all signals" : sigSel == can2::Gnss::esigMax ? "for all sig. w comb" : can2::Gnss::getSignalName(sigSel);
		std::string strTitle = can2::stringFormat("%s, MAX %s %s, ref=%s", antType.c_str(), m_nCoord == 2 ? "DeltaPCV" : "NA DeltaPCV", szSigs, pref->getName().c_str());
		m_wndPlot.addPlotTitle(strTitle.c_str());
	}

	m_wndPlot.setInitArea();
	m_wndPlot.Invalidate();
	m_wndPlot.UpdateWindow();
	*/
}

void CRingIFView::updateCurves2()
{
	CWaitCursor wc;

	if (!IsWindow(m_wndPlot.GetSafeHwnd()))
		return;

	UpdateData(TRUE);
	CRingDoc* pDoc = GetDocument();
	can2::RingNode* prn = pDoc->m_rn;
	can2::Gnss::Signal sigSel = getSignal();
	Reference ref = getReference();
	can2::Node::PcvMode epmMode = getPcvMode();

	//COLORREF clrs[9] = { RGB(255, 0, 0), RGB(0, 200, 0), RGB(0, 0, 255),
	//	RGB(200, 0, 200), RGB(125, 125, 0), RGB(0, 255, 255),
	//	RGB(120, 0, 0), RGB(0, 120, 0), RGB(255, 128, 120) };

	m_wndPlot.removeAllCurves();
	m_wndPlot.removeAllCircles();
	m_wndPlot.clearMarkers();
	m_wndPlot.removeHistGroup();

	m_wndPlot.setAxisTitle(TRUE, "", FALSE);
	m_wndPlot.setAxisTitle(FALSE, "(mm)", FALSE);
	m_wndPlot.resetInitArea();

	if (m_show.isNothing())
		return;

	can2::RingAntenna* pref = nullptr;
	if (ref.ert == ertCluster)
		pref = prn->m_cls[ref.n].atm.get();
	else if (ref.ert == ertAntenna)
		pref = prn->m_ants[ref.n].get();
	else
		return;

	can2::HistGroup hg;

	hg.name = "SCM";

	//COLORREF clrs[4] = { RGB(240, 106, 46), RGB(91, 155, 213), RGB(255, 192, 0), RGB(64, 191, 70) };
	COLORREF clrs[11] = { RGB(64, 191, 70), RGB(43, 130, 47), RGB(91, 155, 213),
		RGB(240, 106, 46), RGB(0, 230, 230), RGB(255, 192, 0), RGB(37, 21, 232), 
		RGB(158, 72, 14), RGB(145, 155, 13), RGB(163, 73, 164), RGB(244, 98, 123) };
		//RGB(58, 114, 196), , RGB(91, 155, 213), RGB(37, 94, 145) };
	COLORREF clrsPpp[4] = { RGB(65, 105, 225), RGB(178, 34, 34), RGB(218, 165, 32), RGB(0, 0, 0) }; // 200, 80, 26), RGB(70, 120, 180), RGB(180, 160, 0), RGB(0, 0, 0)

	const char* valNames[11] = { "DPCC", "DPCC Full","DPCO", "east", "north",
		"vertical", "DHPCO", "DPCV NA e>10", "DPCV NA e<10", "DPCV e>10", "DPCV e<10" };

	std::string antType;
	std::vector<std::string> groupNames;
	for (int na = 0; na < (int)prn->m_ants.size(); na++)
	{
		can2::RingAntenna* pa = prn->m_ants[na].get();
		std::shared_ptr<can2::Node> pd(pref->subtract(pa));
		if (pa == pref)
			continue;
		can2::AntexAntenna* pda = (can2::AntexAntenna*)pd.get();
		if (!pda->isAntexAntenna())
			continue;

		antType = pa->m_type;

		double ro = 0;
		can2::Point3d pcoComp(NAN, NAN, NAN);
		double dpccS = NAN;
		double dpccC = NAN;
		double dpco = NAN;
		double dhpco = NAN;
		if (sigSel == can2::Gnss::esigInvalid || sigSel == can2::Gnss::esigMax)
		{
			can2::Gnss::Signal sigMax = sigSel;
			for (int i = 0; i < sigMax; i++)
			{
				if (!pda->hasPcc(i))
					continue;
				can2::Point3d p = pda->calcOffset((can2::Gnss::Signal)i, m_eleMask, can2::Node::eSinAndCos, &ro) * 1000;
				double dS = pda->calcNorm((can2::Gnss::Signal)i, can2::Node::eSinAndCos, true) * 1000;
				double dC = pda->calcNorm((can2::Gnss::Signal)i, can2::Node::eSinAndCos, false) * 1000;
				for (int j = 0; j < 3; j++)
				{
					if (_isnan(pcoComp[j]))
						pcoComp[j] = p[j];
					else if (fabs(p[j]) > fabs(pcoComp[j]))
						pcoComp[j] = p[j];
				}
				if (_isnan(dpccS))
					dpccS = dS;
				else if (fabs(dpccS) < fabs(dS))
					dpccS = dS;
				if (_isnan(dpccC))
					dpccC = dC;
				else if (fabs(dpccC) < fabs(dC))
					dpccC = dC;
				if (_isnan(dpco))
					dpco = p.rad();
				else if (dpco < p.rad())
					dpco = p.rad();
				if (_isnan(dhpco))
					dhpco = sqrt(p.x * p.x + p.y * p.y);
				else if (dhpco < sqrt(p.x * p.x + p.y * p.y))
					dhpco = sqrt(p.x * p.x + p.y * p.y);
			}
		}
		else
		{
			pcoComp = pda->calcOffset(sigSel, m_eleMask, can2::Node::eSinAndCos, &ro) * 1000;
			dpccS = pda->calcNorm(sigSel, can2::Node::eSinAndCos, true) * 1000;
			dpccC = pda->calcNorm(sigSel, can2::Node::eSinAndCos, false) * 1000;
			dpco = pcoComp.rad();
			dhpco = sqrt(pcoComp.x * pcoComp.x + pcoComp.y * pcoComp.y);
		}

		double pcvHi = NAN;
		double pcvLo = NAN;
		if (m_show.pcv)
		{
			std::pair<double, double> r = pd->getMaxPcc(sigSel, false, 10.0, epmMode, can2::Node::eSinAndCos);
			pcvHi = r.first * 1000;
			pcvLo = r.second * 1000;
		}
		double naPcvHi = NAN;
		double naPcvLo = NAN;
		if (m_show.naPcv)
		{
			std::pair<double, double> r = pd->getMaxPcc(sigSel, true, 10.0, epmMode, can2::Node::eSinAndCos);
			naPcvHi = r.first * 1000;
			naPcvLo = r.second * 1000;
		}

		std::vector<double> vs;
		std::vector<const char*> vNames;
		std::vector<COLORREF> vClrs;

		if (m_show.pccS)
		{
			vs.push_back(dpccS);
			vNames.push_back(valNames[0]);
			vClrs.push_back(clrs[0]);
		}
		if (m_show.pccC)
		{
			vs.push_back(dpccC);
			vNames.push_back(valNames[1]);
			vClrs.push_back(clrs[1]);
		}
		if (m_show.pco)
		{
			vs.push_back(dpco);
			vNames.push_back(valNames[2]);
			vClrs.push_back(clrs[2]);
		}
		if (m_show.n)
		{
			vs.push_back(pcoComp.n);
			vNames.push_back(valNames[4]);
			vClrs.push_back(RGB(65, 105, 225)); // clrs[3]);
		}
		if (m_show.e)
		{
			vs.push_back(pcoComp.e);
			vNames.push_back(valNames[3]);
			vClrs.push_back(RGB(178, 34, 34)); // clrs[2]);
		}
		if (m_show.u)
		{
			vs.push_back(pcoComp.u);
			vNames.push_back(valNames[5]);
			vClrs.push_back(RGB(218, 165, 32)); // clrs[4]);
		}
		if (m_show.h)
		{
			vs.push_back(dhpco);
			vNames.push_back(valNames[6]);
			vClrs.push_back(clrs[6]);
		}
		if (m_show.naPcv)
		{
			vs.push_back(naPcvHi);
			vNames.push_back(valNames[7]);
			vClrs.push_back(clrs[7]);
			vs.push_back(naPcvLo);
			vNames.push_back(valNames[8]);
			vClrs.push_back(clrs[8]);
		}
		if (m_show.pcv)
		{
			vs.push_back(pcvHi);
			vNames.push_back(valNames[9]);
			vClrs.push_back(clrs[9]);
			vs.push_back(pcvLo);
			vNames.push_back(valNames[10]);
			vClrs.push_back(clrs[10]);
		}
		hg.addGroup(pa->getName().c_str(), vs, vNames, vClrs);
		groupNames.push_back(pa->getName());
	}

	if (!m_bOverlay)
		m_wndPlot.addHistGroup(hg);

	std::vector<PppItem> pppItems;
	if (m_bOverlay && selectPpp(antType.c_str(), sigSel, pppItems))
	{
		can2::HistGroup hg;

		hg.name = "PPP";
		const char* szValNames[3] = { "north", "east", "vertical" };

		for (int na = 0; na < (int)groupNames.size(); na++)
		{
			std::vector<PppItem>::iterator it = std::find_if(pppItems.begin(), pppItems.end(), [&](const PppItem& item) {
				if (item.lab == groupNames[na])
					return true;
				if (groupNames[na] == "ETH" && item.lab == "ETH1")
					return true;
				return false;
			});

			PppItem item = it == pppItems.end() ? PppItem() : *it;
			item.lab = groupNames[na];

			double vals[4];
			vals[0] = item.enu[1];
			vals[1] = item.enu[0];
			vals[2] = item.enu[2];
			//for (int i = 0; i < 3; i++)
			//	vals[i] = item.enu[i];
			vals[3] = NAN;
			hg.addGroup(groupNames[na].c_str(), vals, szValNames, clrsPpp, 3); // m_nCoord == 0 ? 4 : 3);
		}

		m_wndPlot.addHistGroup(hg);
	}

	//LPCTSTR szSubTitle = m_nCoord == 0 ? "MAX DPCO and Sigma" : "MAX DPCO";
	LPCTSTR szSigs = sigSel == can2::Gnss::esigInvalid ? "for all signals" : sigSel == can2::Gnss::esigMax ? "for all sig. with comb." : can2::Gnss::getSignalName(sigSel);
	std::string strTitle = can2::stringFormat("%s, MAX delta metrics %s, ref=%s", prn->m_title.c_str(), szSigs, pref->getName().c_str());
	m_wndPlot.addPlotTitle(strTitle.c_str());

	m_wndPlot.setInitArea();
	m_wndPlot.Invalidate();
	m_wndPlot.UpdateWindow();
}

bool CRingIFView::selectPpp(const char* name, can2::Gnss::Signal sig, std::vector<PppItem>& pppItems) const
{
	std::string sys;
	if (sig == can2::Gnss::GIFL2 || sig == can2::Gnss::GIFL5 || sig == can2::Gnss::GWLL2 || sig == can2::Gnss::GWLL5)
	{
		sys = "G";
	}
	else if (sig == can2::Gnss::RIFL2 || sig == can2::Gnss::RWLL2)
	{
		sys = "R";
	}
	else if (sig == can2::Gnss::EIFL5a || sig == can2::Gnss::EIFL5ab || sig == can2::Gnss::EIFL5b ||
		sig == can2::Gnss::EWLL5a || sig == can2::Gnss::EWLL5ab || sig == can2::Gnss::EWLL5b)
	{
		sys = "E";
	}
	else if (sig == can2::Gnss::CIF0106 || sig == can2::Gnss::CIF0206 || 
		sig == can2::Gnss::CWL0106 || sig == can2::Gnss::CWL0206)
	{
		sys = "C";
	}

	for (int i = 0; i < (int)m_pppItems.size(); i++)
	{
		if (m_pppItems[i].ant != name)
			continue;
		if (m_pppItems[i].sys != sys)
			continue;
		pppItems.push_back(m_pppItems[i]);
	}
	return pppItems.size() > 0;
}

void CRingIFView::OnClickedRadioPCC()
{
	SetTimer(100, 1, NULL);
}


void CRingIFView::OnRadioNorth()
{
	SetTimer(100, 1, NULL);
}


void CRingIFView::OnRadioUp()
{
	SetTimer(100, 1, NULL);
}


void CRingIFView::OnChangeEditEleMask()
{
	SetTimer(100, 1, NULL);
}


void CRingIFView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 100)
	{
		KillTimer(nIDEvent);
		updateCurves();
	}
	else if (nIDEvent == 1)
	{
		KillTimer(nIDEvent);
		enableControls();
		m_wndPlot.Invalidate();
		m_wndPlot.UpdateWindow();
	}
	else if (nIDEvent == 3)
	{
		KillTimer(nIDEvent);
		UpdateData(TRUE);
		m_wndPlot.setAxis(TRUE, m_xMin, m_xMax, m_xDivs);
		m_wndPlot.Invalidate();
		m_wndPlot.UpdateWindow();
	}
	else if (nIDEvent == 4)
	{
		KillTimer(nIDEvent);
		UpdateData(TRUE);
		m_wndPlot.setAxis(FALSE, m_yMin, m_yMax, m_yDivs);
		m_wndPlot.Invalidate();
		m_wndPlot.UpdateWindow();
	}
	else if (nIDEvent == 5)
	{
		KillTimer(nIDEvent);
		UpdateData(TRUE);
		updateCurves();
	}

	CFormView::OnTimer(nIDEvent);
}

void CRingIFView::OnBnClickedCheckAuto()
{
	KillTimer(1);
	SetTimer(1, 10, NULL);
}

void CRingIFView::OnBnClickedCheckOverlay()
{
	KillTimer(5);
	SetTimer(5, 10, NULL);
}

void CRingIFView::OnChangeEditXMinMax()
{
	KillTimer(3);
	SetTimer(3, 2000, NULL);
}


void CRingIFView::OnChangeEditYMinMax()
{
	KillTimer(4);
	SetTimer(4, 2000, NULL);
}

void CRingIFView::OnBnClickedButtonAddComment()
{
	/*
	CPromptTitleDlg dlg(this);
	if (dlg.DoModal() != IDOK)
		return;

	if (dlg.m_strTitle.IsEmpty())
		return;

	CString str = dlg.m_strTitle;
	m_wndPlot.AddInscription(1, str, can2::Point2d(gl_dNan, gl_dNan), DT_TOP, RGB(0, 0, 0), CInscription::etRect, &m_font);
	*/
	static CString strFile = "D:\\user\\ivs\\tps2014\\cv2\\cv2\\RingAnt\\PPP\\vGPP.csv";
	CFileDialog dlg(TRUE, "csv", strFile, OFN_HIDEREADONLY, "CSV files (*.csv)|*.csv|TexFiles (*.tex)|*.tex|All files (*.*)|*.*||", this);
	if (dlg.DoModal() != IDOK)
		return;

	strFile = dlg.GetPathName();
	std::string ext = can2::getFileExt(strFile);
	if (ext == ".tex")
	{
		CRingDoc* pDoc = GetDocument();
		can2::RingNode* prn = pDoc->m_rn;
		m_pppItems.clear();
		if (can2::Tex2Csv::convert(prn->m_title.c_str(), strFile, m_pppItems))
		{
			m_strOverlay = strFile;
			UpdateData(FALSE);
		}
		return;
	}

	CStdioFile f;
	if (!f.Open(strFile, CFile::modeRead))
	{
		AfxMessageBox("Can't open file");
		return;
	}
	m_strOverlay = strFile;
	UpdateData(FALSE);

	auto parse = [](const char* row, PppItem& item) {
		LPCTSTR p = row;
		int n = strcspn(p, ",");
		item.ant = std::string(p, n);
		p += n;
		p += strspn(p, ",");
		n = strcspn(p, ",");
		item.sys = std::string(p, n);
		p += n;
		p += strspn(p, ",");
		n = strcspn(p, ",");
		item.lab = std::string(p, n);
		for (int i = 0; i < 3; i++)
		{
			p += n;
			p += strspn(p, ",");
			n = strcspn(p, ",");
			std::string s(p, n);
			item.enu[i] = s == "nan" ? NAN : atof(p);
		}

	};

	m_pppItems.clear();
	CString str;
	int nRow = 0;
	while (f.ReadString(str))
	{
		nRow++;
		if (nRow == 1)
			continue;

		PppItem item;
		parse(str, item);
		m_pppItems.push_back(item);
	}
}


void CRingIFView::OnBnClickedButtonRemoveComments()
{
	m_wndPlot.deleteInscription(0);
}


void CRingIFView::OnBnClickedButtonSave()
{
	//AfxMessageBox("Not implemented yet");
	/*
	CFileDialog dlg(FALSE, ".plt", m_strFileName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "Plot files (*.plt)|*.plt|All files (*.*)|*.*||", this);

	if (dlg.DoModal() != IDOK)
		return;
	m_strFileName = dlg.GetPathName();

	std::ofstream ofs(m_strFileName);
	if (!ofs.good())
	{
		AfxMessageBox("Can't save to file");
		return;
	}

	try
	{
		can2::Archive ar(ofs, can2::Archive::store);
		m_wndPlot.serialize(ar);
	}
	catch (std::exception e)
	{
		AfxMessageBox(e.what());
	}
	*/
	UpdateData(TRUE);
	CRingDoc* pDoc = GetDocument();
	can2::RingNode* prn = pDoc->m_rn;
	can2::Gnss::Signal sigSel = getSignal();
	Reference ref = getReference();
	can2::Node::PcvMode epmMode = getPcvMode();

	can2::RingAntenna* pref = nullptr;
	std::string strRef = "";
	if (ref.ert == ertCluster)
	{
		pref = prn->m_cls[ref.n].atm.get();
		strRef = "CLUSm";
	}
	else if (ref.ert == ertAntenna)
	{
		pref = prn->m_ants[ref.n].get();
		strRef = pref->getName();
	}
	else
		return;

	std::string strFileName = can2::stringFormat("dpco_dpcc_v%s_%s.tex", strRef.c_str(), prn->m_title.c_str());
	std::string strFile = can2::stringFormat("C:\\User\\Ivs\\Tps2014\\Doc\\IGS\\RingCalibrations\\PcoDef\\Tabs\\%s", strFileName.c_str());
	CFileDialog dlg(FALSE, ".tex", strFile.c_str(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "LaTex files (*.tex)|*.tex|All files (*.*)|*.*||", this);

	if (dlg.DoModal() != IDOK)
		return;
	
	std::string strFilePath = dlg.GetPathName();

	std::ofstream ofs(strFilePath);
	if (!ofs.good())
	{
		AfxMessageBox("Can't save to file");
		return;
	}

	ofs << "\\begin{tabular}{rccccrcccc}\n"
		"\\toprule\n"
		"& N(mm) & E(mm) & U(mm) & PCC(mm) & & N(mm) & E(mm) & U(mm) & PCC(mm) \\\\\n"
		"\\midrule\n";

	struct DpcoDpcc {
		DpcoDpcc() : dpco(NAN, NAN, NAN), dpcc(NAN) {}
		can2::Point3d dpco;
		double dpcc;
	};

	struct LabData
	{
		std::string name;
		DpcoDpcc data[4];
	};

	std::vector<LabData> lds;

	for (int na = 0; na < (int)prn->m_ants.size(); na++)
	{
		can2::RingAntenna* pa = prn->m_ants[na].get();
		std::shared_ptr<can2::Node> pd(pref->subtract(pa));
		if (pa == pref)
			continue;
		can2::AntexAntenna* pda = (can2::AntexAntenna*)pd.get();
		if (!pda->isAntexAntenna())
			continue;

		LabData ld;
		ld.name = pa->getName();
		can2::Gnss::Signal sigs[4] = { can2::Gnss::GIFL2, can2::Gnss::RIFL2, can2::Gnss::EIFL5a, can2::Gnss::CIF0206 };
		for (int sys=0; sys<4; sys++)
		{
			double ro = 0;
			ld.data[sys].dpco = pda->calcOffset(sigs[sys], m_eleMask, can2::Node::eSinAndCos, &ro) * 1000;
			ld.data[sys].dpcc = pda->calcNorm(sigs[sys], can2::Node::eSinAndCos, true) * 1000;
		}
		lds.push_back(ld);
	}

	for (int row = 0; row < (int)lds.size() / 2; row++)
	{
		LabData& ld1 = lds[2 * row];
		LabData& ld2 = 2 * row + 1 < (int)lds.size() ? lds[2 * row + 1] : LabData();

		ofs << "\\\\ & & " << ld1.name << " & & & & & & " << ld2.name << " & \\\\" << std::endl;
		ofs << "\\cmidrule{2-5} \\cmidrule{7-10}" << std::endl;
		const char* szSys[4] = { "G", "R", "E", "C" };
		for (int sys = 0; sys < 4; sys++)
		{
			ofs << szSys[sys] << " & ";
			ofs << (_isnan(ld1.data[sys].dpco.n) ? "nan" : can2::stringFormat("%.1f", ld1.data[sys].dpco.n).c_str()) << " & ";
			ofs << (_isnan(ld1.data[sys].dpco.e) ? "nan" : can2::stringFormat("%.1f", ld1.data[sys].dpco.e).c_str()) << " & ";
			ofs << (_isnan(ld1.data[sys].dpco.u) ? "nan" : can2::stringFormat("%.1f", ld1.data[sys].dpco.u).c_str()) << " & ";
			ofs << (_isnan(ld1.data[sys].dpcc) ? "nan" : can2::stringFormat("%.1f", ld1.data[sys].dpcc).c_str()) << " & ";
			ofs << szSys[sys] << " & ";
			ofs << (_isnan(ld2.data[sys].dpco.n) ? "nan" : can2::stringFormat("%.1f", ld2.data[sys].dpco.n).c_str()) << " & ";
			ofs << (_isnan(ld2.data[sys].dpco.e) ? "nan" : can2::stringFormat("%.1f", ld2.data[sys].dpco.e).c_str()) << " & ";
			ofs << (_isnan(ld2.data[sys].dpco.u) ? "nan" : can2::stringFormat("%.1f", ld2.data[sys].dpco.u).c_str()) << " & ";
			ofs << (_isnan(ld2.data[sys].dpcc) ? "nan" : can2::stringFormat("%.1f", ld2.data[sys].dpcc).c_str()) << " \\\\\n";
		}
	}

	ofs << "\\bottomrule" << std::endl;
	ofs << "\\end{tabular}" << std::endl;
}


void CRingIFView::OnBnClickedButtonLoad()
{
	AfxMessageBox("Not implemented yet");
	/*
	CFileDialog dlg(TRUE, ".plt", m_strFileName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "Plot files (*.plt)|*.plt|All files (*.*)|*.*||", this);

	if (dlg.DoModal() != IDOK)
		return;
	m_strFileName = dlg.GetPathName();

	std::ifstream ifs(m_strFileName);
	if (!ifs.good())
	{
		AfxMessageBox("Can't open source file");
		return;
	}

	try
	{
		can2::Archive ar(ifs, can2::Archive::load);
		m_wndPlot.serialize(ar);
		m_wndPlot.Invalidate();
	}
	catch (std::exception e)
	{
		AfxMessageBox(e.what());
	}
	*/
}



void CRingIFView::OnBnClickedButtonSetPlotTitle()
{
	UpdateData(TRUE);
	CPromptTitleDlg dlg(this);
	dlg.m_strText = m_wndPlot.m_plotTitle.m_text.empty() ? can2::gl_settings.plot2d[0].title.c_str() : m_wndPlot.m_plotTitle.m_text.c_str();
	if (dlg.DoModal() != IDOK)
		return;

	can2::gl_settings.plot2d[0].title = dlg.m_strText;
	can2::gl_settings.save();

	m_wndPlot.addPlotTitle(dlg.m_strText);
	m_wndPlot.Invalidate();
	m_wndPlot.UpdateWindow();
}

void CRingIFView::OnSelchangeComboSignal()
{
	SetTimer(100, 100, NULL);
}

void CRingIFView::OnSelchangeComboRef()
{
	SetTimer(100, 100, NULL);
}

void CRingIFView::OnCurveFormat()
{
	//static COLORREF clrs[9] = { RGB(255, 0, 0), RGB(0, 200, 0), RGB(0, 0, 255),
	//	RGB(200, 0, 200), RGB(125, 125, 0), RGB(0, 255, 255),
	//	RGB(120, 0, 0), RGB(0, 120, 0), RGB(255, 128, 0) };

	std::vector<COLORREF> colors;
	for (int i = 0; i < can2::Plot2d::c_stdColorsNum; i++)
		colors.push_back(can2::Plot2d::getStdColor(i));

	CCurveFormatPg pgCurve;
	pgCurve.m_pPlot = &m_wndPlot;
	pgCurve.m_colors = colors;

	CPropertySheet dlg("Plot properties");


	dlg.AddPage(&pgCurve);

	if (dlg.DoModal() != IDOK)
		return;
	m_wndPlot.Invalidate();
	m_wndPlot.UpdateWindow();
}