
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
#include "framework.h"
#include "mainfrm.h"
#include "FileView.h"
#include "Resource.h"
#include "Can2.h"
#include "AntexFile.h"
#include "AntexAntenna.h"
#include "RingSourcePg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileView

CFileView::CFileView() noexcept : m_eMode(emDef)
{
}

CFileView::~CFileView()
{
}

int CFileView::getImageIndex(const can2::Node* node) const
{
	if (node->isAntexFile())
		return 0;
	else if (node->isAntexAntenna())
		return 2;
	else if (node->isRing())
		return 0;
	else if (node->isRingAntenna())
		return 2;

	return 0;
}

can2::Gnss::Signal CFileView::getSignal(can2::TreeCursor t) const
{
	if (isRealNode(t))
		return can2::Gnss::esigInvalid;

	int es = t.getData();
	if (can2::Gnss::G01 <= es && es < can2::Gnss::esigInvalid)
		return (can2::Gnss::Signal)es;
	return can2::Gnss::esigInvalid;
}

can2::Node* CFileView::getNode(can2::TreeCursor t, can2::Gnss::Signal * pes) const
{
	if (pes != nullptr)
		*pes = can2::Gnss::esigInvalid;
	while (t)
	{
		if (isRealNode(t))
			return (can2::Node *)t.getData();

		if (pes != nullptr && *pes == can2::Gnss::esigInvalid)
			*pes = getSignal(t);

		t = t.getParent();
	}

	return nullptr;
}

BEGIN_MESSAGE_MAP(CFileView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_WM_LBUTTONDOWN()
	ON_COMMAND(ID_PROPERTIES, OnProperties)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_ANTEXFILE_REMOVE, &CFileView::OnAntexfileRemove)
	ON_COMMAND(ID_SUBTRACT, &CFileView::OnSubtract)
	ON_COMMAND(ID_GNSSSIGNAL_VIEWPCV, &CFileView::OnGnsssignalViewpcv)
	ON_COMMAND(ID_GNSSSIGNAL_VIEWPCVRMS, &CFileView::OnGnsssignalViewpcvrms)
	ON_COMMAND(ID_RING_VIEW, &CFileView::OnRingView)
	ON_COMMAND(ID_RING_PROPERTIES, &CFileView::OnRingProperties)
	ON_COMMAND(ID_RING_SAVEAS, &CFileView::OnRingSaveas)
	ON_COMMAND(ID_RING_REMOVE, &CFileView::OnRingRemove)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar message handlers

int CFileView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create view:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	m_wndFileView.m_pFileView = this;

	if (!m_wndFileView.Create(dwViewStyle, rectDummy, this, 4))
	{
		TRACE0("Failed to create file view\n");
		return -1;      // fail to create
	}

	// Load view images:
	m_FileViewImages.Create(IDB_FILE_VIEW, 16, 0, RGB(255, 0, 255));
	m_wndFileView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);
	m_wndToolBar.LoadToolBar(IDR_EXPLORER, 0, 0, TRUE /* Is locked */);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	// Fill in some static tree view data (dummy code, nothing magic here)
	AdjustLayout();

	return 0;
}

void CFileView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CFileView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_eMode == emSubtract)
	{
		can2::TreeCtrlEx& tree = getTree();
		can2::TreeCursor t = tree.hitTest(point, &nFlags);
		if (isRealNode(t) && t != m_tClicked)
		{
			t.select();
			CWaitCursor wc;


			m_eMode = emDef;

			can2::Node* pPlus = (can2::Node*)m_tClicked.getData();
			can2::Node* pMinus = (can2::Node*)t.getData();
			can2::Node* pRes = pPlus->subtract(pMinus);
			if (pRes == nullptr)
				return;
			theApp.addNode(pRes);
			createNode(pRes);
			getMainFrame()->setStatusText("Subtracted");
		}

		return;
	}

	CDockablePane::OnLButtonDown(nFlags, point);
}

void CFileView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	can2::TreeCtrlEx& tree = getTree();

	if (pWnd != &tree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	CPoint ptTree = point;
	tree.ScreenToClient(&ptTree);
	m_tClicked = tree.hitTest(ptTree);
	if (m_tClicked)
	{
		tree.SelectItem(m_tClicked);
	}

	tree.SetFocus();

	auto getMenuIndex = [](can2::TreeCursor t) {
		
		if (!t)
			return 1;
		DWORD_PTR d = t.getData();
		if (can2::Gnss::G01 <= d && d < can2::Gnss::esigInvalid)
			return 4;
		if (d < 1000)
			return 1;
		can2::Node* p = (can2::Node*)d;
		if (p->isAntexFile())
			return 2;
		else if (p->isAntexAntenna())
			return 3;
		else if (p->isRing())
			return 5;
		return 1;
	};

	int menuIndex = getMenuIndex(m_tClicked);
	CMenu menu;
	menu.LoadMenu(IDR_POPUP_EXPLORER);
	HMENU hMenu = menu.GetSubMenu(menuIndex)->Detach();
	theApp.GetContextMenuManager()->ShowPopupMenu(hMenu, point.x, point.y, this, TRUE);
}

void CFileView::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndFileView.SetWindowPos(nullptr, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CFileView::OnProperties()
{
	AfxMessageBox(_T("Properties...."));

}

void CFileView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	m_wndFileView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CFileView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndFileView.SetFocus();
}

void CFileView::OnChangeVisualStyle()
{
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_EXPLORER_24 : IDR_EXPLORER, 0, 0, TRUE /* Locked */);

	m_FileViewImages.DeleteImageList();

	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_FILE_VIEW_24 : IDB_FILE_VIEW;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("Can't load bitmap: %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	m_FileViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_FileViewImages.Add(&bmp, RGB(255, 0, 255));

	m_wndFileView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);
}


void CFileView::createNode(can2::Node * node, can2::TreeCursor tParent)
{
	can2::TreeCursor tNode = tParent ? 
		tParent.addTail(node->getName().c_str(), getImageIndex(node)) : 
		getTree().insertItem(node->getName().c_str(), getImageIndex(node), getImageIndex(node));

	tNode.setData((DWORD_PTR)node);

	for (int nc = 0; nc < node->childs(); nc++)
	{
		can2::Node* child = node->getChild(nc);
		createNode(child, tNode);
	}

	populateNode(tNode, node);
}

void CFileView::populateNode(can2::TreeCursor& tNode, can2::Node* node)
{
	tNode.setText(node->getName().c_str());
	for (int nf = can2::Gnss::G01; nf < can2::Gnss::esigInvalid; nf++)
	{
		can2::TreeCursor tOld = tNode.findChild((DWORD_PTR)nf);
		if (!node->hasPcc(nf))
		{
			if (tOld)
				tOld.deleteItem();
			continue;
		}
		else if (!tOld)
		{
			can2::TreeCursor tPcv = tNode.addTail(can2::Gnss::getSignalName(nf), 2);
			tPcv.setData((DWORD_PTR)nf);
		}
	}
}

void CFileView::OnAntexfileRemove()
{
	if (!m_tClicked)
		return;
	can2::Node* pn = (can2::Node*)m_tClicked.getData();
	if (!pn->isAntexFile())
		return;

	theApp.removeNode(pn);
	m_tClicked.deleteItem();
}


void CFileView::OnSubtract()
{
	m_eMode = emSubtract;
	getMainFrame()->setStatusText("Select an item to subtract");
}


void CFileView::OnGnsssignalViewpcv()
{
	can2::Gnss::Signal es = can2::Gnss::esigInvalid;
	can2::Node* node = getNode(m_tClicked, &es);
	if (node == nullptr)
		return;

	theApp.showPcv(node, es);
}


void CFileView::OnGnsssignalViewpcvrms()
{
	// TODO: Add your command handler code here
}


void CFileView::OnRingView()
{
	can2::Gnss::Signal es = can2::Gnss::esigInvalid;
	can2::Node* node = getNode(m_tClicked, &es);
	if (node == nullptr || !node->isRing())
		return;

	theApp.showRingView((can2::RingNode *)node);
}


void CFileView::OnRingProperties()
{
	if (!m_tClicked)
		return;
	can2::RingNode* prn = (can2::RingNode*)m_tClicked.getData();
	if (!prn->isRing())
		return;

	CPropertySheet dlg;

	CRingSourcePg pgRing;

	*(pgRing.m_prn) = *prn;

	dlg.AddPage(&pgRing);

	if (dlg.DoModal() != IDOK)
		return;

	*prn = *pgRing.m_prn;

	m_tClicked.removeAllChilds();
	for (int i = 0; i < prn->childs(); i++)
	{
		can2::Node* p = prn->getChild(i);
		createNode(p, m_tClicked);
	}
}


void CFileView::OnRingSaveas()
{
	if (!m_tClicked)
		return;
	can2::RingNode* prn = (can2::RingNode*)m_tClicked.getData();
	if (!prn->isRing())
		return;

	CFileDialog dlg(FALSE, "rng", prn->m_strSourceFile.c_str(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "Ring files (*.rng)|*.rng|All files (*.*)|*.*||", this);
	if (dlg.DoModal() != IDOK)
		return;

	std::ofstream ofs(dlg.GetPathName(), std::ios_base::binary|std::ios_base::out);
	if (!ofs.good())
	{
		AfxMessageBox("Can't save to selected file");
		return;
	}


	prn->m_strSourceFile = dlg.GetPathName();
	{
		can2::Archive ar(ofs, can2::Archive::store);

		prn->serialize(ar);
	}

	m_tClicked.setText(dlg.GetPathName());
}


void CFileView::OnRingRemove()
{
	if (!m_tClicked)
		return;
	can2::Node* pn = (can2::Node*)m_tClicked.getData();
	if (!pn->isRing())
		return;

	theApp.removeNode(pn);
	m_tClicked.deleteItem();
}
