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
#include "PcoTableView.h"
#include "ListCtrlEx.h"
#include "Tools.h"

// CPcoTableView
CCan2Doc* CPcoTableView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCan2Doc)));
	return (CCan2Doc*)m_pDocument;
}


IMPLEMENT_DYNCREATE(CPcoTableView, CFormView)

CPcoTableView::CPcoTableView()
	: CFormView(CPcoTableView::IDD)
{
	m_bNeedInit = true;
	m_eleMask = 0.0;
	m_nOffsetMode = can2::AntexAntenna::eSinAndCos;
}

CPcoTableView::~CPcoTableView()
{
}

void CPcoTableView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_ELE_MASK, m_eleMask);
	DDX_Control(pDX, IDC_LIST_PCO, m_lstPco);
	DDX_Radio(pDX, IDC_RADIO_NO_WEIGHT, m_nOffsetMode);
}

BEGIN_MESSAGE_MAP(CPcoTableView, CFormView)
	ON_WM_SIZE()
	ON_EN_CHANGE(IDC_EDIT_ELE_MASK, &CPcoTableView::OnChangeEditEleMask)
	ON_BN_CLICKED(IDC_RADIO_NO_WEIGHT, &CPcoTableView::OnClickedRadioMode)
	ON_BN_CLICKED(IDC_RADIO_SIN_AND_COS, &CPcoTableView::OnClickedRadioMode)
	ON_BN_CLICKED(IDC_RADIO_ONLY_COS, &CPcoTableView::OnClickedRadioMode)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_EXPORT, &CPcoTableView::OnBnClickedButtonExport)
	ON_BN_CLICKED(IDC_BUTTON_COPY, &CPcoTableView::OnBnClickedButtonCopy)
END_MESSAGE_MAP()


// CPcoTableView diagnostics

#ifdef _DEBUG
void CPcoTableView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPcoTableView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CPcoTableView message handlers


void CPcoTableView::OnSize(UINT nType, int cx, int cy)
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
		if (nId == IDC_LIST_PCO)
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


void CPcoTableView::OnInitialUpdate()
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

	CListCtrlEx& lst = (CListCtrlEx&)m_lstPco;
	while (lst.DeleteColumn(0));

	lst.AddColumn("Signal", 0);
	lst.AddColumn("East (mm) computed", 1);
	lst.AddColumn("North (mm) computed", 2);
	lst.AddColumn("Up (mm) computed", 3);
	lst.AddColumn("East (mm) from file", 4);
	lst.AddColumn("North (mm) from file", 5);
	lst.AddColumn("Up (mm) from file", 6);

	lst.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_AUTOSIZECOLUMNS);

	updateCurves();
}

void CPcoTableView::updateCurves()
{
	CWaitCursor wc;

	UpdateData(TRUE);
	CCan2Doc* pDoc = GetDocument();
	can2::Gnss::Signal sigSel = pDoc->m_signal;
	can2::Node* node = pDoc->m_node;
	if (!node->isAntenna())
		return;
	can2::AntexAntenna* aa = (can2::AntexAntenna*)node;

	CListCtrlEx& lst = (CListCtrlEx&)m_lstPco;
	lst.DeleteAllItems();

	int nIndex = 0;
	for (int nf = can2::Gnss::G01; nf < can2::Gnss::esigInvalid; nf++)
	{
		can2::Gnss::Signal es = (can2::Gnss::Signal)nf;

		if (!aa->hasPcc(es))
			continue;

		lst.AddItem(nIndex, 0, can2::Gnss::getSignalName(es));

		for (int i = 0; i < 2; i++)
		{
			can2::Point3d off = i == 0 ? aa->calcOffset(es, m_eleMask, (can2::Node::OffsetMode)m_nOffsetMode, nullptr)
				: aa->getOffset(es);
			if (!off.isValid())
				continue;
			off *= 1000;

			std::string str = can2::stringFormat("%.2f", off.e);
			lst.AddItem(nIndex, 3 * i + 1, str.c_str());
			str = can2::stringFormat("%.2f", off.n);
			lst.AddItem(nIndex, 3 * i + 2, str.c_str());
			str = can2::stringFormat("%.2f", off.u);
			lst.AddItem(nIndex, 3 * i + 3, str.c_str());
		}
		nIndex++;
	}

	for (int i = 0; i < 7; i++)
	{
		if (i == 0)
		{
			lst.SetColumnWidth(i, LVSCW_AUTOSIZE);
		}
		else
		{
			lst.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);

			LVCOLUMN lv;
			memset(&lv, 0, sizeof(lv));
			lv.mask = LVCF_FMT;
			//lst.GetColumn(i, &lv);
			lv.fmt = LVCFMT_CENTER;
			lst.SetColumn(i, &lv);
		}
	}

}

void CPcoTableView::OnChangeEditEleMask()
{
	SetTimer(1, 1, NULL);
}

void CPcoTableView::OnClickedRadioMode()
{
	SetTimer(1, 1, NULL);
}

void CPcoTableView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		KillTimer(1);
		updateCurves();
	}

	CFormView::OnTimer(nIDEvent);
}


std::string CPcoTableView::c_strFile;

void CPcoTableView::OnBnClickedButtonExport()
{
	CWaitCursor wc;

	UpdateData(TRUE);
	CCan2Doc* pDoc = GetDocument();
	can2::Gnss::Signal sigSel = pDoc->m_signal;
	can2::Node* node = pDoc->m_node;
	if (!node->isAntenna())
		return;
	can2::AntexAntenna* aa = (can2::AntexAntenna*)node;

	CStdioFile fOut;
	CFileDialog dlg(FALSE, "txt", c_strFile.c_str(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "Text files (*.txt)|*.txt|All files (*.*)|*.*||", this);
	if (dlg.DoModal() != IDOK)
		return;

	c_strFile = dlg.GetPathName();

	if (!fOut.Open(c_strFile.c_str(), CFile::modeReadWrite | CFile::modeCreate))
	{
		AfxMessageBox("Can't open output file");
		return;
	}

	fOut.WriteString("Signal\tEast (mm) (computed)\tNorth (mm) (computed)\tUp (mm) (computed)\tEast (mm) (from file)\tNorth (mm) (from file)\tUp (mm) (from file)\n");

	int nIndex = 0;
	for (int nf = can2::Gnss::G01; nf < can2::Gnss::esigInvalid; nf++)
	{
		can2::Gnss::Signal es = (can2::Gnss::Signal)nf;

		if (!aa->hasPcc(es))
			continue;

		std::string str = can2::stringFormat("%s\t", can2::Gnss::getSignalName(es));
		fOut.WriteString(str.c_str());

		for (int i = 0; i < 2; i++)
		{
			can2::Point3d off = i == 0 ? aa->calcOffset(es, m_eleMask, (can2::Node::OffsetMode)m_nOffsetMode, nullptr)
				: aa->getOffset(es);
			if (!off.isValid())
				continue;
			off *= 1000;

			str = can2::stringFormat("%.2f\t", off.e);
			fOut.WriteString(str.c_str());
			str = can2::stringFormat("%.2f\t", off.n);
			fOut.WriteString(str.c_str());
			str = can2::stringFormat("%.2f\t", off.u);
			fOut.WriteString(str.c_str());
		}
		fOut.WriteString("\n");
		nIndex++;
	}

	fOut.Close();

	ShellExecute(GetSafeHwnd(), "open", c_strFile.c_str(), NULL, NULL, SW_SHOW);
}


void CPcoTableView::OnBnClickedButtonCopy()
{
	UpdateData(TRUE);
	CCan2Doc* pDoc = GetDocument();
	can2::Gnss::Signal sigSel = pDoc->m_signal;
	can2::Node* node = pDoc->m_node;
	if (!node->isAntenna())
		return;
	can2::AntexAntenna* aa = (can2::AntexAntenna*)node;

	std::string strData = "Signal,East (mm) (computed);North (mm) (computed);Up (mm) (computed);East (mm) (from file);North (mm) (from file);Up (mm) (from file)\r\n";

	int nIndex = 0;
	for (int nf = can2::Gnss::G01; nf < can2::Gnss::esigInvalid; nf++)
	{
		can2::Gnss::Signal es = (can2::Gnss::Signal)nf;

		if (!aa->hasPcc(es))
			continue;

		std::string str = can2::stringFormat("%s;", can2::Gnss::getSignalName(es));
		strData += str;

		for (int i = 0; i < 2; i++)
		{
			can2::Point3d off = i == 0 ? aa->calcOffset(es, m_eleMask, (can2::Node::OffsetMode)m_nOffsetMode, nullptr)
				: aa->getOffset(es);
			if (!off.isValid())
				continue;
			off *= 1000;

			str = can2::stringFormat("%.2f;", off.e);
			strData += str;
			str = can2::stringFormat("%.2f;", off.n);
			strData += str;
			str = can2::stringFormat("%.2f;", off.u);
			strData += str;
		}
		strData += "\r\n";
		nIndex++;
	}

	UINT uCsv = RegisterClipboardFormatA("Csv");
	HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (strData.length() + 1) * sizeof(CHAR));
	if (hglbCopy == NULL)
	{
		return;
	}

	// Lock the handle and copy the text to the buffer. 

	LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
	memcpy(lptstrCopy, strData.c_str(), strData.length() * sizeof(CHAR));
	lptstrCopy[strData.length()] = (CHAR)0;    // null character 
	GlobalUnlock(hglbCopy);

	// Place the handle on the clipboard. 
	OpenClipboard();
	SetClipboardData(uCsv, hglbCopy);
	CloseClipboard();

}
