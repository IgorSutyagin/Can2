#include "pch.h"

#include "Can2.h"

#include "RingDoc.h"
#include "RingNode.h"
#include "RingFrm.h"
#include "RingBar.h"
#include "CurveFormatPg.h"
#include "RingNaPcvView.h"

// CRingNaPcvView
CRingDoc* CRingNaPcvView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CRingDoc)));
	return (CRingDoc*)m_pDocument;
}

CRingBar* CRingNaPcvView::getRingBar() const
{
	CRingFrame* pFrame = (CRingFrame*)GetParentFrame();
	return pFrame->m_pBar;
}



IMPLEMENT_DYNCREATE(CRingNaPcvView, CFormView)

CRingNaPcvView::CRingNaPcvView()
	: CFormView(CRingNaPcvView::IDD)
	, m_yMax(1)
	, m_yMin(0)
	, m_xMin(0)
	, m_xMax(1)
	, m_byAuto(TRUE)
	, m_bxAuto(TRUE)
	, m_yDivs(10)
	, m_xDivs(10)
	, m_useRo(FALSE)
	, m_nPcvType(0)
	, m_bLegend(TRUE)
	, m_bBands(FALSE)
{
	m_bNeedInit = true;
}

CRingNaPcvView::~CRingNaPcvView()
{
}

void CRingNaPcvView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CUSTOM_PLOT, m_wndPlot);
	DDX_Check(pDX, IDC_CHECK_USE_RO, m_useRo);
	DDX_Control(pDX, IDC_COMBO_COMBINATION, m_cmbSignal);
	DDX_Control(pDX, IDC_COMBO_REF, m_cmbRef);
	DDX_Control(pDX, IDC_COMBO_TEST, m_cmbTest);

	DDX_Text(pDX, IDC_EDIT_Y_MAX, m_yMax);
	DDX_Text(pDX, IDC_EDIT_Y_MIN, m_yMin);
	DDX_Text(pDX, IDC_EDIT_X_MIN, m_xMin);
	DDX_Text(pDX, IDC_EDIT_X_MAX, m_xMax);
	DDX_Check(pDX, IDC_CHECK_Y_AUTO, m_byAuto);
	DDX_Check(pDX, IDC_CHECK_X_AUTO, m_bxAuto);
	DDX_Check(pDX, IDC_CHECK_LEGEND, m_bLegend);
	DDX_Check(pDX, IDC_CHECK_BANDS, m_bBands);
	DDX_Text(pDX, IDC_EDIT_Y_DIVS, m_yDivs);
	DDX_Text(pDX, IDC_EDIT_X_DIVS, m_xDivs);
	DDX_Radio(pDX, IDC_RADIO_NA_PCV, m_nPcvType);
}

BEGIN_MESSAGE_MAP(CRingNaPcvView, CFormView)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_CHECK_USE_RO, &CRingNaPcvView::OnClickedCheckUseRo)
	ON_CBN_SELCHANGE(IDC_COMBO_COMBINATION, &OnSelchangeComboSignal)
	ON_CBN_SELCHANGE(IDC_COMBO_REF, &OnSelchangeComboSignal)
	ON_CBN_SELCHANGE(IDC_COMBO_TEST, &OnSelchangeComboSignal)
	ON_BN_CLICKED(IDC_RADIO_NA_PCV, &OnSelchangeComboSignal)
	ON_BN_CLICKED(IDC_RADIO_PCV, &OnSelchangeComboSignal)
	ON_WM_TIMER()

	ON_BN_CLICKED(IDC_CHECK_Y_AUTO, &CRingNaPcvView::OnBnClickedCheckAuto)
	ON_BN_CLICKED(IDC_CHECK_X_AUTO, &CRingNaPcvView::OnBnClickedCheckAuto)
	ON_BN_CLICKED(IDC_CHECK_LEGEND, &CRingNaPcvView::OnSelchangeComboSignal)
	ON_BN_CLICKED(IDC_CHECK_BANDS, &CRingNaPcvView::OnSelchangeComboSignal)
	ON_EN_CHANGE(IDC_EDIT_X_MAX, &CRingNaPcvView::OnChangeEditXMinMax)
	ON_EN_CHANGE(IDC_EDIT_X_MIN, &CRingNaPcvView::OnChangeEditXMinMax)
	ON_EN_CHANGE(IDC_EDIT_X_DIVS, &CRingNaPcvView::OnChangeEditXMinMax)
	ON_EN_CHANGE(IDC_EDIT_Y_MAX, &CRingNaPcvView::OnChangeEditYMinMax)
	ON_EN_CHANGE(IDC_EDIT_Y_MIN, &CRingNaPcvView::OnChangeEditYMinMax)
	ON_EN_CHANGE(IDC_EDIT_Y_DIVS, &CRingNaPcvView::OnChangeEditYMinMax)
	ON_BN_CLICKED(IDC_BUTTON_ADD_COMMENT, &CRingNaPcvView::OnBnClickedButtonAddComment)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_COMMENTS, &CRingNaPcvView::OnBnClickedButtonRemoveComments)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CRingNaPcvView::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, &CRingNaPcvView::OnBnClickedButtonLoad)

	ON_COMMAND(ID_PLOT2D_CURVEFORMAT, &OnCurveFormat)
END_MESSAGE_MAP()


// CRingNaPcvView diagnostics

#ifdef _DEBUG
void CRingNaPcvView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CRingNaPcvView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

CRingNaPcvView::Reference CRingNaPcvView::getRef()
{
	int nSel = m_cmbRef.GetCurSel();
	if (nSel < 0)
		return Reference(ertNone, 0);
	return m_refs[nSel];
}

CRingNaPcvView::Reference CRingNaPcvView::getTest()
{
	int nSel = m_cmbTest.GetCurSel();
	if (nSel < 0)
		return Reference(ertNone, 0);
	return m_refs[nSel];
}

void CRingNaPcvView::fillRef()
{
	m_cmbRef.ResetContent();
	CRingDoc* pDoc = GetDocument();
	can2::RingNode* prn = pDoc->m_rn;

	m_cmbRef.ResetContent();
	m_cmbTest.ResetContent();
	m_refs.clear();

	m_refs.push_back(Reference(ertNone, 0));
	int index = m_cmbRef.AddString("None");
	m_cmbRef.SetItemData(index, 0);
	m_cmbRef.SetCurSel(0);

	index = m_cmbTest.AddString("None");
	m_cmbTest.SetItemData(index, 0);
	m_cmbTest.SetCurSel(0);

	for (int nc = 0; nc < (int)prn->m_cls.size(); nc++)
	{
		if (prn->m_cls[nc].ants.size() <= 1)
			continue;

		std::string str = prn->m_cls[nc].atm->getName();
		int index = m_cmbRef.AddString(str.c_str());
		m_cmbRef.SetItemData(index, m_refs.size());
		index = m_cmbTest.AddString(str.c_str());
		m_cmbTest.SetItemData(index, m_refs.size());
		m_refs.push_back(Reference(ertCluster, nc));
	}

	for (int na = 0; na < (int)prn->m_ants.size(); na++)
	{
		std::string str = prn->m_ants[na]->getName();
		int index = m_cmbRef.AddString(str.c_str());
		m_cmbRef.SetItemData(index, m_refs.size());
		index = m_cmbTest.AddString(str.c_str());
		m_cmbTest.SetItemData(index, m_refs.size());
		m_refs.push_back(Reference(ertAntenna, na));
	}

}

void CRingNaPcvView::fillSignals()
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
}

can2::Gnss::Signal CRingNaPcvView::getSignal()
{
	int nSel = m_cmbSignal.GetCurSel();
	if (nSel < 0)
		return can2::Gnss::esigInvalid;
	return (can2::Gnss::Signal)m_cmbSignal.GetItemData(nSel);
}

// CRingNaPcvView message handlers
void CRingNaPcvView::enableControls()
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


void CRingNaPcvView::OnSize(UINT nType, int cx, int cy)
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


void CRingNaPcvView::OnInitialUpdate()
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
	m_wndPlot.setAxisPrecision(TRUE, 0);
	m_wndPlot.setAxisPrecision(FALSE, 1);

	fillSignals();
	fillRef();
	enableControls();

	updateCurves();
}

void CRingNaPcvView::updateCurves()
{
	CWaitCursor wc;

	if (!IsWindow(m_wndPlot.GetSafeHwnd()))
		return;

	UpdateData(TRUE);

	CRingDoc* pDoc = GetDocument();
	CRingBar* pBar = getRingBar();
	can2::RingNode* prn = pDoc->m_rn;
	can2::Gnss::Signal es = getSignal();
	//can2::AntexAntenna* pa = pDoc->m_selAnt.get();
	can2::RingAntenna* pref = nullptr;

	Reference ref = getRef();
	if (ref.ert == ertCluster)
		pref = prn->m_cls[ref.n].atm.get();
	else if (ref.ert == ertAntenna)
		pref = prn->m_ants[ref.n].get();
	else
		return;

	can2::RingAntenna* ptest = nullptr;

	Reference test = getTest();
	if (test.ert == ertCluster)
		ptest = prn->m_cls[test.n].atm.get();
	else if (test.ert == ertAntenna)
		ptest = prn->m_ants[test.n].get();
	else
		return;

	//COLORREF clrs[9] = { RGB(255, 0, 0), RGB(0, 200, 0), RGB(0, 0, 255),
	//	RGB(200, 0, 200), RGB(125, 125, 0), RGB(0, 200, 200),
	//	RGB(120, 0, 0), RGB(0, 120, 0), RGB(0, 0, 120) };

	can2::AntexAntenna* pa = (can2::AntexAntenna *)ptest->subtract(pref);
	if (pa == nullptr)
		return;

	m_wndPlot.removeAllCurves();
	m_wndPlot.setAxisTitle(TRUE, "Zenith angle (deg)", FALSE);
	m_wndPlot.resetInitArea();
	if (pa == nullptr)
		return;

	if (es == can2::Gnss::esigInvalid || es == can2::Gnss::esigMax)
	{
		if (m_nPcvType == 0)
		{
			for (int ns = can2::Gnss::G01; ns < es; ns++)
			{
				if (ns == can2::Gnss::esigInvalid)
					continue;
				if (!pa->hasPcc(ns))
					continue;

				can2::Gnss::Signal esig = (can2::Gnss::Signal)ns;
				std::vector<can2::Point2d> pts;

				double ro = 0;
				if (m_useRo)
				{
					can2::Point3d pco = pa->calcOffset(esig, 0.0, can2::AntexAntenna::eSinAndCos, &ro);
					ro += pco.z;
				}

				for (double zen = 0; zen <= 90; zen += 1)
				{
					double pcv = pa->getPcv(esig, zen) - ro;
					pts.push_back(can2::Point2d(zen, pcv * 1000));
				}

				m_wndPlot.addCurve(ns + 1, can2::Gnss::getSignalName(ns), pts, 3, can2::Curve2d::eSolid, can2::Gnss::getSignalColor(ns), false, false);
			}
		}
		else
		{
			for (int ns = can2::Gnss::G01; ns < es; ns++)
			{
				if (ns == can2::Gnss::esigInvalid)
					continue;
				if (!pa->hasPcc(ns))
					continue;

				can2::Gnss::Signal esig = (can2::Gnss::Signal)ns;
				std::vector<can2::Point2d> pts;

				double ro = 0;
				if (m_useRo)
				{
					can2::Point3d pco = pa->calcOffset(esig, 0.0, can2::AntexAntenna::eSinAndCos, &ro);
					ro += pco.z;
				}

				if (m_bBands)
				{
					std::vector<can2::Point2d> ptMinMax;
					for (double zen = 0; zen <= 90; zen += pa->m_grid.step.zen)
					{
						double maxPcv = -DBL_MAX;
						double minPcv = DBL_MAX;
						for (double az = 0; az < 360; az += pa->m_grid.step.az)
						{
							double pcv = pa->getPcv(esig, zen, az) - ro;
							if (pcv > maxPcv)
								maxPcv = pcv;
							if (pcv < minPcv)
								minPcv = pcv;
						}
						pts.push_back(can2::Point2d(zen, (minPcv + maxPcv) / 2 * 1000));
						ptMinMax.push_back(can2::Point2d(minPcv * 1000, maxPcv * 1000));
					}
					m_wndPlot.addBand(ns + 1, can2::Gnss::getSignalName(ns), pts, ptMinMax, 1, can2::Gnss::getSignalColor(ns));
				}
				else
				{
					for (double zen = 90; zen >= 0; zen -= 1)
					{
						double maxPcv = -DBL_MAX;
						for (double az = 0; az < 360; az += pa->m_grid.step.az)
						{
							double pcv = pa->getPcv(esig, zen, az) - ro;
							if (pcv > maxPcv)
								maxPcv = pcv;
						}
						pts.push_back(can2::Point2d(zen, maxPcv * 1000));
					}

					for (double zen = 0; zen <= 90; zen += 1)
					{
						double minPcv = DBL_MAX;
						for (double az = 0; az < 360; az += pa->m_grid.step.az)
						{
							double pcv = pa->getPcv(esig, zen, az) - ro;
							if (pcv < minPcv)
								minPcv = pcv;
						}
						pts.push_back(can2::Point2d(zen, minPcv * 1000));
					}
					m_wndPlot.addCurve(ns + 1, can2::Gnss::getSignalName(ns), pts, 3, can2::Curve2d::eSolid, can2::Gnss::getSignalColor(ns), false, false);
				}

			}
		}
	}
	else
	{
		if (pa->hasPcc(es))
		{
			if (m_nPcvType == 0)
			{
				double ro = 0;
				if (m_useRo)
				{
					can2::Point3d pco = pa->calcOffset(es, 0.0, can2::AntexAntenna::eSinAndCos, &ro);
					ro += pco.z;
				}

				std::vector<can2::Point2d> pts;
				for (double zen = 0; zen <= 90; zen += 1)
				{
					double pcv = pa->getPcv(es, zen) - ro;
					pts.push_back(can2::Point2d(zen, pcv * 1000));
				}

				m_wndPlot.addCurve(es + 1, can2::Gnss::getSignalName(es), pts, 3, can2::Curve2d::eSolid, can2::Gnss::getSignalColor(es), false, false);
			}
			else
			{
				double ro = 0;
				if (m_useRo)
				{
					can2::Point3d pco = pa->calcOffset(es, 0.0, can2::AntexAntenna::eSinAndCos, &ro);
					ro += pco.z;
				}

				std::vector<can2::Point2d> pts;

				if (m_bBands)
				{
					std::vector<can2::Point2d> ptMinMax;

					for (double zen = 0; zen <= 90; zen += pa->m_grid.step.zen)
					{
						double maxPcv = -DBL_MAX;
						double minPcv = DBL_MAX;
						for (double az = 0; az < 360; az += pa->m_grid.step.az)
						{
							double pcv = pa->getPcv(es, zen, az) - ro;
							if (pcv > maxPcv)
								maxPcv = pcv;
							if (pcv < minPcv)
								minPcv = pcv;
						}
						pts.push_back(can2::Point2d(zen, (minPcv + maxPcv) / 2 * 1000));
						ptMinMax.push_back(can2::Point2d(minPcv * 1000, maxPcv * 1000));
					}
					m_wndPlot.addBand(es + 1, can2::Gnss::getSignalName(es), pts, ptMinMax, 1, can2::Gnss::getSignalColor(es));
				}
				else
				{
					std::vector<can2::Point2d> pts;
					for (double zen = 90; zen >= 0; zen -= 1)
					{
						double maxPcv = -DBL_MAX;
						for (double az = 0; az < 360; az += pa->m_grid.step.az)
						{
							double pcv = pa->getPcv(es, zen, az) - ro;
							if (pcv > maxPcv)
								maxPcv = pcv;
						}
						pts.push_back(can2::Point2d(zen, maxPcv * 1000));
					}

					for (double zen = 0; zen <= 90; zen += 1)
					{
						double minPcv = DBL_MAX;
						for (double az = 0; az < 360; az += pa->m_grid.step.az)
						{
							double pcv = pa->getPcv(es, zen, az) - ro;
							if (pcv < minPcv)
								minPcv = pcv;
						}
						pts.push_back(can2::Point2d(zen, minPcv * 1000));
					}

					m_wndPlot.addCurve(es + 1, can2::Gnss::getSignalName(es), pts, 3, can2::Curve2d::eSolid, can2::Gnss::getSignalColor(es), false, false);
				}
			}
		}

	}

	delete pa;

	m_wndPlot.setAxis(true, 0, 90, 9);
	m_wndPlot.setLegend(m_bLegend ? true : false);
	m_wndPlot.setInitArea();
	m_wndPlot.Invalidate();
	m_wndPlot.UpdateWindow();
}


void CRingNaPcvView::OnTimer(UINT_PTR nIDEvent)
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

void CRingNaPcvView::OnClickedCheckUseRo()
{
	KillTimer(100);
	SetTimer(100, 5, NULL);
}

void CRingNaPcvView::OnSelchangeComboSignal()
{
	KillTimer(100);
	SetTimer(100, 5, NULL);
}

void CRingNaPcvView::OnBnClickedCheckAuto()
{
	SetTimer(1, 10, NULL);
}

void CRingNaPcvView::OnChangeEditXMinMax()
{
	KillTimer(3);
	SetTimer(3, 2000, NULL);
}


void CRingNaPcvView::OnChangeEditYMinMax()
{
	KillTimer(4);
	SetTimer(4, 2000, NULL);
}


void CRingNaPcvView::OnBnClickedButtonAddComment()
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


void CRingNaPcvView::OnBnClickedButtonRemoveComments()
{
	m_wndPlot.deleteInscription(0);
}


void CRingNaPcvView::OnBnClickedButtonSave()
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


void CRingNaPcvView::OnBnClickedButtonLoad()
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

void CRingNaPcvView::OnCurveFormat()
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