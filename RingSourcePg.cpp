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
#include "ListCtrlEx.h"
#include "AntexFile.h"
#include "RingSourcePg.h"
#include "SelAntDlg.h"
#include "Tools.h"


// CRingSourcePg dialog

IMPLEMENT_DYNAMIC(CRingSourcePg, CPropertyPage)

CRingSourcePg::CRingSourcePg()
	: m_prn(new can2::RingNode()), CPropertyPage(IDD_PROPPAGE_RING_SOURCE)
{
}

CRingSourcePg::~CRingSourcePg()
{
}

void CRingSourcePg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SOURCE, m_lstSource);
	DDX_Text(pDX, IDC_EDIT_TITLE, m_strTitle);
}


BEGIN_MESSAGE_MAP(CRingSourcePg, CPropertyPage)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST_SOURCE, &CRingSourcePg::OnLvnEndlabeleditListSource)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CRingSourcePg::OnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CRingSourcePg::OnClickedButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_UP, &CRingSourcePg::OnBnClickedButtonUp)
	ON_BN_CLICKED(IDC_BUTTON_DOWN, &CRingSourcePg::OnBnClickedButtonDown)
END_MESSAGE_MAP()


// CRingSourcePg message handlers
BOOL CRingSourcePg::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	CListCtrlEx& lst = (CListCtrlEx&)m_lstSource;

	while (lst.DeleteColumn(0));

	lst.SetExtendedStyle(lst.GetExtendedStyle() | LVS_EX_CHECKBOXES);
	lst.SetExtendedStyle(lst.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	lst.AddColumn("Name", 0);
	lst.AddColumn("Antenna name", 1);
	lst.AddColumn("ANTEX file", 2);

	int item = 0;
	if (m_prn != nullptr)
	{
		for (int na = 0; na < m_prn->childs(); na++)
		{
			can2::RingAntenna* pa = (can2::RingAntenna*)m_prn->getChild(na);

			lst.AddItem(item, 0, pa->m_alias.c_str());
			lst.AddItem(item, 1, pa->getFullName().c_str());
			lst.AddItem(item, 2, can2::AntexFile::getShortPathName(pa->m_sourceFile.c_str()).c_str());
			lst.SetCheck(item, TRUE);
			lst.SetItemData(item, (DWORD_PTR)pa);
			item++;
		}
	}

	if (item == 0)
	{
		lst.SetColumnWidth(0, 100);
		lst.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
		lst.SetColumnWidth(2, LVSCW_AUTOSIZE_USEHEADER);
	}
	else
	{
		lst.SetColumnWidth(0, 100);
		lst.SetColumnWidth(1, LVSCW_AUTOSIZE);
		lst.SetColumnWidth(2, LVSCW_AUTOSIZE);
	}

	m_strTitle = m_prn->m_title.c_str();
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CRingSourcePg::OnLvnEndlabeleditListSource(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVDISPINFO* pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	int nAnt = pDispInfo->item.iItem;
	int col = pDispInfo->item.iSubItem;

	if (pDispInfo->item.pszText == nullptr)
	{
		*pResult = 0;
		return;
	}

	if (col == 0)
	{
		CListCtrlEx& lst = (CListCtrlEx&)m_lstSource;
		can2::RingAntenna* pa = (can2::RingAntenna*)lst.GetItemData(nAnt);
		pa->m_alias = pDispInfo->item.pszText;
	}

	*pResult = TRUE;
}

void CRingSourcePg::OnClickedButtonAdd()
{
	const int FILE_LIST_BUFFER_SIZE = 2048;
	CString fileName;
	CHAR* p = fileName.GetBuffer(FILE_LIST_BUFFER_SIZE);

	CFileDialog dlgFile(TRUE, "", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT,
		"Antex files (*.atx)|*.atx|All files (*.*)|*.*||", AfxGetMainWnd());

	OPENFILENAME& ofn = dlgFile.GetOFN();
	ofn.Flags |= OFN_ALLOWMULTISELECT;
	ofn.lpstrFile = p;
	ofn.nMaxFile = FILE_LIST_BUFFER_SIZE;

	if (dlgFile.DoModal() != IDOK)
		return;

	fileName.ReleaseBuffer();
	CStringArray strFiles;

	CHAR* pBufEnd = p + FILE_LIST_BUFFER_SIZE - 2;
	CHAR* start = p;
	while ((p < pBufEnd) && (*p))
		p++;
	if (p > start)
	{
		TRACE(_T("Path to folder where files were selected:  %s\r\n\r\n"), start);
		CString strPath = start;
		p++;

		CFileStatus fs;
		if (CFile::GetStatus(strPath, fs))
		{
			if (!(fs.m_attribute & CFile::directory))
			{
				strFiles.Add(strPath);
			}
		}

		int fileCount = 1;
		while ((p < pBufEnd) && (*p))
		{
			start = p;
			while ((p < pBufEnd) && (*p))
				p++;
			if (p > start)
				TRACE(_T("%2d. %s\r\n"), fileCount, start);
			p++;

			CString str = strPath + "\\" + start;
			bool bAlreadyExist = false;
			for (int j = 0; j < strFiles.GetSize(); j++)
			{
				if (strFiles[j].CompareNoCase(str) == 0)
				{
					bAlreadyExist = true;
					break;
				}
			}
			if (!bAlreadyExist)
			{
				fileCount++;
				strFiles.Add(str);
			}
		}
	}

	std::vector<std::shared_ptr<can2::AntexFile>> afs;
	for (int i = 0; i < strFiles.GetSize(); i++)
	{
		CWaitCursor wc;
		try
		{
			std::shared_ptr<can2::AntexFile> af = can2::AntexFile::load(strFiles[i]);
			afs.push_back(af);
		}
		catch (std::exception e)
		{
			AfxMessageBox(e.what());
		}
	}

	bool bManyAnts = afs.end() != std::find_if(afs.begin(), afs.end(), [](const std::shared_ptr<can2::AntexFile>& af) {
		return af->childs() > 1;
	});

	std::vector<std::shared_ptr<can2::RingAntenna>> ras;
	if (bManyAnts)
	{
		CSelAntDlg dlg(this);
		dlg.m_afs = afs;
		if (dlg.DoModal() != IDOK)
			return;

		ras = dlg.m_selAnts;
	}
	else
	{
		for (auto itf = afs.begin(); itf != afs.end(); itf++)
		{
			can2::AntexFile* paf = itf->get();
			for (int na = 0; na < paf->childs(); na++)
			{
				can2::AntexAntenna* pa = (can2::AntexAntenna *)paf->getChild(na);
				ras.push_back(std::shared_ptr<can2::RingAntenna>(new can2::RingAntenna(*pa, "", paf->m_sourceFile.c_str())));
			}
		}
	}
	
	CWaitCursor wc;

	m_prn->addAntennas(ras);
	CListCtrlEx& lst = (CListCtrlEx&)m_lstSource;
	int item = lst.GetItemCount();
	for (int na = 0; na < (int)ras.size(); na++)
	{
		can2::RingAntenna* pa = ras[na].get();

		lst.AddItem(item, 0, pa->m_alias.c_str());
		lst.AddItem(item, 1, pa->getFullName().c_str());
		lst.AddItem(item, 2, can2::AntexFile::getShortPathName(pa->m_sourceFile.c_str()).c_str());
		lst.SetCheck(item, TRUE);
		lst.SetItemData(item, (DWORD_PTR)pa);
		item++;
	}
	lst.SetColumnWidth(1, LVSCW_AUTOSIZE);
	lst.SetColumnWidth(2, LVSCW_AUTOSIZE);

}

void CRingSourcePg::OnClickedButtonRemove()
{
	CListCtrlEx& lst = (CListCtrlEx&)m_lstSource;
	int sel = lst.getSelIndex();
	if (sel < 0)
		return;

	CWaitCursor wc;

	can2::RingAntenna* pra = (can2::RingAntenna*)lst.GetItemData(sel);
	m_prn->removeAnt(pra);
	lst.DeleteItem(sel);
}

void CRingSourcePg::OnOK()
{
	UpdateData(TRUE);
	m_prn->m_title = m_strTitle;
	CPropertyPage::OnOK();
}


BOOL CRingSourcePg::OnKillActive()
{
	for (int na = 0; na < m_prn->childs(); na++)
	{
		can2::RingAntenna* pra = (can2::RingAntenna*)m_prn->getChild(na);
		if (pra->m_alias.empty())
		{
			AfxMessageBox("Antenna alias is not set");
			return FALSE;
		}
	}

	return CPropertyPage::OnKillActive();
}


void CRingSourcePg::OnBnClickedButtonUp()
{
	CListCtrlEx& lst = (CListCtrlEx&)m_lstSource;
	int sel = lst.getSelIndex();
	if (sel <= 0)
		return;

	CWaitCursor wc;

	can2::RingAntenna* pa0 = (can2::RingAntenna*)lst.GetItemData(sel);
	can2::RingAntenna* pa1 = (can2::RingAntenna*)lst.GetItemData(sel-1);

	lst.SetItemText(sel, 0, pa1->m_alias.c_str());
	lst.SetItemText(sel, 1, pa1->getFullName().c_str());
	lst.SetItemText(sel, 2, can2::AntexFile::getShortPathName(pa1->m_sourceFile.c_str()).c_str());
	lst.SetCheck(sel, TRUE);
	lst.SetItemData(sel, (DWORD_PTR)pa1);

	lst.SetItemText(sel-1, 0, pa0->m_alias.c_str());
	lst.SetItemText(sel-1, 1, pa0->getFullName().c_str());
	lst.SetItemText(sel-1, 2, can2::AntexFile::getShortPathName(pa0->m_sourceFile.c_str()).c_str());
	lst.SetCheck(sel-1, TRUE);
	lst.SetItemData(sel-1, (DWORD_PTR)pa0);

	m_prn->swapAntennas(pa0, pa1);
	lst.SetItemState(sel, 0, LVIS_SELECTED);
	lst.SetItemState(sel-1, LVIS_SELECTED, LVIS_SELECTED);
	//lst.Invalidate();
	//lst.UpdateWindow();
}


void CRingSourcePg::OnBnClickedButtonDown()
{
	CListCtrlEx& lst = (CListCtrlEx&)m_lstSource;
	int sel = lst.getSelIndex();
	if (sel < 0 || sel>=lst.GetItemCount())
		return;

	CWaitCursor wc;

	can2::RingAntenna* pa0 = (can2::RingAntenna*)lst.GetItemData(sel);
	can2::RingAntenna* pa1 = (can2::RingAntenna*)lst.GetItemData(sel + 1);

	lst.SetItemText(sel, 0, pa1->m_alias.c_str());
	lst.SetItemText(sel, 1, pa1->getFullName().c_str());
	lst.SetItemText(sel, 2, can2::AntexFile::getShortPathName(pa1->m_sourceFile.c_str()).c_str());
	lst.SetCheck(sel, TRUE);
	lst.SetItemData(sel, (DWORD_PTR)pa1);

	lst.SetItemText(sel + 1, 0, pa0->m_alias.c_str());
	lst.SetItemText(sel + 1, 1, pa0->getFullName().c_str());
	lst.SetItemText(sel + 1, 2, can2::AntexFile::getShortPathName(pa0->m_sourceFile.c_str()).c_str());
	lst.SetCheck(sel + 1, TRUE);
	lst.SetItemData(sel + 1, (DWORD_PTR)pa0);

	m_prn->swapAntennas(pa0, pa1);
	lst.SetItemState(sel, 0, LVIS_SELECTED);
	lst.SetItemState(sel + 1, LVIS_SELECTED, LVIS_SELECTED);
}
