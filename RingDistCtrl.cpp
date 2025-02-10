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
#include "RingDistCtrl.h"

using namespace can2;


RingDistCtrl::RingDistCtrl() : m_node(nullptr), m_sel(-1, -1)
{
	// Register the window class if it has not already been registered.
	WNDCLASS wndclass;
	HINSTANCE hInst = AfxGetInstanceHandle();

	if (!(::GetClassInfo(hInst, IVSRINGDIST_CLASSNAME, &wndclass)))
	{
		// Otherwise we need to register a new class
		wndclass.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = ::DefWindowProc;
		wndclass.cbClsExtra = wndclass.cbWndExtra = 0;
		wndclass.hInstance = hInst;
		wndclass.hIcon = NULL;
		wndclass.hCursor = LoadCursor(hInst, IDC_ARROW);
		wndclass.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = IVSRINGDIST_CLASSNAME;

		if (!AfxRegisterClass(&wndclass))
			AfxThrowResourceException();
	}
}

RingDistCtrl::~RingDistCtrl()
{
}

void RingDistCtrl::onInitialUpdate(RingNode * node, CWnd * pParent)
{
	m_node = node;

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
	CFont* pFont = pParent->GetFont();
	if (pFont != NULL)
	{
		pFont->GetLogFont(&lf);
	}
	else
	{
		NONCLIENTMETRICS ncm;
		ncm.cbSize = sizeof(NONCLIENTMETRICS);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
		memmove(&lf, &ncm.lfMessageFont, sizeof(LOGFONT));
	}


	lf.lfWeight = FW_SEMIBOLD;
	m_fontX.CreateFontIndirect(&lf);
	m_lfx = lf;
	lf.lfWeight = FW_SEMIBOLD;
	lf.lfOrientation = lf.lfEscapement = 900;
	m_fontY.CreateFontIndirect(&lf);
	m_lfy = lf;
}

void RingDistCtrl::layout(CDC& dc)
{
	m_cols.clear();
	m_rows.clear();
	if (m_node == nullptr)
		return;

	CRect rc;
	GetClientRect(rc);

	CFont* pOldFont = (CFont*)dc.SelectObject(&m_fontX);

	CSize sizeLeft(0, 0);
	for (auto itc = m_node->m_cls.begin(); itc != m_node->m_cls.end(); itc++)
	{
		const RingNode::Cluster& c = *itc;

		CRect r;
		dc.DrawText(c.atm->getName().c_str(), c.atm->getName().length(), r, DT_SINGLELINE | DT_CALCRECT | DT_NOCLIP);
		r.InflateRect(CSize(5, 5));

		if (sizeLeft.cx < r.Width())
			sizeLeft.cx = r.Width();

		if (sizeLeft.cy < r.Height())
			sizeLeft.cy = r.Height();
	}

	dc.SelectObject(&m_fontY);
	CSize sizeTop(0, 0);
	for (auto itc = m_node->m_cls.begin(); itc != m_node->m_cls.end(); itc++)
	{
		const RingNode::Cluster& c = *itc;

		CRect r;
		dc.DrawText(c.atm->getName().c_str(), c.atm->getName().length(), r, DT_SINGLELINE | DT_CALCRECT | DT_NOCLIP);
		r.InflateRect(CSize(5, 5));

		if (sizeTop.cx < r.Width())
			sizeTop.cx = r.Width();

		if (sizeTop.cy < r.Height())
			sizeTop.cy = r.Height();
	}
	dc.SelectObject(&m_fontX);
	int x = sizeLeft.cx;
	int y = 0; // sizeTop.cx;
	for (int nr = 0; nr < (int)m_node->m_cls.size(); nr++)
	{
		CSize colSize(0, 0);
		for (int nc = 0; nc < (int)m_node->m_cls.size(); nc++)
		{
			double d = nc == nr ? NAN : m_node->m_dists[std::pair<size_t, size_t>(std::min(nr, nc), std::max(nr, nc))];
			std::string str = isnan(d) ? "NAN" : stringFormat("%.2f", d);
			CRect r(x, y, x, y);
			dc.DrawText(str.c_str(), r, DT_SINGLELINE | DT_CALCRECT);
			r.InflateRect(5, 5);
			if (r.Width() > colSize.cx)
				colSize.cx = r.Width();
		}
		m_cols.push_back(CRect(x, y, x + colSize.cx, y + sizeTop.cx));
		x += colSize.cx;
	}

	y = sizeTop.cx;
	for (auto itc = m_node->m_cls.begin(); itc != m_node->m_cls.end(); itc++)
	{
		m_rows.push_back(CRect(0, y, sizeLeft.cx, y + sizeLeft.cy));
		y += sizeLeft.cy;
	}

	dc.SelectObject(pOldFont);
}

COLORREF RingDistCtrl::getColor(double val, double minVal, double maxVal) const
{
	if (isnan(val))
		return RGB(100, 100, 100);
	return colorrefRainbowColor(val, minVal, maxVal);
}

std::pair<int, int> RingDistCtrl::hitTest(CPoint pt) const
{
	for (int nr = 0; nr < (int)m_node->m_cls.size(); nr++)
	{
		for (int nc = 0; nc < (int)m_node->m_cls.size(); nc++)
		{
			CRect r(m_cols[nc].left, m_rows[nr].top, m_cols[nc].right, m_rows[nr].bottom);
			if (r.PtInRect(pt))
				return std::pair<int, int>(nr, nc);
		}
	}
	return std::pair<int, int>(-1, -1);
}

BEGIN_MESSAGE_MAP(RingDistCtrl, CWnd)
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	//ON_WM_MOUSEWHEEL()
	//ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void RingDistCtrl::OnPaint()
{
	CPaintDC dc(this);

	if (m_node == nullptr)
		return;

	CRect rc;
	GetClientRect(rc);

	CBrush back(RGB(255, 255, 255));
	dc.FillRect(rc, &back);
	layout(dc);

	dc.SetTextColor(RGB(0, 0, 0));
	dc.SetBkMode(TRANSPARENT);
	
	std::pair<double, double> valMinMax(DBL_MAX, -DBL_MAX);
	std::for_each(m_node->m_dists.begin(), m_node->m_dists.end(), [&](const auto& a) {
		if (a.second < valMinMax.first)
			valMinMax.first = a.second;
		if (a.second > valMinMax.second)
			valMinMax.second = a.second;
	});

	CFont* pOldFont = (CFont*)dc.SelectObject(&m_fontX);
	COLORREF clrText = dc.GetTextColor();

	dc.SelectObject(&m_fontX);
	for (int nr = 0; nr < (int)m_node->m_cls.size(); nr++)
	{
		RingNode::Cluster& c = m_node->m_cls[nr];

		dc.DrawFrameControl(m_rows[nr], DFC_BUTTON, DFCS_BUTTONPUSH);
		dc.DrawText(c.atm->getName().c_str(), m_rows[nr], DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOCLIP);
	}

	dc.SelectObject(&m_fontY);
	for (int nc = 0; nc < (int)m_node->m_cls.size(); nc++)
	{
		RingNode::Cluster& c = m_node->m_cls[nc];

		dc.DrawFrameControl(m_cols[nc], DFC_BUTTON, DFCS_BUTTONPUSH);
		CRect r = m_cols[nc];
		int n = dc.SetTextAlign(TA_CENTER|TA_BOTTOM);
		dc.TextOut((r.left+r.right)/2 - m_lfy.lfHeight/2, (r.top+r.bottom)/2, c.atm->getName().c_str(), c.atm->getName().length());
		dc.SetTextAlign(n);
	}

	CBrush br(RGB(255, 255, 255));
	CBrush* pOldBrush = (CBrush*)dc.SelectObject(&br);
	for (int nr=0; nr<(int)m_node->m_cls.size(); nr++)
	{
		for (int nc = 0; nc < (int)m_node->m_cls.size(); nc++)
		{
			double d = nc == nr ? NAN : m_node->m_dists[std::pair<size_t, size_t>(std::min(nr, nc), std::max(nr, nc))];
			std::string str = isnan(d) ? "" : stringFormat("%.2f", d);
			CRect r(m_cols[nc].left, m_rows[nr].top, m_cols[nc].right, m_rows[nr].bottom);
			if (std::pair<int, int>(nr, nc) == m_sel)
			{
				//dc.DrawFrameControl (r, DFC_BUTTON, DFCS_BUTTONPUSH|DFCS_HOT);
				dc.DrawFocusRect(r);
			}
			else
			{
				CBrush br(getColor(d, valMinMax.first, valMinMax.second));
				dc.SelectObject(&br);
				r.DeflateRect(1, 1);
				dc.Rectangle(r);
				r.InflateRect(1, 1);
			}
			dc.DrawFocusRect(r);
			dc.SelectObject(&m_fontX);
			dc.SetTextColor(clrText);
			dc.DrawText(str.c_str(), r, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOCLIP);

		}
	}

	dc.SelectObject(pOldBrush);
	dc.SelectObject(pOldFont);
}

void RingDistCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonDown(nFlags, point);

	m_sel = hitTest(point);
	Invalidate();

	SetCapture();
}

void RingDistCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonUp(nFlags, point);
	m_sel = hitTest(point);
	Invalidate();

	if (GetCapture() == this)
	{
		ReleaseCapture();
		CWnd* pParent = GetParent();
		if (pParent != NULL)
		{
			int nControlID = GetDlgCtrlID();
			NMHDR nm;
			memset(&nm, 0, sizeof(NMHDR));
			nm.hwndFrom = m_hWnd;
			nm.idFrom = nControlID;
			nm.code = IVSRINGDIST_SEL_CHANGED;
			pParent->SendMessage(WM_NOTIFY, nControlID, (LPARAM)&nm);
		}
	}

}

void RingDistCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	CWnd::OnRButtonDown(nFlags, point);

	//m_sel = hitTest(point);
	//Invalidate();

	//SetCapture();
}

void RingDistCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	//CWnd::OnRButtonUp(nFlags, point);
	m_sel = hitTest(point);
	Invalidate();

	if (m_sel.first == m_sel.second && m_sel.first >= 0)
	{
		int menuIndex = 6;
		CMenu menu;
		menu.LoadMenu(IDR_POPUP_EXPLORER);
		HMENU hMenu = menu.GetSubMenu(menuIndex)->Detach();
		CWnd* pParent = GetParent();
		CPoint pt = point;
		ClientToScreen(&pt);
		//pParent->ScreenToClient(&pt);
		theApp.GetContextMenuManager()->ShowPopupMenu(hMenu, pt.x, pt.y, pParent, TRUE);
	}

}

BOOL RingDistCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	SetCursor(::LoadCursor(NULL, IDC_ARROW));

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}


