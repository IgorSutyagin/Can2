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
#include "RingBar.h"
#include "RingTabView.h"
#include "RingDoc.h"
#include "RingPcvView.h"
#include "RingPcoView.h"
#include "RingDifView.h"
#include "ManualClusterDlg.h"

IMPLEMENT_DYNAMIC(CRingBar, CPaneDialog)

int CRingBar::c_nSigIDCs[can2::Gnss::esigInvalid] = {
	IDC_CHECK_GPS_P1, // G01
	IDC_CHECK_GPS_P2, // G02
	IDC_CHECK_GPS_L5, // G05

	IDC_CHECK_GLO_P1, // R01
	IDC_CHECK_GLO_P2, // R02,	// L2 GLONASS 1246
	-1,				  // R03,	// GLO 1202.025
	-1,				  // R04,	// GLO 1600.995
	-1,				  // R06,	// GLO 1248.06

	IDC_CHECK_GAL_E1, // E01,	// E1  Galileo  1575.42
	IDC_CHECK_GAL_E5A,// E05,	// E5a Galileo  1176.45
	IDC_CHECK_GAL_E5B,// E07,	// E5b Galileo 1207.14
	IDC_CHECK_GAL_E5AB,// E08,	// E5 (E5a+E5b) Galileo 1191.795
	IDC_CHECK_GAL_E6, // E06,	// E6 Galileo 1278.75

	-1,				  // C01,	// B1C 1575.42	 // IVS 2020.08.29 Added BeiDou (Compass) and other systems
	IDC_CHECK_BEI_B1, // C02,	// B1I 1561.098
	-1,				  // C05,	// 1176.45
	IDC_CHECK_BEI_B3, // C06,	// B3 1268.52
	IDC_CHECK_BEI_B2, // C07,	// B2 1207.14
	-1,				  // C08,	// 1191.795

	-1,				  // J01,	// L1 1575.42
	-1,				  // J02,	// L2 1227.6
	-1,				  // J05,	// L5 1176.45
	-1,				  // J06,	// LEX 1278.75

	-1,				  // S01,	// L1 1575.42
	-1,				  // S05,	// L5 1176.45

	-1,				  // I05,	// 1176.45
};

CRingBar::CRingBar()
{
	m_bHandleMinSize = TRUE;
	SetMinSize(can2::calcDialogSize(IDD_DIALOGBAR_RING, theApp.m_hInstance));
	m_pTabView = nullptr;
	m_nMeanType = 1;
	m_nUseSignal = 2;

}

CRingBar::~CRingBar()
{
}

can2::RingNode* CRingBar::getRingNode()
{
	CRingDoc* pd = m_pTabView->GetDocument();
	return pd->m_rn;
}

BEGIN_MESSAGE_MAP(CRingBar, CPaneDialog)
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_MESSAGE(WM_INITDIALOG, OnInitDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_CLUST, &OnSelchangeComboClust)
	ON_NOTIFY(IVSRINGDIST_SEL_CHANGED, IDC_CUSTOM_DISTS, OnSelchangeRingDist)

	ON_COMMAND(IDC_CHECK_GPS_P1, &OnCheckSlot)
	ON_COMMAND(IDC_CHECK_GPS_P2, &OnCheckSlot)
	ON_COMMAND(IDC_CHECK_GPS_L5, &OnCheckSlot)

	ON_COMMAND(IDC_CHECK_GLO_P1, &OnCheckSlot)
	ON_COMMAND(IDC_CHECK_GLO_P2, &OnCheckSlot)

	ON_COMMAND(IDC_CHECK_GAL_E1, &OnCheckSlot)
	ON_COMMAND(IDC_CHECK_GAL_E5A, &OnCheckSlot)
	ON_COMMAND(IDC_CHECK_GAL_E5AB, &OnCheckSlot)
	ON_COMMAND(IDC_CHECK_GAL_E5B, &OnCheckSlot)
	ON_COMMAND(IDC_CHECK_GAL_E6, &OnCheckSlot)

	ON_COMMAND(IDC_CHECK_BEI_B1, &OnCheckSlot)
	ON_COMMAND(IDC_CHECK_BEI_B2, &OnCheckSlot)
	ON_COMMAND(IDC_CHECK_BEI_B3, &OnCheckSlot)

	ON_COMMAND(IDC_CHECK_SIMPLE, &OnCheckSlot)
	ON_BN_CLICKED(IDC_BUTTON_MANUAL_CLUSTER, &OnBnClickedButtonManualCluster)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_MANUAL_CLUSTER, &OnUpdateButtonManualCluster)
END_MESSAGE_MAP()

LRESULT CRingBar::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

void CRingBar::onCreateView(CRingTabView* pView)
{
	m_pTabView = pView;

	UpdateData(FALSE);

	can2::RingNode* prn = getRingNode();

	for (int i = 0; i < prn->childs(); i++)
	{
		CString str;
		str.Format("%i", i);
		int index = m_cmbCluster.AddString(str);
		if (i == 0)
			m_cmbCluster.SetCurSel(i);
		m_cmbCluster.SetItemData(index, i);
	}
	m_wndDists.onInitialUpdate(getRingNode(), this);
	if (prn->childs() > 0)
	{
		m_wndDists.setSel(0, 0);
		CRingDoc* pDoc = m_pTabView->GetDocument();
		pDoc->select(0, 0);
	}
}

bool CRingBar::isSelected(int nc1, int nc2) const
{
	return std::min(nc1, nc2) == std::min(m_wndDists.getSel().first, m_wndDists.getSel().second) &&
		std::max(nc1, nc2) == std::max(m_wndDists.getSel().first, m_wndDists.getSel().second);
}

int CRingBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPaneDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	PostMessage(WM_INITDIALOG, 0, 0);
	return 0;
}

void CRingBar::DoDataExchange(CDataExchange* pDX)
{
	// TODO: Add your specialized code here and/or call the base class

	CPaneDialog::DoDataExchange(pDX);

	can2::RingNode* prn = getRingNode();

	DDX_Control(pDX, IDC_COMBO_CLUST, m_cmbCluster);
	DDX_Control(pDX, IDC_CUSTOM_DISTS, m_wndDists);
	DDX_Radio(pDX, IDC_RADIO_ARMEAN, m_nMeanType);
	DDX_Radio(pDX, IDC_RADIO_CLUST_ANY, m_nUseSignal);

	BOOL bSimple = prn->m_metrics.bSimpleMode;
	DDX_Check(pDX, IDC_CHECK_SIMPLE, bSimple);
	prn->m_metrics.bSimpleMode = bSimple;
	for (int ns = 0; ns < can2::Gnss::esigInvalid; ns++)
	{
		if (c_nSigIDCs[ns] < 0)
			continue;
		can2::Gnss::Signal es = (can2::Gnss::Signal)ns;
		BOOL bOn = prn->m_metrics.isUsed(es) ? TRUE : FALSE;
		DDX_Check(pDX, c_nSigIDCs[ns], bOn);
		if (pDX->m_bSaveAndValidate)
			prn->m_metrics.use(es, bOn ? true : false);
	}

}

void CRingBar::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == 6) // Signal
	{
		CWaitCursor wc;
		KillTimer(nIDEvent);
		UpdateData(TRUE);
		can2::RingNode* prn = getRingNode();
		prn->compDists();
		m_wndDists.Invalidate();
	}
	else if (nIDEvent == 2) // Cluster
	{
		KillTimer(nIDEvent);
		UpdateData(TRUE);
		can2::RingNode* prn = getRingNode();
		int nSel = m_cmbCluster.GetCurSel();
		if (nSel >= 0)
		{
			CWaitCursor wc;
			int clust = m_cmbCluster.GetItemData(nSel);
			prn->clusterize(clust, can2::RingNode::ClusterMode(m_nMeanType, m_nUseSignal));
			m_wndDists.Invalidate();
			CRingDoc* pDoc = m_pTabView->GetDocument();
			m_wndDists.setSel(0, 0);
			pDoc->select(0, 0);
		}
	}
	CPaneDialog::OnTimer(nIDEvent);
}

void CRingBar::OnSelchangeComboClust()
{
	SetTimer(2, 1, NULL);
}

void CRingBar::OnSelchangeRingDist(NMHDR* pnmhdr, LRESULT* result)
{
	std::pair<int, int> sel = m_wndDists.getSel();
	CRingDoc* pDoc = m_pTabView->GetDocument();
	pDoc->select(sel.first, sel.second);
}

void CRingBar::OnCheckSlot()
{
	SetTimer(6, 1, NULL);
}



void CRingBar::OnBnClickedButtonManualCluster()
{
	UpdateData(TRUE);
	can2::RingNode* prn = getRingNode();
	CManualClusterDlg dlg(this);
	dlg.m_prn = prn;

	if (dlg.DoModal() != IDOK)
		return;

	CWaitCursor wc;

	prn->clusterizeManual(dlg.m_cls, can2::RingNode::ClusterMode(m_nMeanType, m_nUseSignal));

	m_wndDists.Invalidate();
	CRingDoc* pDoc = m_pTabView->GetDocument();
	m_wndDists.setSel(0, 0);
	pDoc->select(0, 0);
}

void CRingBar::OnUpdateButtonManualCluster(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}
