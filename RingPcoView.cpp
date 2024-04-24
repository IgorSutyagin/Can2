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
}

BEGIN_MESSAGE_MAP(CRingPcoView, CFormView)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_RADIO_EAST, &CRingPcoView::OnClickedRadioEast)
	ON_COMMAND(IDC_RADIO_NORTH, &CRingPcoView::OnRadioNorth)
	ON_COMMAND(IDC_RADIO_UP, &CRingPcoView::OnRadioUp)
	ON_BN_CLICKED(IDC_RADIO_HPCO, &CRingPcoView::OnClickedRadioEast)
	ON_EN_CHANGE(IDC_EDIT_ELE_MASK, &CRingPcoView::OnChangeEditEleMask)
	ON_WM_TIMER()
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
			r1.bottom = m_rCrt.bottom - 30; // += dy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, 0, 0,
				r1.Width(), r1.Height(),
				SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
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
	m_font.CreateFontIndirect(&lf);

	m_wndPlot.initControl(this);
	m_wndPlot.setLegendFont(&m_font);
	m_wndPlot.setMenuIDs(IDR_POPUP, 0, NULL);

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

	COLORREF clrs[9] = { RGB(255, 0, 0), RGB(0, 200, 0), RGB(0, 0, 255),
		RGB(200, 0, 200), RGB(125, 125, 0), RGB(0, 200, 200),
		RGB(120, 0, 0), RGB(0, 120, 0), RGB(0, 0, 120) };

	m_wndPlot.removeAllCurves();
	m_wndPlot.removeAllCircles();
	m_wndPlot.clearMarkers();

	m_wndPlot.setAxisTitle(TRUE, m_nCoord <= 2 ? "Frequency (MHz)" : "East (mm)", FALSE);
	CString str;
	str.Format("%s (mm)", m_nCoord == 0 ? "East" : m_nCoord == 1 ? "North" : m_nCoord == 2 ? "Up" : "North");
	m_wndPlot.setAxisTitle(FALSE, str, FALSE);
	m_wndPlot.resetInitArea();

	can2::Point3d ptMax(-DBL_MAX, -DBL_MAX, -DBL_MAX);
	can2::Point3d ptMin(DBL_MAX, DBL_MAX, DBL_MAX);
	for (int nc = 0; nc < (int)prn->m_cls.size(); nc++)
	{
		const can2::RingNode::Cluster& cls = prn->m_cls[nc];
		can2::AntexAntenna* pa = cls.atm.get();

		std::vector<can2::Point2d> pts;
		std::vector<can2::Point3d> ptHs;

		for (int nf = can2::Gnss::G01; nf < can2::Gnss::esigInvalid; nf++)
		{
			can2::Gnss::Signal eft = (can2::Gnss::Signal)nf;

			if (!pa->hasPcc(eft))
				continue;

			//CF3DPoint off = pa->calcOffset(sys, slot, m_eleMask);
			can2::Point3d off = pa->calcOffset(eft, m_eleMask, prn->m_metrics.em);
			if (!off.isValid())
				continue;
			off *= 1000;
			if (m_nCoord <= 2)
				pts.push_back(can2::Point2d(can2::Gnss::getSysFreq(eft) / 1000000.0, off[m_nCoord]));
			else
				ptHs.push_back(can2::Point3d(can2::Gnss::getSysFreq(eft) / 1000000.0, off.e, off.n));

			for (int i = 0; i < 3; i++)
			{
				if (off[i] < ptMin[i])
					ptMin[i] = off[i];
				if (off[i] > ptMax[i])
					ptMax[i] = off[i];
			}
		}

		if (m_nCoord <= 2)
		{
			std::sort(pts.begin(), pts.end(), [](const can2::Point2d& a, const can2::Point2d& b) {
				return a.x < b.x;
			});
		}
		else
		{
			if (sigSel != can2::Gnss::esigInvalid)
			{
				double fsel = can2::Gnss::getSysFreq(sigSel) / 1000000.0;
				std::vector<can2::Point3d>::iterator it = std::find_if(ptHs.begin(), ptHs.end(), [&](const can2::Point3d& pt) {
					return pt.x == fsel;
				});
				if (it != ptHs.end())
				{
					m_wndPlot.addCircle(nc+10, it->y, it->z, 0.1, clrs[nc % 9], 2);
				}
			}

			std::sort(ptHs.begin(), ptHs.end(), [](const can2::Point3d& a, const can2::Point3d& b) {
				return a.x < b.x;
			});
			pts.resize(ptHs.size());
			for (int i = 0; i < (int)ptHs.size(); i++)
				pts[i] = can2::Point2d(ptHs[i].y, ptHs[i].z);
		}

		m_wndPlot.addCurve(nc + 1, pa->getName().c_str(), pts, 2, can2::Curve2d::eLineAndPoints, clrs[nc % 9]);

		can2::Curve2d* pCurve = m_wndPlot.getCurve(nc + 1);
		if (pCurve != nullptr)
			pCurve->m_bDirection = m_nCoord > 2;
	}

	if (m_nCoord <= 2)
	{
		m_wndPlot.setAxis(TRUE, 1100, 1700, 6);
		m_wndPlot.autoScale(FALSE, TRUE, TRUE);
		if (sigSel != can2::Gnss::esigInvalid)
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
	SetTimer(1, 1, NULL);
}


void CRingPcoView::OnRadioNorth()
{
	SetTimer(1, 1, NULL);
}


void CRingPcoView::OnRadioUp()
{
	SetTimer(1, 1, NULL);
}


void CRingPcoView::OnChangeEditEleMask()
{
	SetTimer(1, 1, NULL);
}


void CRingPcoView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		KillTimer(1);
		updateCurves();
	}

	CFormView::OnTimer(nIDEvent);
}
