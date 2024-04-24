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
#include "SelAntDlg.h"


// CSelAntDlg dialog

IMPLEMENT_DYNAMIC(CSelAntDlg, CDialogEx)

CSelAntDlg::CSelAntDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_SELECT_ANTENNA, pParent)
{

}

CSelAntDlg::~CSelAntDlg()
{
}

void CSelAntDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_ANTENNAS, m_tree);
}


BEGIN_MESSAGE_MAP(CSelAntDlg, CDialogEx)
	ON_NOTIFY(NM_CLICK, IDC_TREE_ANTENNAS, &CSelAntDlg::OnClickTreeAntennas)
END_MESSAGE_MAP()


// CSelAntDlg message handlers


BOOL CSelAntDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_tree.ModifyStyle(TVS_CHECKBOXES, TVS_CHECKBOXES);
	can2::TreeCursor tRoot = m_tree.getRootItem();
	for (auto f = m_afs.begin(); f != m_afs.end(); f++)
	{
		can2::AntexFile* paf = (*f).get();
		can2::TreeCursor tf = tRoot.addTail(paf->getName().c_str());
		tf.setData((DWORD_PTR)paf);
		tf.setCheck(TRUE);
		for (int na = 0; na < paf->childs(); na++)
		{
			can2::AntexAntenna* pa = (can2::AntexAntenna *)paf->getChild(na);
			can2::TreeCursor ta = tf.addTail(pa->getName().c_str());
			ta.setData((DWORD_PTR)pa);
			ta.setCheck(TRUE);
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CSelAntDlg::OnOK()
{
	UpdateData(TRUE);

	for (can2::TreeCursor tf = m_tree.getRootItem(); tf; tf = tf.getNextSibling())
	{
		if (!tf.getCheck())
			continue;
		can2::AntexFile* af = (can2::AntexFile *)tf.getData();
		for (can2::TreeCursor ta = tf.getChild(); ta; ta = ta.getNextSibling())
		{
			if (!ta.getCheck())
				continue;

			can2::AntexAntenna* pa = (can2::AntexAntenna *)ta.getData();
			m_selAnts.push_back(std::shared_ptr<can2::RingAntenna> (new can2::RingAntenna(*pa, "", af->m_sourceFile.c_str())));
		}
	}

	CDialogEx::OnOK();
}


void CSelAntDlg::OnClickTreeAntennas(NMHDR* pNMHDR, LRESULT* pResult)
{
	CPoint ptPos = GetMessagePos();
	m_tree.ScreenToClient(&ptPos);

	UINT nFlags = 0;
	can2::TreeCursor tHit = m_tree.hitTest(ptPos, &nFlags);

	if (!tHit)
		return;

	int nCheck = tHit.getThreeStateCheck();
	BOOL bCheck = !(nCheck > 1);
	if (nCheck == 2)
	{
		tHit.setThreeStateCheck(nCheck);
	}

	for (can2::TreeCursor t = tHit.getChild(); t; t = t.getNextSibling())
	{
		t.setCheck(bCheck);
	}
	*pResult = 0;
}
