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
#include "RingPcoView.h"
#include "PromptTitleDlg.h"
#include "Settings.h"
#include "CurveFormatPg.h"

// CRingPcoView
CRingDoc* CRingPcoView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CRingDoc)));
	return (CRingDoc*)m_pDocument;
}


IMPLEMENT_DYNCREATE(CRingPcoView, CFormView)

CRingPcoView::CRingPcoView()
	: CFormView(CRingPcoView::IDD)
	, m_nCoord(2)
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

CRingPcoView::~CRingPcoView()
{
}

void CRingPcoView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CUSTOM_PLOT, m_wndPlot);
	DDX_Radio(pDX, IDC_RADIO_EAST, m_nCoord);
	DDX_Text(pDX, IDC_EDIT_ELE_MASK, m_eleMask);
	DDX_Check(pDX, IDC_CHECK_OFFSET_FROM_ANTEX, m_bOffsetFromAntex);
	DDX_Check(pDX, IDC_CHECK_MARKER, m_bMarker);

	DDX_Text(pDX, IDC_EDIT_Y_MAX, m_yMax);
	DDX_Text(pDX, IDC_EDIT_Y_MIN, m_yMin);
	DDX_Text(pDX, IDC_EDIT_X_MIN, m_xMin);
	DDX_Text(pDX, IDC_EDIT_X_MAX, m_xMax);
	DDX_Check(pDX, IDC_CHECK_Y_AUTO, m_byAuto);
	DDX_Check(pDX, IDC_CHECK_X_AUTO, m_bxAuto);
	DDX_Text(pDX, IDC_EDIT_Y_DIVS, m_yDivs);
	DDX_Text(pDX, IDC_EDIT_X_DIVS, m_xDivs);
}

BEGIN_MESSAGE_MAP(CRingPcoView, CFormView)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_RADIO_EAST, &CRingPcoView::OnClickedRadioEast)
	ON_COMMAND(IDC_RADIO_NORTH, &CRingPcoView::OnRadioNorth)
	ON_COMMAND(IDC_RADIO_UP, &CRingPcoView::OnRadioUp)
	ON_BN_CLICKED(IDC_RADIO_HPCO, &CRingPcoView::OnClickedRadioEast)
	ON_BN_CLICKED(IDC_RADIO_R, &CRingPcoView::OnRadioUp)
	ON_EN_CHANGE(IDC_EDIT_ELE_MASK, &CRingPcoView::OnChangeEditEleMask)
	ON_WM_TIMER()

	ON_BN_CLICKED(IDC_CHECK_Y_AUTO, &CRingPcoView::OnBnClickedCheckAuto)
	ON_BN_CLICKED(IDC_CHECK_X_AUTO, &CRingPcoView::OnBnClickedCheckAuto)
	ON_EN_CHANGE(IDC_EDIT_X_MAX, &CRingPcoView::OnChangeEditXMinMax)
	ON_EN_CHANGE(IDC_EDIT_X_MIN, &CRingPcoView::OnChangeEditXMinMax)
	ON_EN_CHANGE(IDC_EDIT_X_DIVS, &CRingPcoView::OnChangeEditXMinMax)
	ON_EN_CHANGE(IDC_EDIT_Y_MAX, &CRingPcoView::OnChangeEditYMinMax)
	ON_EN_CHANGE(IDC_EDIT_Y_MIN, &CRingPcoView::OnChangeEditYMinMax)
	ON_EN_CHANGE(IDC_EDIT_Y_DIVS, &CRingPcoView::OnChangeEditYMinMax)
	ON_BN_CLICKED(IDC_BUTTON_ADD_COMMENT, &CRingPcoView::OnBnClickedButtonAddComment)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_COMMENTS, &CRingPcoView::OnBnClickedButtonRemoveComments)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CRingPcoView::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, &CRingPcoView::OnBnClickedButtonLoad)

	ON_BN_CLICKED(IDC_BUTTON_SET_PLOT_TITLE, &CRingPcoView::OnBnClickedButtonSetPlotTitle)
	ON_BN_CLICKED(IDC_CHECK_OFFSET_FROM_ANTEX, &CRingPcoView::OnBnClickedOffsetFromAntex)
	ON_BN_CLICKED(IDC_CHECK_MARKER, &CRingPcoView::OnBnClickedOffsetFromAntex)

	ON_COMMAND (ID_PLOT2D_CURVEFORMAT, &OnCurveFormat)
END_MESSAGE_MAP()


// CRingPcoView diagnostics

#ifdef _DEBUG
void CRingPcoView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CRingPcoView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

void CRingPcoView::enableControls()
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


// CRingPcoView message handlers


void CRingPcoView::OnSize(UINT nType, int cx, int cy)
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


void CRingPcoView::OnInitialUpdate()
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

	updateCurves();
}

void CRingPcoView::updateCurves()
{
	CWaitCursor wc;

	if (!IsWindow(m_wndPlot.GetSafeHwnd()))
		return;

	UpdateData(TRUE);
	CRingDoc* pDoc = GetDocument();
	can2::RingNode* prn = pDoc->m_rn;
	can2::Gnss::Signal sigSel = pDoc->m_signal;

	//COLORREF clrs[9] = { RGB(255, 0, 0), RGB(0, 200, 0), RGB(0, 0, 255),
	//	RGB(200, 0, 200), RGB(125, 125, 0), RGB(0, 255, 255),
	//	RGB(120, 0, 0), RGB(0, 120, 0), RGB(255, 128, 120) };

	m_wndPlot.removeAllCurves();
	m_wndPlot.removeAllCircles();
	m_wndPlot.clearMarkers();

	m_wndPlot.setAxisTitle(TRUE, m_nCoord <= 2 ? "Frequency (MHz)" : "East (mm)", FALSE);
	CString str;
	str.Format("%s (mm)", m_nCoord == 0 ? "East" : m_nCoord == 1 ? "North" : m_nCoord == 2 ? "VPCO" : m_nCoord == 3 ? "North" : "R");
	m_wndPlot.setAxisTitle(FALSE, str, FALSE);
	m_wndPlot.resetInitArea();

	can2::Point3d ptMax(-DBL_MAX, -DBL_MAX, -DBL_MAX);
	can2::Point3d ptMin(DBL_MAX, DBL_MAX, DBL_MAX);
	double roMin = DBL_MAX;
	double roMax = -DBL_MAX;
	for (int nc = 0; nc < (int)prn->m_cls.size(); nc++)
	{
		const can2::RingNode::Cluster& cls = prn->m_cls[nc];
		can2::RingAntenna* pa = cls.atm.get();
		bool bCluster = cls.ants.size() > 1;

		COLORREF clr = can2::Plot2d::getStdColor(nc); // clrs[nc % 9];
		if (can2::gl_settings.plot2dColors.find(pa->getName()) != can2::gl_settings.plot2dColors.end())
		{
			clr = can2::gl_settings.plot2dColors[pa->getName()];
		}

		std::vector<can2::Point2d> pts;
		std::vector<can2::Point3d> ptMinMax;
		std::vector<can2::Point3d> ptHs;

		for (int nf = can2::Gnss::G01; nf < can2::Gnss::esigInvalid; nf++)
		{
			can2::Gnss::Signal eft = (can2::Gnss::Signal)nf;

			if (!pa->hasPcc(eft))
				continue;

			//CF3DPoint off = pa->calcOffset(sys, slot, m_eleMask);
			double ro = 0;
			can2::Point3d off = m_bOffsetFromAntex ? pa->getOffset(eft, &ro) : pa->calcOffset(eft, m_eleMask, prn->m_metrics.em, &ro);
			can2::Point3d ptMinC;
			can2::Point3d ptMaxC;
			double roMinC = 0;
			double roMaxC = 0;
			if (bCluster)
			{
				cls.getMinMaxOff(ptMinC, ptMaxC, roMinC, roMaxC, eft, m_eleMask, prn->m_metrics.em);
				if (off.x < ptMinC.x)
					ptMinC.x = off.x;
				if (off.y < ptMinC.y)
					ptMinC.y = off.y;
				if (off.z < ptMinC.z)
					ptMinC.z = off.z;
				if (off.x > ptMaxC.x)
					ptMaxC.x = off.x;
				if (off.y > ptMaxC.y)
					ptMaxC.y = off.y;
				if (off.z > ptMaxC.z)
					ptMaxC.z = off.z;
				ptMinC *= 1000;
				ptMaxC *= 1000;
				if (ro < roMinC)
					roMinC = ro;
				if (ro > roMaxC)
					roMaxC = ro;
				roMinC *= 1000;
				roMaxC *= 1000;
			}

			if (!off.isValid())
				continue;
			off *= 1000;
			ro *= 1000;
			if (m_nCoord <= 2)
			{
				pts.push_back(can2::Point2d(can2::Gnss::getSysFreq(eft) / 1000000.0, off[m_nCoord]));
				ptMinMax.push_back(can2::Point3d(can2::Gnss::getSysFreq(eft) / 1000000.0, ptMinC[m_nCoord], ptMaxC[m_nCoord]));
			}
			else if (m_nCoord == 3)
			{
				ptHs.push_back(can2::Point3d(can2::Gnss::getSysFreq(eft) / 1000000.0, off.e, off.n));
			}
			else // if (m_nCoord == 4)
			{
				pts.push_back(can2::Point2d(can2::Gnss::getSysFreq(eft) / 1000000.0, ro));
				ptMinMax.push_back(can2::Point3d(can2::Gnss::getSysFreq(eft) / 1000000.0, roMinC, roMaxC));
			}


			for (int i = 0; i < 3; i++)
			{
				if (off[i] < ptMin[i])
					ptMin[i] = off[i];
				if (off[i] > ptMax[i])
					ptMax[i] = off[i];
			}
			if (ro < roMin)
				roMin = ro;
			if (ro > roMax)
				roMax = ro;
		}

		if (m_nCoord <= 2 || m_nCoord == 4)
		{
			std::sort(pts.begin(), pts.end(), [](const can2::Point2d& a, const can2::Point2d& b) {
				return a.x < b.x;
			});
			std::sort(ptMinMax.begin(), ptMinMax.end(), [](const can2::Point3d& a, const can2::Point3d& b) {
				return a.x < b.x;
			});
		}
		else if (m_nCoord == 3)
		{
			if (sigSel != can2::Gnss::esigInvalid)
			{
				double fsel = can2::Gnss::getSysFreq(sigSel) / 1000000.0;
				std::vector<can2::Point3d>::iterator it = std::find_if(ptHs.begin(), ptHs.end(), [&](const can2::Point3d& pt) {
					return pt.x == fsel;
				});
				if (it != ptHs.end())
				{
					m_wndPlot.addCircle(nc+10, it->y, it->z, 0.1, can2::Plot2d::getStdColor(nc), 2);
				}
			}

			std::sort(ptHs.begin(), ptHs.end(), [](const can2::Point3d& a, const can2::Point3d& b) {
				return a.x < b.x;
			});
			pts.resize(ptHs.size());
			for (int i = 0; i < (int)ptHs.size(); i++)
				pts[i] = can2::Point2d(ptHs[i].y, ptHs[i].z);
		}

		if (!bCluster)
		{
			CCan2App* pApp = getCan2App();
			int nWidth = (int)ceil(pApp->m_sp.pixInPoint * 3);
			m_wndPlot.addCurve(nc + 1, pa->getName().c_str(), pts, nWidth, can2::Curve2d::eLineAndPoints, clr);
		}
		else
		{
			if (m_nCoord <= 2 || m_nCoord == 4)
			{
				std::vector<can2::Point2d> ptMinMax2d;
				for (int i = 0; i < (int)ptMinMax.size(); i++)
				{
					ptMinMax2d.push_back(can2::Point2d(ptMinMax[i].y, ptMinMax[i].z));
				}
				m_wndPlot.addBand(nc + 1, pa->getName().c_str(), pts, ptMinMax2d, 1, clr); // clrs[nc % 9]);
			}
			else
			{
				m_wndPlot.addCurve(nc + 1, pa->getName().c_str(), pts, 2, can2::Curve2d::eLineAndPoints, clr);
				
			}
		}

		can2::Curve2d* pCurve = m_wndPlot.getCurve(nc + 1);
		if (pCurve != nullptr)
			pCurve->m_bDirection = m_nCoord > 2;
	}

	if (m_nCoord <= 2 || m_nCoord == 4)
	{
		for (int nb = 0; nb < can2::Gnss::ebS; nb++)
		{
			double fmin, fmax;
			can2::Gnss::getBand((can2::Gnss::Band)nb, fmin, fmax);
			m_wndPlot.addArea(nb + 1, fmin / 1000000, fmax / 1000000, can2::Gnss::getBandColor(nb), can2::Gnss::getBandName(nb));
		}

		if (m_bxAuto)
		{
			m_wndPlot.setAxis(TRUE, 1100, 1700, 6);
		}
		else
		{
			m_wndPlot.setAxis(TRUE, m_xMin, m_xMax, m_xDivs);
		}

		if (m_byAuto)
		{
			m_wndPlot.autoScale(FALSE, TRUE, TRUE);
		}
		else
		{
			m_wndPlot.setAxis(FALSE, m_yMin, m_yMax, m_yDivs);
		}

		if (m_bMarker && sigSel != can2::Gnss::esigInvalid)
		{
			double fsel = can2::Gnss::getSysFreq(sigSel) / 1000000.0;
			m_wndPlot.setMarker(1, fsel);
		}
		else
			m_wndPlot.clearMarkers();
	}
	else
	{
		double range = std::max(std::max(fabs(ptMin.x), fabs(ptMax.x)), std::max(fabs(ptMin.y), fabs(ptMax.y)));
		range = ceil(range);
		int nDivs = (int)(range * 2);
		//m_wndPlot.AutoScale(TRUE, TRUE, TRUE);
		m_wndPlot.setAxis(TRUE, -range, range, nDivs);
		m_wndPlot.setAxis(FALSE, -range, range, nDivs);
		m_wndPlot.addCircle(1, 0, 0, 1);
	}

	m_wndPlot.setInitArea();
	m_wndPlot.Invalidate();
	m_wndPlot.UpdateWindow();
}

void CRingPcoView::OnClickedRadioEast()
{
	SetTimer(100, 1, NULL);
}


void CRingPcoView::OnRadioNorth()
{
	SetTimer(100, 1, NULL);
}


void CRingPcoView::OnRadioUp()
{
	SetTimer(100, 1, NULL);
}


void CRingPcoView::OnChangeEditEleMask()
{
	SetTimer(100, 1, NULL);
}


void CRingPcoView::OnTimer(UINT_PTR nIDEvent)
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

	CFormView::OnTimer(nIDEvent);
}

void CRingPcoView::OnBnClickedCheckAuto()
{
	KillTimer(1);
	SetTimer(1, 10, NULL);
}

void CRingPcoView::OnChangeEditXMinMax()
{
	KillTimer(3);
	SetTimer(3, 2000, NULL);
}


void CRingPcoView::OnChangeEditYMinMax()
{
	KillTimer(4);
	SetTimer(4, 2000, NULL);
}

void CRingPcoView::OnBnClickedButtonAddComment()
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


void CRingPcoView::OnBnClickedButtonRemoveComments()
{
	m_wndPlot.deleteInscription(0);
}


void CRingPcoView::OnBnClickedButtonSave()
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


void CRingPcoView::OnBnClickedButtonLoad()
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



void CRingPcoView::OnBnClickedButtonSetPlotTitle()
{
	UpdateData(TRUE);
	CPromptTitleDlg dlg(this);
	dlg.m_strText = m_wndPlot.m_plotTitle.m_text.empty() ? can2::gl_settings.plot2d[m_nCoord].title.c_str() : m_wndPlot.m_plotTitle.m_text.c_str();
	if (dlg.DoModal() != IDOK)
		return;

	can2::gl_settings.plot2d[m_nCoord].title = dlg.m_strText;
	can2::gl_settings.save();

	m_wndPlot.addPlotTitle(dlg.m_strText);
	m_wndPlot.Invalidate();
	m_wndPlot.UpdateWindow();
}


void CRingPcoView::OnBnClickedOffsetFromAntex()
{
	SetTimer(100, 1, NULL);
}

void CRingPcoView::OnCurveFormat()
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