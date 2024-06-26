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
#include "afxdialogex.h"
#include "AutoClustDlg.h"


// CAutoClustDlg dialog

std::map<std::string, BOOL> CAutoClustDlg::c_mapUse;

IMPLEMENT_DYNAMIC(CAutoClustDlg, CDialogEx)

CAutoClustDlg::CAutoClustDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_AUTO_CLUSTER, pParent)
{
	m_nLevel = 0;
}

CAutoClustDlg::~CAutoClustDlg()
{
}

void CAutoClustDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST_ANTS, m_lst);
	DDX_Control(pDX, IDC_COMBO_LEVEL, m_cmbLevel);
}


BEGIN_MESSAGE_MAP(CAutoClustDlg, CDialogEx)
END_MESSAGE_MAP()


// CAutoClustDlg message handlers


BOOL CAutoClustDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	while (m_lst.DeleteColumn(0));

	m_lst.AddColumn("Clibration", 0);
	m_lst.SetExtendedStyle(m_lst.GetExtendedStyle() | LVS_EX_CHECKBOXES);
	m_lst.SetExtendedStyle(m_lst.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	for (int i = 0; i < (int)m_prn->m_ants.size(); i++)
	{
		can2::RingAntenna* pa = m_prn->m_ants[i].get();
		int index = m_lst.AddItem(i, 0, pa->getName().c_str());
		if (c_mapUse.find(pa->getName()) != c_mapUse.end())
		{
			m_lst.SetCheck(index, c_mapUse[pa->getName()]);
		}
		else
		{
			m_lst.SetCheck(index, TRUE);
		}

		CString str;
		str.Format("%i", i);
		index = m_cmbLevel.AddString(str);
		m_cmbLevel.SetItemData(index, i);
		if (i == m_nLevel)
			m_cmbLevel.SetCurSel(index);
	}



	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CAutoClustDlg::OnOK()
{
	UpdateData(TRUE);

	m_mapUse.clear();
	for (int i = 0; i < m_lst.GetItemCount(); i++)
	{
		can2::RingAntenna* pa = m_prn->m_ants[i].get();
		m_mapUse[pa->getName()] = m_lst.GetCheck(i) ? true : false;
		c_mapUse[pa->getName()] = m_lst.GetCheck(i);
	}

	int sel = m_cmbLevel.GetCurSel();
	if (sel < 0)
		return;
	m_nLevel = m_cmbLevel.GetItemData(sel);

	CDialogEx::OnOK();
}
