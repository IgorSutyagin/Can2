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
#include "ManualClusterDlg.h"
#include "PromptNameDlg.h"


// CManualClusterDlg dialog

IMPLEMENT_DYNAMIC(CManualClusterDlg, CDialogEx)

CManualClusterDlg::CManualClusterDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_MANUAL_CLUSTER, pParent)
{

}

CManualClusterDlg::~CManualClusterDlg()
{
}

void CManualClusterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_CLUSTER, m_tree);
}


BEGIN_MESSAGE_MAP(CManualClusterDlg, CDialogEx)
	ON_NOTIFY(TVN_BEGINDRAG, IDC_TREE_CLUSTER, &OnTvnBegindragTreeCluster)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_BUTTON_UP, &CManualClusterDlg::OnBnClickedButtonUp)
	ON_BN_CLICKED(IDC_BUTTON_DOWN, &CManualClusterDlg::OnBnClickedButtonDown)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_CLUSTER, &CManualClusterDlg::OnRclickTreeCluster)
	ON_COMMAND(ID_MANUALCLUSTER_SETCLUSTERNAME, &CManualClusterDlg::OnManualclusterSetclustername)
	ON_UPDATE_COMMAND_UI(ID_MANUALCLUSTER_SETCLUSTERNAME, &CManualClusterDlg::OnUpdateManualclusterSetclustername)
END_MESSAGE_MAP()


// CManualClusterDlg message handlers


BOOL CManualClusterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;
	m_tree.ModifyStyle(0, dwViewStyle);

	CBitmap bmp;

	// normal tree images	
	//m_imgList.Create(IDB_FILE_VIEW_24, 18, 7, RGB(255, 0, 255));


	m_imgList.Create(IDB_FILE_VIEW, 16, 0, RGB(255, 0, 255));

	m_tree.SetImageList(&m_imgList, TVSIL_NORMAL); //TVSIL_STATE

	can2::TreeCursor tRoot = m_tree.getRootItem();
	for (int i = 0; i < (int)m_prn->m_cls.size(); i++)
	{
		can2::RingNode::Cluster& c = m_prn->m_cls[i];

		can2::TreeCursor tc = tRoot.addTail(c.atm->getName().c_str());

		if (c.ants.size() > 1)
		{
			tc.setData(1);
			for (int i = 0; i < (int)c.ants.size(); i++)
			{
				can2::RingAntenna* pa = c.ants[i].get();
				can2::TreeCursor ta = tc.addTail(pa->getName().c_str());
				ta.setData((DWORD_PTR)pa);
			}
		}
		else
		{
			tc.setData((DWORD_PTR)c.atm.get());
		}
	}

	tRoot.addTail("New Cluster").setData(1);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CManualClusterDlg::OnOK()
{
	m_cls.clear();
	for (can2::TreeCursor t = m_tree.getRootItem(); t; t = t.getNextSibling())
	{
		if (t.getData() == 1)
		{
			std::vector<can2::RingAntenna*> as;
			for (can2::TreeCursor ta = t.getChild(); ta; ta = ta.getNextSibling())
			{
				can2::RingAntenna* pa = (can2::RingAntenna*)ta.getData();
				ASSERT(pa->isRingAntenna());
				as.push_back(pa);
			}
			if (as.size() != 0)
			{
				m_cls.push_back(as);
				m_names.push_back(t.getText().c_str());
			}
		}
		else
		{
			std::vector<can2::RingAntenna*> as;
			can2::RingAntenna* pa = (can2::RingAntenna*)t.getData();
			ASSERT(pa->isRingAntenna());
			as.push_back(pa);
			m_cls.push_back(as);
			m_names.push_back(pa->getName().c_str());
		}
	}

	CDialogEx::OnOK();
}

void CManualClusterDlg::OnTvnBegindragTreeCluster(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	can2::TreeCursor t(pNMTreeView->itemNew.hItem, &m_tree);
	// So user cant drag root node
	if (t.getData() == 0 || t.getChild()) 
		return;

	// Item user started dragging ...
	m_tDrag = t;
	m_tDrop.reset();

	m_pDragImage = m_tree.CreateDragImage(m_tDrag);  // get the image list for dragging
	// CreateDragImage() returns NULL if no image list
	// associated with the tree view control
	if (!m_pDragImage)
		return;

	m_bDragging = true;
	m_pDragImage->BeginDrag(0, CPoint(-15, -15));
	POINT pt = pNMTreeView->ptDrag;
	ClientToScreen(&pt);
	m_pDragImage->DragEnter(NULL, pt);
	SetCapture();
}


void CManualClusterDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	can2::TreeCursor tItem;
	UINT		flags;

	if (m_bDragging)
	{
		POINT pt = point;
		ClientToScreen(&pt);
		CImageList::DragMove(pt);
		tItem = m_tree.hitTest(point, &flags);
		if (canDrop(tItem))
		{
			CImageList::DragShowNolock(FALSE);

			if (tItem)
			{
				can2::TreeCursor tParent = tItem.getParent();
				// Tests if dragged item is over another child !
				if (tParent && (cursor_no != ::GetCursor()))
				{
					::SetCursor(cursor_no);
					// Dont want last selected target highlighted after mouse
					// has moved off of it, do we now ?
					m_tree.SelectDropTarget(NULL);
				}
				// Is item we're over a root node and not parent root node ?
				if ((tParent == NULL) && (m_tDrag.getParent() != tItem))
				{
					if (cursor_arr != ::GetCursor()) ::SetCursor(cursor_arr);
					m_tree.SelectDropTarget(tItem);
				}

				m_tDrop = tItem;
			}
			else
			{
				m_tDrop.reset();
			}
			CImageList::DragShowNolock(TRUE);
		}
	}
	else
	{
		// Set cursor to arrow if not dragged
		// Otherwise, cursor will stay hand or arrow depen. on prev setting
		::SetCursor(cursor_arr);
	}

	CDialogEx::OnMouseMove(nFlags, point);
}

void CManualClusterDlg::OnLButtonUp(UINT nFlags, CPoint point)
{

	CDialogEx::OnLButtonUp(nFlags, point);

	if (m_bDragging)
	{
		m_bDragging = false;
		CImageList::DragLeave(this);
		CImageList::EndDrag();
		ReleaseCapture();

		if (m_pDragImage != NULL)
		{
			delete m_pDragImage;
			m_pDragImage = NULL;
		}

		// Remove drop target highlighting
		m_tree.SelectDropTarget(NULL);

		if (m_tDrag == m_tDrop)
			return;

		can2::TreeCursor tItem = m_tree.hitTest(point, &nFlags);
		// Make sure pt is over some item
		if (!canDrop(tItem)) 
			return;

		// If Drag item is an ancestor of Drop item then return
		can2::TreeCursor tParent = m_tDrop ? m_tDrop.getParent() : can2::TreeCursor();
		while (tParent != NULL)
		{
			if (tParent == m_tDrag) 
				return;
		}

		m_tree.Expand(m_tDrop, TVE_EXPAND);

		bool bDel = !(::GetKeyState(VK_CONTROL) & 0x8000);

		HTREEITEM htiNew = copyChildItem(m_tDrag, m_tDrop, TVI_LAST, bDel);
		if (bDel)
			m_tree.DeleteItem(m_tDrag);
		m_tree.SelectItem(htiNew);
	}

}

bool CManualClusterDlg::canDrop(can2::TreeCursor t) const
{
	if (t == NULL)
		return true;
	if (t.getParent())
		return false;
	if (t.getData() == 1)
		return true;

	return false;
}

bool CManualClusterDlg::canDrag(can2::TreeCursor t) const
{
	if (t == NULL)
		return false;
	if (t.getData() == 1)
		return false;
	return true;
}

void CManualClusterDlg::setDefaultCursor()
{
	// Get the windows directory
	CString strWndDir;
	GetWindowsDirectory(strWndDir.GetBuffer(MAX_PATH), MAX_PATH);
	strWndDir.ReleaseBuffer();

	strWndDir += _T("\\winhlp32.exe");
	// This retrieves cursor #106 from winhlp32.exe, which is a hand pointer
	HMODULE hModule = LoadLibrary(strWndDir);
	if (hModule) {
		HCURSOR hHandCursor = ::LoadCursor(hModule, MAKEINTRESOURCE(106));
		if (hHandCursor)
		{
			cursor_hand = CopyCursor(hHandCursor);
		}

	}
	FreeLibrary(hModule);

	cursor_arr = ::LoadCursor(NULL, IDC_ARROW);
	cursor_no = ::LoadCursor(NULL, IDC_NO);
}

can2::TreeCursor CManualClusterDlg::copyChildItem(can2::TreeCursor tItem, can2::TreeCursor tNewParent, HTREEITEM hAfter, bool bDelOrg)
{

	TV_INSERTSTRUCT tvstruct;
	HTREEITEM hNewItem;
	CString sText;

	// get information of the source item
	tvstruct.item.hItem = tItem;
	tvstruct.item.mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	m_tree.GetItem(&tvstruct.item);
	sText = m_tree.GetItemText(tItem);

	tvstruct.item.cchTextMax = sText.GetLength();
	tvstruct.item.pszText = sText.LockBuffer();

	//insert the item at proper location
	tvstruct.hParent = tNewParent;
	tvstruct.hInsertAfter = hAfter;
	tvstruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
	hNewItem = m_tree.InsertItem(&tvstruct);
	sText.ReleaseBuffer();

	//now copy item data and item state.
	m_tree.SetItemData(hNewItem, tItem.getData());
	m_tree.SetItemState(hNewItem, m_tree.GetItemState(tItem, TVIS_STATEIMAGEMASK), TVIS_STATEIMAGEMASK);

	//now delete the old item
	can2::TreeCursor tOldCluster = tItem.getParent();
	if (bDelOrg)
	{
		m_tree.DeleteItem(tItem);
		if (tOldCluster)
		{
			CString strName;
			for (can2::TreeCursor t = tOldCluster.getChild(); t; t = t.getNextSibling())
			{
				if (strName.IsEmpty())
					strName = "(";
				strName += t.getText().c_str();
				if (t.getNextSibling())
					strName += ",";
			}
			if (strName.IsEmpty())
				tOldCluster.deleteItem();
			else
			{
				strName += ")";
				tOldCluster.setText(strName);
			}
		}
	}

	can2::TreeCursor tNewItem = can2::TreeCursor(hNewItem, &m_tree);
	can2::TreeCursor tCluster = tNewItem.getParent();
	if (tCluster && tCluster.getData() == 1)
	{
		CString strName;
		for (can2::TreeCursor t = tCluster.getChild(); t; t = t.getNextSibling())
		{
			if (strName.IsEmpty())
				strName = "(";
			strName += t.getText().c_str();
			if (t.getNextSibling())
				strName += ",";
		}
		strName += ")";
		tCluster.setText(strName);
	}

	bool bFound = false;
	for (can2::TreeCursor t = m_tree.getRootItem(); t; t = t.getNextSibling())
	{
		if (t.getText() == "New Cluster" && t.getData() == 1)
		{
			bFound = true;
			break;
		}
	}
	if (!bFound)
	{
		m_tree.insertItem("New Cluster").setData(1);
	}

	return tNewItem;
}

void swapTreeItems(can2::TreeCtrlEx& tree, can2::TreeCursor& t1, can2::TreeCursor& t2)
{
	std::string str = t1.getText();
	DWORD_PTR dw = t1.getData();
	t1.setText(t2.getText().c_str());
	t1.setData(t2.getData());
	t2.setText(str.c_str());
	t2.setData(dw);
}


void CManualClusterDlg::OnBnClickedButtonUp()
{
	can2::TreeCursor ts = m_tree.getSelectedItem();
	if (!ts || ts.hasChildren())
		return;
	
	can2::TreeCursor tp = ts;
	tp = tp.getPrevSibling();
	if (!tp)
		return;

	if (!tp.hasChildren())
	{
		swapTreeItems(m_tree, ts, tp);
		tp.select();
		return;
	}

	while (tp && tp.hasChildren())
	{
		tp = tp.getPrevSibling();
	}

	can2::TreeCursor t = tp.insertNextSibling(ts.getText().c_str());
	t.setData(ts.getData());
	ts.deleteItem();
	t.select();
}


void CManualClusterDlg::OnBnClickedButtonDown()
{
	can2::TreeCursor ts = m_tree.getSelectedItem();
	if (!ts || ts.hasChildren())
		return;

	can2::TreeCursor tn = ts.getNextSibling();
	if (!tn)
		return;

	if (!tn.hasChildren())
	{
		swapTreeItems(m_tree, ts, tn);
		tn.select();
		return;
	}

	while (tn && tn.hasChildren())
	{
		tn = tn.getNextSibling();
	}

	if (!tn)
		return;

	tn = tn.getPrevSibling();
	can2::TreeCursor t = tn.insertNextSibling(ts.getText().c_str());
	t.setData(ts.getData());
	ts.deleteItem();
	t.select();
}


void CManualClusterDlg::OnRclickTreeCluster(NMHDR* pNMHDR, LRESULT* pResult)
{
	CPoint ptPos = GetMessagePos();
	m_tree.ScreenToClient(&ptPos);

	m_tClicked = m_tree.hitTest(ptPos);

	CMenu wndMenu;
	if (!wndMenu.LoadMenu(IDR_POPUP))
		return;

	// Get the File popup menu from the top level menu.
	CMenu* pMenu = wndMenu.GetSubMenu(1);

	// Convert the mouse location to screen coordinates before passing
	// it to the TrackPopupMenu() function.
	m_tree.ClientToScreen(&ptPos);

	// Display the File popup menu as a floating popup menu in the
	// client area of the main application window.
	pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON,
		ptPos.x,
		ptPos.y,
		this);    // owner is the main application window
	*pResult = 1;
}


void CManualClusterDlg::OnManualclusterSetclustername()
{
	if (!m_tClicked)
		return;

	DWORD dwData = m_tClicked.getData();
	if (dwData == 1)
	{
		CPromptNameDlg dlg(this);
		dlg.m_strName = m_tClicked.getText().c_str();

		if (dlg.DoModal() != IDOK)
			return;

		m_tClicked.setText(dlg.m_strName);
	}
	else
	{
		can2::RingAntenna* pa = (can2::RingAntenna*)dwData;
		if (!pa->isRingAntenna())
			return;

		CPromptNameDlg dlg(this);
		dlg.m_strName = pa->getName().c_str();

		if (dlg.DoModal() != IDOK)
			return;

		pa->m_alias = dlg.m_strName;
		m_tClicked.setText(pa->getName().c_str());
	}
}


void CManualClusterDlg::OnUpdateManualclusterSetclustername(CCmdUI* pCmdUI)
{
	if (m_tClicked)
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}
