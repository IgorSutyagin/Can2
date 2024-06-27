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
#include "AntexAntenna.h"
#include "PcoView.h"

// CPcoView
CCan2Doc* CPcoView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCan2Doc)));
	return (CCan2Doc*)m_pDocument;
}


IMPLEMENT_DYNCREATE(CPcoView, CFormView)

CPcoView::CPcoView()
	: CFormView(CPcoView::IDD)
	, m_nCoord(2)
{
	m_bNeedInit = true;
	m_eleMask = 0.0;
	m_nOffsetMode = can2::AntexAntenna::eSinAndCos;
}

CPcoView::~CPcoView()
{
}

void CPcoView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CUSTOM_PLOT, m_wndPlot);
	DDX_Radio(pDX, IDC_RADIO_EAST, m_nCoord);
	DDX_Text(pDX, IDC_EDIT_ELE_MASK, m_eleMask);
	DDX_Radio(pDX, IDC_RADIO_NO_WEIGHT, m_nOffsetMode);
}

BEGIN_MESSAGE_MAP(CPcoView, CFormView)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_RADIO_EAST, &CPcoView::OnClickedRadioEast)
	ON_COMMAND(IDC_RADIO_NORTH, &CPcoView::OnRadioNorth)
	ON_COMMAND(IDC_RADIO_UP, &CPcoView::OnRadioUp)
	ON_BN_CLICKED(IDC_RADIO_HPCO, &CPcoView::OnClickedRadioEast)
	ON_BN_CLICKED(IDC_RADIO_NO_WEIGHT, &CPcoView::OnClickedRadioMode)
	ON_BN_CLICKED(IDC_RADIO_SIN_AND_COS, &CPcoView::OnClickedRadioMode)
	ON_BN_CLICKED(IDC_RADIO_ONLY_COS, &CPcoView::OnClickedRadioMode)
	ON_EN_CHANGE(IDC_EDIT_ELE_MASK, &CPcoView::OnChangeEditEleMask)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CPcoView diagnostics

#ifdef _DEBUG
void CPcoView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPcoView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CPcoView message handlers


void CPcoView::OnSize(UINT nType, int cx, int cy)
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


void CPcoView::OnInitialUpdate()
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

void CPcoView::updateCurves()
{
	CWaitCursor wc;

	UpdateData(TRUE);
	CCan2Doc* pDoc = GetDocument();
	can2::Gnss::Signal sigSel = pDoc->m_signal;
	can2::Node * node = pDoc->m_node;
	if (!node->isAntenna())
		return;
	can2::AntexAntenna* aa = (can2::AntexAntenna*)node;

	COLORREF clrs[9] = { RGB(255, 0, 0), RGB(0, 200, 0), RGB(0, 0, 255),
		RGB(200, 0, 200), RGB(125, 125, 0), RGB(0, 200, 200),
		RGB(120, 0, 0), RGB(0, 120, 0), RGB(0, 0, 120) };

	m_wndPlot.removeAllCurves();

	m_wndPlot.setAxisTitle(true, m_nCoord <= 2 ? "Frequency (MHz)" : "East (mm)", false);
	CString str;
	str.Format("%s (mm)", m_nCoord == 0 ? "East" : m_nCoord == 1 ? "North" : m_nCoord == 2 ? "Up" : "North");
	m_wndPlot.setAxisTitle(false, str, false);
	m_wndPlot.resetInitArea();
	m_wndPlot.removeAllCircles();

	can2::Point3d ptMax(-DBL_MAX, -DBL_MAX, -DBL_MAX);
	can2::Point3d ptMin(DBL_MAX, DBL_MAX, DBL_MAX);

	std::vector<can2::Point2d> pts[2]; // [0] - Computed PCOs <freq, [u,e,n]>, [1] - PCOs from the ANTEX file  <freq, [u,e,n]>
	std::vector<can2::Point3d> ptHs[2]; // [0] - Computed HPCO <freq, e, n>, [1] - HPCO from the ANTEX file <freq, e, n>

	for (int nf = can2::Gnss::G01; nf < can2::Gnss::esigInvalid; nf++)
	{
		can2::Gnss::Signal es = (can2::Gnss::Signal)nf;

		if (!aa->hasPcc(es))
			continue;

		for (int i=0; i<2; i++)
		{
			can2::Point3d off = i == 0 ? aa->calcOffset(es, m_eleMask, (can2::Node::OffsetMode)m_nOffsetMode, nullptr)
				: aa->getOffset(es);
			if (!off.isValid())
				continue;
			off *= 1000;
			if (m_nCoord <= 2)
				pts[i].push_back(can2::Point2d(can2::Gnss::getSysFreq(es) / 1000000.0, off[m_nCoord]));
			else
				ptHs[i].push_back(can2::Point3d(can2::Gnss::getSysFreq(es) / 1000000.0, off.e, off.n));

			for (int i = 0; i < 3; i++)
			{
				if (off[i] < ptMin[i])
					ptMin[i] = off[i];
				if (off[i] > ptMax[i])
					ptMax[i] = off[i];
			}
		}

	}

	if (m_nCoord <= 2)
	{
		for (int i = 0; i < 2; i++)
		{
			std::sort(pts[i].begin(), pts[i].end(), [](const can2::Point2d& a, const can2::Point2d& b) {
				return a.x < b.x;
			});
		}
	}
	else
	{
		for (int i = 0; i < 2; i++)
		{
			std::sort(ptHs[i].begin(), ptHs[i].end(), [](const can2::Point3d& a, const can2::Point3d& b) {
				return a.x < b.x;
			});
			pts[i].resize(ptHs[i].size());
			for (int j = 0; j < (int)ptHs[i].size(); j++)
				pts[i][j] = can2::Point2d(ptHs[i][j].y, ptHs[i][j].z);
		}
	}

	for (int i = 0; i < 2; i++)
	{
		std::string str = i == 0 ? "Computed" : "From ANTEX file";
		CCan2App* pApp = getCan2App();
		int nWidth = (int)ceil(pApp->m_sp.pixInPoint * 3);
		m_wndPlot.addCurve(i+1, str.c_str(), pts[i], nWidth, can2::Curve2d::eLineAndPoints, clrs[i]);

		can2::Curve2d* pCurve = m_wndPlot.getCurve(i+1);
		if (pCurve != nullptr)
			pCurve->m_bDirection = m_nCoord > 2;
	}

	if (m_nCoord <= 2)
	{
		m_wndPlot.setAxis(true, 1100, 1700, 6);
		m_wndPlot.autoScale(false, true, true);
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
		m_wndPlot.setAxis(true, -range, range, nDivs);
		m_wndPlot.setAxis(false, -range, range, nDivs);
		m_wndPlot.addCircle(1, 0, 0, 1);
		if (sigSel != can2::Gnss::esigInvalid)
		{
			double fsel = can2::Gnss::getSysFreq(sigSel) / 1000000.0;
			for (int i = 0; i < 2; i++)
			{
				std::vector<can2::Point3d>::iterator it = std::find_if(ptHs[i].begin(), ptHs[i].end(), [&](const can2::Point3d& pt) {
					return pt.x == fsel;
				});
				if (it != ptHs[i].end())
				{
					m_wndPlot.addCircle(10+i, it->y, it->z, 0.1, RGB(255, 0, 0), 2);
				}
			}
		}
	}

	m_wndPlot.setInitArea();
	m_wndPlot.Invalidate();
	m_wndPlot.UpdateWindow();
}

void CPcoView::OnClickedRadioEast()
{
	SetTimer(1, 1, NULL);
}


void CPcoView::OnRadioNorth()
{
	SetTimer(1, 1, NULL);
}


void CPcoView::OnRadioUp()
{
	SetTimer(1, 1, NULL);
}


void CPcoView::OnChangeEditEleMask()
{
	SetTimer(1, 500, NULL);
}

void CPcoView::OnClickedRadioMode()
{
	SetTimer(1, 1, NULL);
}



void CPcoView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		KillTimer(1);
		updateCurves();
	}

	CFormView::OnTimer(nIDEvent);
}
