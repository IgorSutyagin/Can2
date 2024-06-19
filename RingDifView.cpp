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
#include "can2.h"
#include "RingDifView.h"
#include "RingDoc.h"
#include "RingFrm.h"
#include "RingNode.h"
#include "RingBar.h"

// CRingDifView
CRingDoc* CRingDifView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CRingDoc)));
	return (CRingDoc*)m_pDocument;
}

CRingBar* CRingDifView::getRingBar() const
{
	CRingFrame* pFrame = (CRingFrame*)GetParentFrame();
	return pFrame->m_pBar;
}



IMPLEMENT_DYNCREATE(CRingDifView, CFormView)

CRingDifView::CRingDifView()
	: CFormView(CRingDifView::IDD)
	, m_yMax(1)
	, m_yMin(0)
	, m_xMin(0)
	, m_xMax(1)
	, m_byAuto(TRUE)
	, m_bxAuto(TRUE)
	, m_yDivs(10)
	, m_xDivs(10)
	, m_bClusterSpread (FALSE)
{
	m_bNeedInit = true;
	m_nRingOffsetMode = can2::AntexAntenna::eSinAndCos;
	m_bSimpleMode = TRUE;
	m_nWhatToPlot = 0;
}

CRingDifView::~CRingDifView()
{
}

void CRingDifView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CUSTOM_PLOT, m_wndPlot);
	DDX_Radio(pDX, IDC_RADIO_NOWEIGHT, m_nRingOffsetMode);
	DDX_Check(pDX, IDC_CHECK_SIMPLE, m_bSimpleMode);
	DDX_Radio(pDX, IDC_RADIO_PCO_PCV, m_nWhatToPlot);

	DDX_Text(pDX, IDC_EDIT_Y_MAX, m_yMax);
	DDX_Text(pDX, IDC_EDIT_Y_MIN, m_yMin);
	DDX_Text(pDX, IDC_EDIT_X_MIN, m_xMin);
	DDX_Text(pDX, IDC_EDIT_X_MAX, m_xMax);
	DDX_Check(pDX, IDC_CHECK_Y_AUTO, m_byAuto);
	DDX_Check(pDX, IDC_CHECK_X_AUTO, m_bxAuto);
	DDX_Text(pDX, IDC_EDIT_Y_DIVS, m_yDivs);
	DDX_Text(pDX, IDC_EDIT_X_DIVS, m_xDivs);
	DDX_Check(pDX, IDC_CHECK_CLUSTER_SPREAD, m_bClusterSpread);
}

BEGIN_MESSAGE_MAP(CRingDifView, CFormView)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_CHECK_SIMPLE, &CRingDifView::OnClickedRadioOffsetMode)
	ON_BN_CLICKED(IDC_CHECK_CLUSTER_SPREAD, &CRingDifView::OnClickedRadioOffsetMode)
	ON_BN_CLICKED(IDC_RADIO_NOWEIGHT, &CRingDifView::OnClickedRadioOffsetMode)
	ON_BN_CLICKED(IDC_RADIO_ONE, &CRingDifView::OnClickedRadioOffsetMode)
	ON_BN_CLICKED(IDC_RADIO_SINANDCOS, &CRingDifView::OnClickedRadioOffsetMode)
	ON_BN_CLICKED(IDC_RADIO_PCO_PCV, &CRingDifView::OnClickedRadioOffsetMode)
	ON_BN_CLICKED(IDC_RADIO_PCO, &CRingDifView::OnClickedRadioOffsetMode)
	ON_BN_CLICKED(IDC_RADIO_VPCO, &CRingDifView::OnClickedRadioOffsetMode)
	ON_BN_CLICKED(IDC_RADIO_HPCO, &CRingDifView::OnClickedRadioOffsetMode)
	ON_BN_CLICKED(IDC_RADIO_EAST, &CRingDifView::OnClickedRadioOffsetMode)
	ON_BN_CLICKED(IDC_RADIO_NORTH, &CRingDifView::OnClickedRadioOffsetMode)
	ON_WM_TIMER()

	ON_BN_CLICKED(IDC_CHECK_Y_AUTO, &CRingDifView::OnBnClickedCheckAuto)
	ON_BN_CLICKED(IDC_CHECK_X_AUTO, &CRingDifView::OnBnClickedCheckAuto)
	ON_EN_CHANGE(IDC_EDIT_X_MAX, &CRingDifView::OnChangeEditXMinMax)
	ON_EN_CHANGE(IDC_EDIT_X_MIN, &CRingDifView::OnChangeEditXMinMax)
	ON_EN_CHANGE(IDC_EDIT_X_DIVS, &CRingDifView::OnChangeEditXMinMax)
	ON_EN_CHANGE(IDC_EDIT_Y_MAX, &CRingDifView::OnChangeEditYMinMax)
	ON_EN_CHANGE(IDC_EDIT_Y_MIN, &CRingDifView::OnChangeEditYMinMax)
	ON_EN_CHANGE(IDC_EDIT_Y_DIVS, &CRingDifView::OnChangeEditYMinMax)
	ON_BN_CLICKED(IDC_BUTTON_ADD_COMMENT, &CRingDifView::OnBnClickedButtonAddComment)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_COMMENTS, &CRingDifView::OnBnClickedButtonRemoveComments)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CRingDifView::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, &CRingDifView::OnBnClickedButtonLoad)
END_MESSAGE_MAP()


// CRingDifView diagnostics

#ifdef _DEBUG
void CRingDifView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CRingDifView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CRingDifView message handlers
void CRingDifView::enableControls()
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

	//CRingDoc* pDoc = GetDocument();
	//CRingBar* pBar = getRingBar();
	//can2::RingNode* prn = pDoc->m_rn;
	//bool bCluster = prn->hasCluster();
	//GetDlgItem(IDC_CHECK_CLUSTER_SPREAD)->EnableWindow(bCluster);

	UpdateData(FALSE);
}


void CRingDifView::OnSize(UINT nType, int cx, int cy)
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
		if (nId == IDC_RADIO_EAST || nId == IDC_RADIO_NORTH || nId == IDC_RADIO_UP) //pChild->SendMessage(WM_GETDLGCODE) & DLGC_BUTTON)
		{
			//pChild->GetWindowRect(&r1); ScreenToClient(&r1);
			//r1.top += dy; r1.bottom += dy; r1.left += dx; r1.right += dx;
			//::DeferWindowPos(hDWP, pChild->m_hWnd, NULL,
			//	r1.left, r1.top, 0, 0,
			//	SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (nId == IDC_CUSTOM_PLOT)
		{
			pChild->GetWindowRect(&r1);
			ScreenToClient(r1);
			r1.right = m_rCrt.right - 20; // += dx;
			r1.bottom = m_rCrt.bottom - 60; // += dy;
			//r1.right += dx; r1.bottom += dy;
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
		else if (nID == IDC_BUTTON_ADD_COMMENT)
		{
			CRect r = r1;
			r.left = (rectPlot.left + rectPlot.right) / 2 + (rectPlot.right - rectPlot.left) / 4 - r1.Size().cx - 5;
			r.right = r.left + r1.Size().cx;
			r.top = rectPlot.bottom + 5;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (nID == IDC_BUTTON_REMOVE_COMMENTS)
		{
			CRect r = r1;
			r.left = (rectPlot.left + rectPlot.right) / 2 + (rectPlot.right - rectPlot.left) / 4 + 5;
			r.right = r.left + r1.Size().cx;
			r.top = rectPlot.bottom + 5;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}

	}

	::EndDeferWindowPos(hDWP);

}


void CRingDifView::OnInitialUpdate()
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
	m_font.CreateFontIndirect(&lf);

	m_wndPlot.initControl(this);
	m_wndPlot.setLegendFont(&m_font);
	m_wndPlot.setMenuIDs(IDR_POPUP, 0, NULL);

	enableControls();

	updateCurves();
}

void CRingDifView::updateCurves()
{
	CWaitCursor wc;

	if (!IsWindow(m_wndPlot.GetSafeHwnd()))
		return;

	UpdateData(TRUE);

	if (m_bClusterSpread)
	{
		updateSpreadCurves();
		return;
	}

	CRingDoc* pDoc = GetDocument();
	CRingBar* pBar = getRingBar();
	can2::RingNode* prn = pDoc->m_rn;

	COLORREF clrs[9] = { RGB(255, 0, 0), RGB(0, 200, 0), RGB(0, 0, 255),
		RGB(200, 0, 200), RGB(125, 125, 0), RGB(0, 200, 200),
		RGB(120, 0, 0), RGB(0, 120, 0), RGB(0, 0, 120) };

	m_wndPlot.removeAllCurves();
	m_wndPlot.setAxisTitle(TRUE, "Frequency (MHz)", FALSE);
	m_wndPlot.resetInitArea();

	int nCurve = 0;

	bool bSimpleMode = m_bSimpleMode ? true : false;
	prn->m_metrics.bSimpleMode = bSimpleMode;
	can2::AntexAntenna::OffsetMode em = m_nRingOffsetMode == 0 ? can2::AntexAntenna::eNoWeight :
		m_nRingOffsetMode == can2::AntexAntenna::eOnlyCos ? can2::AntexAntenna::eOnlyCos : can2::AntexAntenna::eSinAndCos;
	prn->m_metrics.em = em;

	if (m_nWhatToPlot == 0)
	{
		for (int nc0 = 0; nc0 < (int)prn->m_cls.size(); nc0++)
		{
			const can2::RingNode::Cluster& cls0 = prn->m_cls[nc0];
			can2::AntexAntenna* pa0 = cls0.atm.get();
			for (int nc1 = nc0 + 1; nc1 < (int)prn->m_cls.size(); nc1++)
			{
				const can2::RingNode::Cluster& cls1 = prn->m_cls[nc1];
				can2::AntexAntenna* pa1 = cls1.atm.get();

				can2::AntexAntenna* padif = (can2::AntexAntenna*)pa1->subtract(pa0);

				std::vector<can2::Point2d> pts;

				for (int nf = can2::Gnss::G01; nf < can2::Gnss::esigInvalid; nf++)
				{
					can2::Gnss::Signal eft = (can2::Gnss::Signal)nf;

					if (!padif->hasPcc(eft))
						continue;

					double d = padif->calcNorm(eft, prn->m_metrics.em, prn->m_metrics.bSimpleMode);
					double f = can2::Gnss::getSysFreq(eft) / 1000000.0;
					pts.push_back(can2::Point2d(f, d * 1000.0));
				}

				std::sort(pts.begin(), pts.end(), [](const can2::Point2d& a, const can2::Point2d& b) {
					return a.x < b.x;
				});

				if (!m_bClusterSpread)
				{
					CString str;
					str.Format("%s - %s", pa0->getName().c_str(), pa1->getName().c_str());
					m_wndPlot.addCurve(nCurve + 1, str, pts, pBar->isSelected(nc0, nc1) ? 4 : 2, can2::Curve2d::eLineAndPoints, clrs[nCurve % 9]);
					nCurve++;
				}
				delete padif;
			}
		}
		m_wndPlot.setAxisTitle(FALSE, "Delta PCC scalar metrics (mm)", FALSE);
	}
	else
	{
		std::vector<std::map<can2::Gnss::Signal, can2::Point3d>> offs;
		for (int nc = 0; nc < (int)prn->m_cls.size(); nc++)
		{
			const can2::RingNode::Cluster& cls = prn->m_cls[nc];
			can2::AntexAntenna* pa = cls.atm.get();

			std::map<can2::Gnss::Signal, can2::Point3d> antOffs;
			for (int nf = can2::Gnss::G01; nf < can2::Gnss::esigInvalid; nf++)
			{
				can2::Gnss::Signal eft = (can2::Gnss::Signal)nf;

				if (!pa->hasPcc(eft))
					continue;

				can2::Point3d off = pa->calcOffset(eft, 0.0, prn->m_metrics.em);
				if (!off.isValid())
					continue;
				antOffs[eft] = off;
			}
			offs.push_back(antOffs);
		}

		for (int nc0 = 0; nc0 < (int)prn->m_cls.size(); nc0++)
		{
			const can2::RingNode::Cluster& cls0 = prn->m_cls[nc0];
			can2::AntexAntenna* pa0 = cls0.atm.get();
			std::map<can2::Gnss::Signal, can2::Point3d>& a0 = offs[nc0];
			for (int nc1 = nc0 + 1; nc1 < (int)prn->m_cls.size(); nc1++)
			{
				const can2::RingNode::Cluster& cls1 = prn->m_cls[nc1];
				can2::AntexAntenna* pa1 = cls1.atm.get();
				std::map <can2::Gnss::Signal, can2::Point3d >& a1 = offs[nc1];

				std::vector<can2::Point2d> pts;

				for (int nf = can2::Gnss::G01; nf < can2::Gnss::esigInvalid; nf++)
				{
					can2::Gnss::Signal eft = (can2::Gnss::Signal)nf;

					if (a0.find(eft) == a0.end() || a1.find(eft) == a1.end())
						continue;

					can2::Point3d off0 = a0[eft];
					can2::Point3d off1 = a1[eft];
					double val = 0;
					if (m_nWhatToPlot == 1)
						val = can2::Point3d(off0 - off1).rad();
					else if (m_nWhatToPlot == 2)
						val = (off0.z - off1.z);
					else if (m_nWhatToPlot == 3)
						val = sqrt((off0.x - off1.x) * (off0.x - off1.x) + (off0.y - off1.y) * (off0.y - off1.y));
					else if (m_nWhatToPlot == 4)
						val = off0.x - off1.x;
					else if (m_nWhatToPlot == 5)
						val = off0.y - off1.y;
					pts.push_back(can2::Point2d(can2::Gnss::getSysFreq(eft) / 1000000.0, val * 1000.0));
				}

				std::sort(pts.begin(), pts.end(), [](const can2::Point2d& a, const can2::Point2d& b) {
					return a.x < b.x;
				});

				CString str;
				str.Format("%s - %s", pa0->getName().c_str(), pa1->getName().c_str());
				m_wndPlot.addCurve(nCurve + 1, str, pts, pBar->isSelected(nc0, nc1) ? 4 : 2, can2::Curve2d::eLineAndPoints, clrs[nCurve % 9]);
				nCurve++;
			}
		}

		const char* szTitles[5] = { "PCO difference (mm)", "VPCO difference (mm)", "HPCO difference (mm)", "PCO East difference (mm)", "PCO North difference (mm)" };
		if (0 < m_nWhatToPlot && m_nWhatToPlot < 5)
			m_wndPlot.setAxisTitle(FALSE, szTitles[m_nWhatToPlot - 1], FALSE);
		else
			m_wndPlot.setAxisTitle(FALSE, "PCO difference (mm)", FALSE);
	}

	for (int nb = 0; nb < can2::Gnss::ebMax; nb++)
	{
		double fmin, fmax;
		can2::Gnss::getBand((can2::Gnss::Band)nb, fmin, fmax);
		m_wndPlot.addArea(nb + 1, fmin / 1000000, fmax / 1000000, can2::Gnss::getBandColor(nb), can2::Gnss::getBandName(nb));
	}

	m_wndPlot.setInitArea();
	m_wndPlot.Invalidate();
	m_wndPlot.UpdateWindow();
}

void getClusterSpread(std::vector<can2::RingAntenna*>& ras, int nWhatToPlot, const can2::RingNode::Metrics& m, std::map<double, can2::Point2d>& mapSpread)
{
	if (nWhatToPlot == 0)
	{
		for (int nc0 = 0; nc0 < (int)ras.size(); nc0++)
		{
			const can2::RingAntenna * pa0 = ras[nc0];
			for (int nc1 = nc0 + 1; nc1 < (int)ras.size(); nc1++)
			{
				can2::RingAntenna* pa1 = ras[nc1];

				can2::AntexAntenna* padif = (can2::AntexAntenna*)pa1->subtract(pa0);

				for (int nf = can2::Gnss::G01; nf < can2::Gnss::esigInvalid; nf++)
				{
					can2::Gnss::Signal eft = (can2::Gnss::Signal)nf;

					if (!padif->hasPcc(eft))
						continue;

					double d = padif->calcNorm(eft, m.em, m.bSimpleMode);
					double f = can2::Gnss::getSysFreq(eft) / 1000000.0;
					if (mapSpread.find(f) != mapSpread.end())
					{
						if (mapSpread[f].x > d * 1000)
							mapSpread[f].x = d * 1000;
						if (mapSpread[f].y < d * 1000)
							mapSpread[f].y = d * 1000;
					}
					else
					{
						mapSpread[f] = can2::Point2d(d * 1000, d * 1000);
					}
				}

				delete padif;
			}
		}
	}
	else
	{
		std::vector<std::map<can2::Gnss::Signal, can2::Point3d>> offs;
		for (int nc = 0; nc < (int)ras.size(); nc++)
		{
			can2::RingAntenna* pa = ras[nc];

			std::map<can2::Gnss::Signal, can2::Point3d> antOffs;
			for (int nf = can2::Gnss::G01; nf < can2::Gnss::esigInvalid; nf++)
			{
				can2::Gnss::Signal eft = (can2::Gnss::Signal)nf;

				if (!pa->hasPcc(eft))
					continue;

				can2::Point3d off = pa->calcOffset(eft, 0.0, m.em);
				if (!off.isValid())
					continue;
				antOffs[eft] = off;
			}
			offs.push_back(antOffs);
		}

		for (int nc0 = 0; nc0 < (int)ras.size(); nc0++)
		{
			can2::RingAntenna* pa0 = ras[nc0];
			std::map<can2::Gnss::Signal, can2::Point3d>& a0 = offs[nc0];
			for (int nc1 = nc0 + 1; nc1 < (int)ras.size(); nc1++)
			{
				can2::RingAntenna* pa1 = ras[nc1];
				std::map <can2::Gnss::Signal, can2::Point3d >& a1 = offs[nc1];

				for (int nf = can2::Gnss::G01; nf < can2::Gnss::esigInvalid; nf++)
				{
					can2::Gnss::Signal eft = (can2::Gnss::Signal)nf;

					if (a0.find(eft) == a0.end() || a1.find(eft) == a1.end())
						continue;

					can2::Point3d off0 = a0[eft];
					can2::Point3d off1 = a1[eft];
					double val = 0;
					if (nWhatToPlot == 1)
						val = can2::Point3d(off0 - off1).rad();
					else if (nWhatToPlot == 2)
						val = (off0.z - off1.z);
					else if (nWhatToPlot == 3)
						val = sqrt((off0.x - off1.x) * (off0.x - off1.x) + (off0.y - off1.y) * (off0.y - off1.y));
					else if (nWhatToPlot == 4)
						val = off0.x - off1.x;
					else if (nWhatToPlot == 5)
						val = off0.y - off1.y;
					val *= 1000;
					double f = can2::Gnss::getSysFreq(eft) / 1000000.0;
					if (mapSpread.find(f) != mapSpread.end())
					{
						if (mapSpread[f].x > val)
							mapSpread[f].x = val;
						if (mapSpread[f].y < val)
							mapSpread[f].y = val;
					}
					else
					{
						mapSpread[f] = can2::Point2d(val, val);
					}
				}
			}
		}
	}
}

void CRingDifView::updateSpreadCurves()
{
	CRingDoc* pDoc = GetDocument();
	CRingBar* pBar = getRingBar();
	can2::RingNode* prn = pDoc->m_rn;
	std::map<double, can2::Point2d> mapSpread; // Frequency to (min,max) spread map
	bool bHasCluster = prn->hasCluster();


	COLORREF clrs[9] = { RGB(255, 0, 0), RGB(0, 200, 0), RGB(0, 0, 255),
		RGB(200, 0, 200), RGB(125, 125, 0), RGB(0, 200, 200),
		RGB(120, 0, 0), RGB(0, 120, 0), RGB(0, 0, 120) };

	m_wndPlot.removeAllCurves();
	m_wndPlot.setAxisTitle(TRUE, "Frequency (MHz)", FALSE);
	m_wndPlot.resetInitArea();

	int nCurve = 0;

	bool bSimpleMode = m_bSimpleMode ? true : false;
	prn->m_metrics.bSimpleMode = bSimpleMode;
	can2::AntexAntenna::OffsetMode em = m_nRingOffsetMode == 0 ? can2::AntexAntenna::eNoWeight :
		m_nRingOffsetMode == can2::AntexAntenna::eOnlyCos ? can2::AntexAntenna::eOnlyCos : can2::AntexAntenna::eSinAndCos;
	prn->m_metrics.em = em;

	CCan2App* pApp = getCan2App();
	int nWidth = 1; // (int)ceil(pApp->m_sp.pixInPoint * 3);

	if (bHasCluster)
	{
		for (int nc = 0; nc < (int)prn->m_cls.size(); nc++)
		{
			can2::RingNode::Cluster& c = prn->m_cls[nc];
			if (c.ants.size() < 2)
				continue;

			std::map<double, can2::Point2d> mapSpread;
			std::vector <can2::RingAntenna*> ras;
			for (int i = 0; i < (int)c.ants.size(); i++)
			{
				ras.push_back(c.ants[i].get());
			}
			getClusterSpread(ras, m_nWhatToPlot, prn->m_metrics, mapSpread);

			std::vector <can2::Point2d> pts;
			std::vector <can2::Point2d> ptMinMax;

			for (auto it = mapSpread.begin(); it != mapSpread.end(); it++)
			{
				double f = it->first;
				can2::Point2d minMax = it->second;
				pts.push_back(can2::Point2d(f, (minMax.x + minMax.y) / 2));
				ptMinMax.push_back(minMax);
			}

			m_wndPlot.addBand(nCurve + 1, c.atm->getName().c_str(), pts, ptMinMax, nWidth, clrs[nCurve]);
		}
	}
	else
	{
		std::map<double, can2::Point2d> mapSpread;
		std::vector <can2::RingAntenna*> ras;
		for (int i = 0; i < (int)prn->m_cls.size(); i++)
		{
			ras.push_back(prn->m_cls[i].atm.get());
		}
		getClusterSpread(ras, m_nWhatToPlot, prn->m_metrics, mapSpread);

		std::vector <can2::Point2d> pts;
		std::vector <can2::Point2d> ptMinMax;

		for (auto it = mapSpread.begin(); it != mapSpread.end(); it++)
		{
			double f = it->first;
			can2::Point2d minMax = it->second;
			pts.push_back(can2::Point2d(f, (minMax.x + minMax.y) / 2));
			ptMinMax.push_back(minMax);
		}

		m_wndPlot.addBand(nCurve + 1, "", pts, ptMinMax, nWidth, clrs[nCurve]);
	}

	if (m_nWhatToPlot == 0)
	{
		m_wndPlot.setAxisTitle(FALSE, "Delta PCC scalar metrics (mm)", FALSE);
	}
	else
	{
		const char* szTitles[5] = { "PCO difference (mm)", "VPCO difference (mm)", "HPCO difference (mm)", "PCO East difference (mm)", "PCO North difference (mm)" };
		if (0 < m_nWhatToPlot && m_nWhatToPlot < 5)
			m_wndPlot.setAxisTitle(FALSE, szTitles[m_nWhatToPlot - 1], FALSE);
		else
			m_wndPlot.setAxisTitle(FALSE, "PCO difference (mm)", FALSE);
	}

	for (int nb = 0; nb < can2::Gnss::ebMax; nb++)
	{
		double fmin, fmax;
		can2::Gnss::getBand((can2::Gnss::Band)nb, fmin, fmax);
		m_wndPlot.addArea(nb + 1, fmin / 1000000, fmax / 1000000, can2::Gnss::getBandColor(nb), can2::Gnss::getBandName(nb));
	}

	m_wndPlot.setInitArea();
	m_wndPlot.Invalidate();
	m_wndPlot.UpdateWindow();
}

void CRingDifView::OnClickedRadioOffsetMode()
{
	SetTimer(100, 1, NULL);
}


void CRingDifView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 100)
	{
		KillTimer(nIDEvent);
		updateCurves();
	}
	if (nIDEvent == 1)
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

	CFormView::OnTimer(nIDEvent);
}

void CRingDifView::OnBnClickedCheckAuto()
{
	SetTimer(1, 10, NULL);
}

void CRingDifView::OnChangeEditXMinMax()
{
	KillTimer(3);
	SetTimer(3, 2000, NULL);
}


void CRingDifView::OnChangeEditYMinMax()
{
	KillTimer(4);
	SetTimer(4, 2000, NULL);
}


void CRingDifView::OnBnClickedButtonAddComment()
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
}


void CRingDifView::OnBnClickedButtonRemoveComments()
{
	m_wndPlot.deleteInscription(0);
}


void CRingDifView::OnBnClickedButtonSave()
{
	AfxMessageBox("Not implemented yet");
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
}


void CRingDifView::OnBnClickedButtonLoad()
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

