// ComboListCtrl.cpp : implementation file
//

#include "pch.h"
#include "can2.h"
#include "ComboListCtrl.h"


#define CTRL_C	0x3
#define CTRL_V	0x16
#define CTRL_X	0x18

/////////////////////////////////////////////////////////////////////////////
// CInPlaceCombo

CInPlaceCombo* CInPlaceCombo::m_pInPlaceCombo = NULL;

CInPlaceCombo::CInPlaceCombo()
{
	m_iRowIndex = -1;
	m_iColumnIndex = -1;
	m_bESC = FALSE;
}

CInPlaceCombo::~CInPlaceCombo()
{
}

BEGIN_MESSAGE_MAP(CInPlaceCombo, CComboBox)
	//{{AFX_MSG_MAP(CInPlaceCombo)
	ON_WM_CREATE()
	ON_WM_KILLFOCUS()
	ON_WM_CHAR()
	ON_CONTROL_REFLECT(CBN_CLOSEUP, OnCloseup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInPlaceCombo message handlers

int CInPlaceCombo::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CComboBox::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	// Set the proper font
	CFont* pFont = GetParent()->GetFont();
	SetFont(pFont);

	SetFocus();

	ResetContent();
	for (POSITION Pos_ = m_DropDownList.GetHeadPosition(); Pos_ != NULL;)
	{
		int i = AddString((LPCTSTR)(m_DropDownList.GetNext(Pos_)));
		if (i < (int)m_colors.size())
		{
			SetItemData(i, m_colors[i]);
		}
	}

	return 0;
}

BOOL CInPlaceCombo::PreTranslateMessage(MSG* pMsg)
{
	// If the message if for "Enter" or "Esc"
	// Do not process
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
		{
			::TranslateMessage(pMsg);
			::DispatchMessage(pMsg);
			// DO NOT process further
			return TRUE;
		}
	}

	return CComboBox::PreTranslateMessage(pMsg);
}

void CInPlaceCombo::DrawItem(LPDRAWITEMSTRUCT lpDrawItem)
{
	CDC dc;

	dc.Attach(lpDrawItem->hDC);
	COLORREF crOldTextColor = dc.GetTextColor();
	COLORREF crOldBkColor = dc.GetBkColor();

	COLORREF clr = lpDrawItem->itemID == -1 ? m_curSelColor : m_colors[lpDrawItem->itemID];
	
	if ((lpDrawItem->itemAction | ODA_SELECT) &&
		(lpDrawItem->itemState & ODS_SELECTED))
	{
		dc.SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
		dc.FillSolidRect(&lpDrawItem->rcItem, clr);
		dc.DrawFocusRect(&lpDrawItem->rcItem);
	}
	else
	{
		dc.FillSolidRect(&lpDrawItem->rcItem, clr);
	}

	dc.SetTextColor(crOldTextColor);
	dc.SetBkColor(crOldBkColor);
	dc.Detach();
}

void CInPlaceCombo::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	ASSERT(lpMeasureItemStruct->CtlType == ODT_COMBOBOX);

	if (lpMeasureItemStruct->itemID != (UINT)-1)
	{
		LPCTSTR lpszText = "123456"; // (LPCTSTR)lpMeasureItemStruct->itemData;
		ASSERT(lpszText != NULL);
		CSize sz;
		CDC* pDC = GetDC();

		sz = pDC->GetTextExtent(lpszText);

		ReleaseDC(pDC);

		lpMeasureItemStruct->itemHeight = sz.cy * 3 / 2;
	}
}


void CInPlaceCombo::OnKillFocus(CWnd* pNewWnd)
{
	CComboBox::OnKillFocus(pNewWnd);

	// Get the current selection text
	CString str;
	GetWindowText(str);
	int sel = GetCurSel();
	m_curSelColor = GetItemData(sel);

	// Send Notification to parent of ListView ctrl
	LV_DISPINFO dispinfo;
	dispinfo.hdr.hwndFrom = GetParent()->m_hWnd;
	dispinfo.hdr.idFrom = GetParent()->GetDlgCtrlID();
	dispinfo.hdr.code = LVN_ENDLABELEDIT;

	dispinfo.item.mask = m_colors.size() > 0 ? LVIF_PARAM : LVIF_TEXT;
	dispinfo.item.iItem = m_iRowIndex;
	dispinfo.item.iSubItem = m_iColumnIndex;
	if (m_colors.size() == 0)
	{
		dispinfo.item.pszText = m_bESC ? LPTSTR((LPCTSTR)m_strWindowText) : LPTSTR((LPCTSTR)str);
		dispinfo.item.cchTextMax = m_bESC ? m_strWindowText.GetLength() : str.GetLength();
	}
	else
	{
		dispinfo.item.pszText = LPTSTR((LPCTSTR)m_strWindowText);
		dispinfo.item.cchTextMax = m_strWindowText.GetLength();
	}
	dispinfo.item.lParam = m_curSelColor;

	GetParent()->GetParent()->SendMessage(WM_NOTIFY, GetParent()->GetDlgCtrlID(), (LPARAM)&dispinfo);

	// Close the control
	PostMessage(WM_CLOSE);
}

void CInPlaceCombo::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// If the key is "Esc" set focus back to the list control
	if (nChar == VK_ESCAPE || nChar == VK_RETURN)
	{
		if (nChar == VK_ESCAPE)
		{
			m_bESC = TRUE;
		}

		GetParent()->SetFocus();
		return;
	}

	CComboBox::OnChar(nChar, nRepCnt, nFlags);
}

void CInPlaceCombo::OnCloseup()
{
	// Set the focus to the parent list control
	GetParent()->SetFocus();
}

CInPlaceCombo* CInPlaceCombo::GetInstance()
{
	if (m_pInPlaceCombo == NULL)
	{
		m_pInPlaceCombo = new CInPlaceCombo;
	}
	return m_pInPlaceCombo;
}

void CInPlaceCombo::DeleteInstance()
{
	delete m_pInPlaceCombo;
	m_pInPlaceCombo = NULL;
}

BOOL CInPlaceCombo::ShowComboCtrl(DWORD dwStyle, const CRect &rCellRect, CWnd* pParentWnd, UINT uiResourceID,
	int iRowIndex, int iColumnIndex, CStringList* pDropDownList,
	CString strCurSelecetion /*= ""*/, int iCurSel /*= -1*/)
{

	m_iRowIndex = iRowIndex;
	m_iColumnIndex = iColumnIndex;
	m_bESC = FALSE;

	m_DropDownList.RemoveAll();
	m_DropDownList.AddTail(pDropDownList);

	BOOL bRetVal = TRUE;

	if (-1 != iCurSel)
	{
		GetLBText(iCurSel, m_strWindowText);
	}
	else if (!strCurSelecetion.IsEmpty())
	{
		m_strWindowText = strCurSelecetion;
	}

	if (NULL == m_pInPlaceCombo->m_hWnd)
	{
		bRetVal = m_pInPlaceCombo->Create(dwStyle, rCellRect, pParentWnd, uiResourceID);
	}

	SetCurSel(iCurSel);

	return bRetVal;
}

BOOL CInPlaceCombo::ShowComboCtrl(DWORD dwStyle, const CRect& rCellRect, CWnd* pParentWnd, UINT uiResourceID,
	int iRowIndex, int iColumnIndex, std::vector<COLORREF>& colors,	COLORREF curSelColor, int iCurSel /*= -1*/)
{

	m_iRowIndex = iRowIndex;
	m_iColumnIndex = iColumnIndex;
	m_bESC = FALSE;

	m_DropDownList.RemoveAll();
	m_colors = colors;
	for (int i = 0; i < (int)m_colors.size(); i++)
	{
		CString str;
		//str.Format("%lu", m_colors[i]);
		m_DropDownList.AddTail(str);
		if (m_colors[i] == curSelColor)
		{
			m_strWindowText = str;
			iCurSel = i;
		}
	}

	BOOL bRetVal = TRUE;

	m_curSelColor = curSelColor;
	//if (-1 != iCurSel)
	//{
	//	GetLBText(iCurSel, m_strWindowText);
	//}

	if (NULL == m_pInPlaceCombo->m_hWnd)
	{
		bRetVal = m_pInPlaceCombo->Create(dwStyle, rCellRect, pParentWnd, uiResourceID);
	}

	SetCurSel(iCurSel);

	return bRetVal;
}
/////////////////////////////////////////////////////////////////////////////
// CInPlaceEdit

CInPlaceEdit* CInPlaceEdit::m_pInPlaceEdit = NULL;

CInPlaceEdit::CInPlaceEdit()
{
	m_iRowIndex = -1;
	m_iColumnIndex = -1;
	m_bESC = FALSE;
	m_strValidChars.Empty();
}

CInPlaceEdit::~CInPlaceEdit()
{
}

BEGIN_MESSAGE_MAP(CInPlaceEdit, CEdit)
	//{{AFX_MSG_MAP(CInPlaceEdit)
	ON_WM_KILLFOCUS()
	ON_WM_CHAR()
	ON_MESSAGE(WM_PASTE, OnPaste)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInPlaceEdit message handlers

LRESULT CInPlaceEdit::OnPaste(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	//if (m_strValidChars.IsEmpty())
	//{
	//	return 0L;
	//}

	CString strFromClipboard;

	// get the text from clipboard
	if (OpenClipboard()) {
		HANDLE l_hData = GetClipboardData(CF_TEXT);
		if (NULL == l_hData) {
			return 0L;
		}

		char *l_pBuffer = (char*)GlobalLock(l_hData);
		if (NULL != l_pBuffer) {
			strFromClipboard = l_pBuffer;
		}

		GlobalUnlock(l_hData);
		CloseClipboard();
	}

	// Validate the characters before pasting 
	//for (int iCounter_ = 0; iCounter_ < strFromClipboard.GetLength(); iCounter_++)
	//{
	//	if (-1 == m_strValidChars.Find(strFromClipboard.GetAt(iCounter_)))
	//	{
	//		return 0L;
	//	}
	//}

	//let the individual control handle other processing
	CEdit::Default();

	return 0L;
}

void CInPlaceEdit::OnKillFocus(CWnd* pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);

	// TODO: Add your message handler code here

	// Get the text in the edit ctrl
	CString strEdit;
	GetWindowText(strEdit);

	// Send Notification to parent of edit ctrl
	LV_DISPINFO dispinfo;
	dispinfo.hdr.hwndFrom = GetParent()->m_hWnd;
	dispinfo.hdr.idFrom = GetParent()->GetDlgCtrlID();
	dispinfo.hdr.code = LVN_ENDLABELEDIT;

	dispinfo.item.mask = LVIF_TEXT;
	dispinfo.item.iItem = m_iRowIndex;
	dispinfo.item.iSubItem = m_iColumnIndex;
	dispinfo.item.pszText = m_bESC ? LPTSTR((LPCTSTR)m_strWindowText) : LPTSTR((LPCTSTR)strEdit);
	dispinfo.item.cchTextMax = m_bESC ? m_strWindowText.GetLength() : strEdit.GetLength();

	GetParent()->GetParent()->SendMessage(WM_NOTIFY, GetParent()->GetDlgCtrlID(), (LPARAM)&dispinfo);

	PostMessage(WM_CLOSE);
}

void CInPlaceEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	if ((m_strValidChars.IsEmpty()) || ((-1 != m_strValidChars.Find(static_cast<TCHAR> (nChar))) ||
		(nChar == VK_BACK) || (nChar == CTRL_C) || (nChar == CTRL_V) || (nChar == CTRL_X)))
	{
		CEdit::OnChar(nChar, nRepCnt, nFlags);
	}
	else
	{
		MessageBeep(MB_ICONEXCLAMATION);
		return;
	}
}

BOOL CInPlaceEdit::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (WM_KEYDOWN == pMsg->message && (VK_ESCAPE == pMsg->wParam || VK_RETURN == pMsg->wParam))
	{
		if (VK_ESCAPE == pMsg->wParam)
		{
			m_bESC = TRUE;
		}

		GetParent()->SetFocus();
		return TRUE;
	}

	return CEdit::PreTranslateMessage(pMsg);
}

int CInPlaceEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CEdit::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO: Add your specialized creation code here
	// Set the proper font
	CFont* pFont = GetParent()->GetFont();
	SetFont(pFont);

	ShowWindow(SW_SHOW);
	SetWindowText(m_strWindowText);
	SetSel(0, -1);
	SetFocus();


	return 0;
}

CInPlaceEdit* CInPlaceEdit::GetInstance()
{
	if (m_pInPlaceEdit == NULL)
	{
		m_pInPlaceEdit = new CInPlaceEdit;
	}
	return m_pInPlaceEdit;
}

void CInPlaceEdit::DeleteInstance()
{
	delete m_pInPlaceEdit;
	m_pInPlaceEdit = NULL;
}

BOOL CInPlaceEdit::ShowEditCtrl(DWORD dwStyle, const RECT &rCellRect, CWnd* pParentWnd,
	UINT uiResourceID, int iRowIndex, int iColumnIndex,
	CString& strValidChars, CString& rstrCurSelection)
{
	m_iRowIndex = iRowIndex;
	m_iColumnIndex = iColumnIndex;
	m_strValidChars = strValidChars;
	m_strWindowText = rstrCurSelection;
	m_bESC = FALSE;

	if (NULL == m_pInPlaceEdit->m_hWnd)
	{
		return m_pInPlaceEdit->Create(dwStyle, rCellRect, pParentWnd, uiResourceID);
	}

	return TRUE;
}

// CComboListCtrl
//#defines
#define FIRST_COLUMN				0
#define MIN_COLUMN_WIDTH			10
#define MAX_DROP_DOWN_ITEM_COUNT	10

IMPLEMENT_DYNAMIC(CComboListCtrl, CListCtrl)

CComboListCtrl::CComboListCtrl()
{
	m_ComboSupportColumnsList.RemoveAll();
	m_ReadOnlyColumnsList.RemoveAll();
	m_strValidEditCtrlChars.Empty();
	m_dwEditCtrlStyle = ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_LEFT | ES_NOHIDESEL;
	m_dwDropDownCtrlStyle = WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL |
		CBS_DROPDOWNLIST | CBS_DISABLENOSCROLL;
}

CComboListCtrl::~CComboListCtrl()
{
	CInPlaceCombo::DeleteInstance();
	CInPlaceEdit::DeleteInstance();
}


CInPlaceCombo* CComboListCtrl::ShowInPlaceList(int iRowIndex, int iColumnIndex, CStringList& rComboItemsList,
	CString strCurSelecetion /*= ""*/, int iSel /*= -1*/)
{
	// The returned obPointer should not be saved

	// Make sure that the item is visible
	if (!EnsureVisible(iRowIndex, TRUE))
	{
		return NULL;
	}

	if (rComboItemsList.IsEmpty())
		return nullptr;

	// Make sure that iColumnIndex is valid 
	CHeaderCtrl* pHeader = static_cast<CHeaderCtrl*> (GetDlgItem(FIRST_COLUMN));

	int iColumnCount = pHeader->GetItemCount();

	if (iColumnIndex >= iColumnCount || GetColumnWidth(iColumnIndex) < MIN_COLUMN_WIDTH)
	{
		return NULL;
	}

	// Calculate the rectangle specifications for the combo box
	CRect obCellRect(0, 0, 0, 0);
	CalculateCellRect(iColumnIndex, iRowIndex, obCellRect);

	int iHeight = obCellRect.Height();
	int iCount = rComboItemsList.GetCount();

	iCount = (iCount < MAX_DROP_DOWN_ITEM_COUNT) ?
		iCount + MAX_DROP_DOWN_ITEM_COUNT : (MAX_DROP_DOWN_ITEM_COUNT + 1);

	obCellRect.bottom += iHeight * iCount;

	// Create the in place combobox
	CInPlaceCombo* pInPlaceCombo = CInPlaceCombo::GetInstance();
	pInPlaceCombo->ShowComboCtrl(m_dwDropDownCtrlStyle, obCellRect, this, 0, iRowIndex, iColumnIndex, &rComboItemsList,
		strCurSelecetion, iSel);

	return pInPlaceCombo;
}

CInPlaceCombo* CComboListCtrl::ShowInPlaceList(int iRowIndex, int iColumnIndex, std::vector<COLORREF>& colors, COLORREF curSelColor, int iSel /*= -1*/)
{
	// The returned obPointer should not be saved

	// Make sure that the item is visible
	if (!EnsureVisible(iRowIndex, TRUE))
	{
		return NULL;
	}

	if (colors.size() == 0)
		return nullptr;

	// Make sure that iColumnIndex is valid 
	CHeaderCtrl* pHeader = static_cast<CHeaderCtrl*> (GetDlgItem(FIRST_COLUMN));

	int iColumnCount = pHeader->GetItemCount();

	if (iColumnIndex >= iColumnCount || GetColumnWidth(iColumnIndex) < MIN_COLUMN_WIDTH)
	{
		return NULL;
	}

	// Calculate the rectangle specifications for the combo box
	CRect obCellRect(0, 0, 0, 0);
	CalculateCellRect(iColumnIndex, iRowIndex, obCellRect);

	int iHeight = obCellRect.Height();
	int iCount = colors.size();

	iCount = (iCount < MAX_DROP_DOWN_ITEM_COUNT) ?
		iCount + MAX_DROP_DOWN_ITEM_COUNT : (MAX_DROP_DOWN_ITEM_COUNT + 1);

	obCellRect.bottom += iHeight * iCount;

	// Create the in place combobox
	CInPlaceCombo* pInPlaceCombo = CInPlaceCombo::GetInstance();
	pInPlaceCombo->ShowComboCtrl(m_dwDropDownCtrlStyle| CBS_OWNERDRAWFIXED, obCellRect, this, 0, iRowIndex, iColumnIndex, colors, curSelColor, iSel);

	return pInPlaceCombo;
}

CInPlaceEdit* CComboListCtrl::ShowInPlaceEdit(int iRowIndex, int iColumnIndex, CString& rstrCurSelection)
{
	// Create an in-place edit control
	CInPlaceEdit* pInPlaceEdit = CInPlaceEdit::GetInstance();

	CRect obCellRect(0, 0, 0, 0);
	CalculateCellRect(iColumnIndex, iRowIndex, obCellRect);

	pInPlaceEdit->ShowEditCtrl(m_dwEditCtrlStyle, obCellRect, this, 0,
		iRowIndex, iColumnIndex,
		m_strValidEditCtrlChars, rstrCurSelection);

	return pInPlaceEdit;
}

bool CComboListCtrl::HitTestEx(CPoint &obPoint, int* pRowIndex, int* pColumnIndex, bool * pbCheckBox) const
{
	if (!pRowIndex || !pColumnIndex || !pbCheckBox)
	{
		return false;
	}

	// Get the row index
	*pRowIndex = HitTest(obPoint, NULL);

	if (pColumnIndex)
	{
		*pColumnIndex = 0;
	}

	// Make sure that the ListView is in LVS_REPORT
	if ((GetWindowLong(m_hWnd, GWL_STYLE) & LVS_TYPEMASK) != LVS_REPORT)
	{
		return false;
	}

	bool bCheckBox = GetExtendedStyle() & LVS_EX_CHECKBOXES ? true : false;
	int nCheckBoxWidth = 0;
	if (bCheckBox)
	{
		CRect rb;
		GetItemRect(0, &rb, LVIR_BOUNDS);
		CRect rl;
		GetItemRect(0, &rl, LVIR_LABEL);
		nCheckBoxWidth = rl.left - rb.left;
	}

	// Get the number of columns
	CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);

	int iColumnCount = pHeader->GetItemCount();

	// Get bounding rect of item and check whether obPoint falls in it.
	CRect obCellRect;
	GetItemRect(*pRowIndex, &obCellRect, LVIR_BOUNDS);

	if (obCellRect.PtInRect(obPoint))
	{
		// Now find the column
		for (*pColumnIndex = 0; *pColumnIndex < iColumnCount; (*pColumnIndex)++)
		{
			int iColWidth = GetColumnWidth(*pColumnIndex);
			if (*pColumnIndex == 0 && bCheckBox)
			{
				if (obPoint.x >= obCellRect.left && obPoint.x <= (obCellRect.left + nCheckBoxWidth))
				{
					*pbCheckBox = true;
					return true;
				}

				obCellRect.left += nCheckBoxWidth;
				iColWidth -= nCheckBoxWidth;
			}

			if (obPoint.x >= obCellRect.left && obPoint.x <= (obCellRect.left + iColWidth))
			{
				return true;
			}
			obCellRect.left += iColWidth;
		}
	}
	return false;
}

void CComboListCtrl::SetComboColumns(int iColumnIndex, bool bSet /*= true*/)
{
	// If the Column Index is not present && Set flag is false
	// Then do nothing 
	// If the Column Index is present && Set flag is true
	// Then do nothing
	POSITION Pos = m_ComboSupportColumnsList.Find(iColumnIndex);

	// If the Column Index is not present && Set flag is true
	// Then Add to list
	if ((NULL == Pos) && bSet)
	{
		m_ComboSupportColumnsList.AddTail(iColumnIndex);
	}

	// If the Column Index is present && Set flag is false
	// Then Remove from list
	if ((NULL != Pos) && !bSet)
	{
		m_ComboSupportColumnsList.RemoveAt(Pos);
	}
}

void CComboListCtrl::SetReadOnlyColumns(int iColumnIndex, bool bSet /*= true*/)
{
	// If the Column Index is not present && Set flag is false
	// Then do nothing 
	// If the Column Index is present && Set flag is true
	// Then do nothing
	POSITION Pos = m_ReadOnlyColumnsList.Find(iColumnIndex);

	// If the Column Index is not present && Set flag is true
	// Then Add to list
	if ((NULL == Pos) && bSet)
	{
		m_ReadOnlyColumnsList.AddTail(iColumnIndex);
	}

	// If the Column Index is present && Set flag is false
	// Then Remove from list
	if ((NULL != Pos) && !bSet)
	{
		m_ReadOnlyColumnsList.RemoveAt(Pos);
	}
}

bool CComboListCtrl::IsReadOnly(int iColumnIndex)
{
	if (m_ReadOnlyColumnsList.Find(iColumnIndex))
	{
		return true;
	}

	return false;
}

bool CComboListCtrl::IsCombo(int iColumnIndex)
{
	if (m_ComboSupportColumnsList.Find(iColumnIndex))
	{
		return true;
	}

	return false;
}

void CComboListCtrl::CalculateCellRect(int iColumnIndex, int iRowIndex, CRect& robCellRect)
{
	GetItemRect(iRowIndex, &robCellRect, LVIR_BOUNDS);

	CRect rcClient;
	GetClientRect(&rcClient);

	if (robCellRect.right > rcClient.right)
	{
		robCellRect.right = rcClient.right;
	}

	ScrollToView(iColumnIndex, robCellRect);
}

void CComboListCtrl::SetValidEditCtrlCharacters(CString &rstrValidCharacters)
{
	m_strValidEditCtrlChars = rstrValidCharacters;
}

void CComboListCtrl::EnableHScroll(bool bEnable /*= true*/)
{
	if (bEnable)
	{
		m_dwDropDownCtrlStyle |= WS_HSCROLL;
	}
	else
	{
		m_dwDropDownCtrlStyle &= ~WS_HSCROLL;
	}
}

void CComboListCtrl::EnableVScroll(bool bEnable /*= true*/)
{
	if (bEnable)
	{
		m_dwDropDownCtrlStyle |= WS_VSCROLL;
	}
	else
	{
		m_dwDropDownCtrlStyle &= ~WS_VSCROLL;
	}
}

int CComboListCtrl::getSelRow()
{
	for (int i = 0; i < GetItemCount(); i++)
	{
		if (GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED)
			return i;
	}
	return -1;
}

void CComboListCtrl::setSelRow(int nr)
{
	for (int i = 0; i < GetItemCount(); i++)
	{
		if (i == nr)
			SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
		else
			SetItemState(i, 0, LVIS_SELECTED);
	}
}

void CComboListCtrl::ScrollToView(int iColumnIndex, /*int iOffSet, */CRect& robCellRect)
{
	// Now scroll if we need to expose the column
	CRect rcClient;
	GetClientRect(&rcClient);

	int iColumnWidth = GetColumnWidth(iColumnIndex);

	// Get the column iOffset
	int iOffSet = 0;
	for (int iIndex_ = 0; iIndex_ < iColumnIndex; iIndex_++)
	{
		iOffSet += GetColumnWidth(iIndex_);
	}

	// If x1 of cell rect is < x1 of ctrl rect or
	// If x1 of cell rect is > x1 of ctrl rect or **Should not ideally happen**
	// If the width of the cell extends beyond x2 of ctrl rect then
	// Scroll

	CSize obScrollSize(0, 0);

	if (((iOffSet + robCellRect.left) < rcClient.left) ||
		((iOffSet + robCellRect.left) > rcClient.right))
	{
		obScrollSize.cx = iOffSet + robCellRect.left;
	}
	else if ((iOffSet + robCellRect.left + iColumnWidth) > rcClient.right)
	{
		obScrollSize.cx = iOffSet + robCellRect.left + iColumnWidth - rcClient.right;
	}

	Scroll(obScrollSize);
	robCellRect.left -= obScrollSize.cx;

	// Set the width to the column width
	robCellRect.left += iOffSet;
	robCellRect.right = robCellRect.left + iColumnWidth;
}

BEGIN_MESSAGE_MAP(CComboListCtrl, CListCtrl)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_NOTIFY_REFLECT_EX(LVN_ENDLABELEDIT, &CComboListCtrl::OnLvnEndlabeledit)
	ON_NOTIFY_REFLECT(LVN_BEGINLABELEDIT, &CComboListCtrl::OnLvnBeginlabeledit)
END_MESSAGE_MAP()



// CComboListCtrl message handlers




void CComboListCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (GetFocus() != this)
	{
		SetFocus();
	}

	CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CComboListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (GetFocus() != this)
	{
		SetFocus();
	}

	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}


void CComboListCtrl::OnLButtonDown(UINT iFlags, CPoint obPoint)
{

	int iColumnIndex = -1;
	int iRowIndex = -1;

	// Get the current column and row
	bool bCheckBox = false;
	if (!HitTestEx(obPoint, &iRowIndex, &iColumnIndex, &bCheckBox))
	{
		return;
	}

	CListCtrl::OnLButtonDown(iFlags, obPoint);

	if (bCheckBox)
		return;

	// If column is not read only then
	// If the SHIFT or CTRL key is down call the base class
	// Check the high bit of GetKeyState to determine whether SHIFT or CTRL key is down
	if ((GetKeyState(VK_SHIFT) & 0x80) || (GetKeyState(VK_CONTROL) & 0x80))
	{
		return;
	}

	// Get the current selection before creating the in place combo box
	CString strCurSelection = GetItemText(iRowIndex, iColumnIndex);

	if (-1 != iRowIndex)
	{
		UINT flag = LVIS_FOCUSED;

		if ((GetItemState(iRowIndex, flag) & flag) == flag)
		{
			// Add check for LVS_EDITLABELS
			if (GetWindowLong(m_hWnd, GWL_STYLE) & LVS_EDITLABELS)
			{
				// Send Notification to parent of ListView ctrl
				/*
				LV_DISPINFO dispinfo;
				dispinfo.hdr.hwndFrom = m_hWnd;
				dispinfo.hdr.idFrom = GetDlgCtrlID();
				dispinfo.hdr.code = LVN_BEGINLABELEDIT;

				dispinfo.item.mask = LVIF_TEXT;
				dispinfo.item.iItem = iRowIndex;
				dispinfo.item.iSubItem = iColumnIndex;
				dispinfo.item.pszText = nullptr;
				dispinfo.item.cchTextMax = 0;

				int ret = GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&dispinfo);
				if (ret)
					return;
					*/

				// If combo box is supported
				// Create and show the in place combo box
				if (IsCombo(iColumnIndex))
				{
					//CStringList obComboItemsList;
					ComboInfo comboInfo;
					comboInfo.pListCtrl = this;
					comboInfo.col = iColumnIndex;
					comboInfo.row = iRowIndex;
					
					GetParent()->SendMessage(WM_SET_ITEMS, (WPARAM)0, (LPARAM)&comboInfo);

					CInPlaceCombo* pInPlaceComboBox = (comboInfo.colors.size() > 0) ?
						ShowInPlaceList(iRowIndex, iColumnIndex, comboInfo.colors, comboInfo.curSelColor) :
						ShowInPlaceList(iRowIndex, iColumnIndex, comboInfo.lstStrings, strCurSelection);
					//ASSERT(pInPlaceComboBox);

					// Set the selection to previous selection
					if (pInPlaceComboBox != nullptr)
						pInPlaceComboBox->SelectString(-1, strCurSelection);
				}
				// If combo box is not read only
				// Create and show the in place edit control
				else if (!IsReadOnly(iColumnIndex))
				{
					CInPlaceEdit* pInPlaceEdit = ShowInPlaceEdit(iRowIndex, iColumnIndex, strCurSelection);
				}
			}
		}
	}
}


BOOL CComboListCtrl::OnLvnEndlabeledit(NMHDR *pNMHDR, LRESULT *pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	// TODO: Add your control notification handler code here

	// Update the item text with the new text
	SetItemText(pDispInfo->item.iItem, pDispInfo->item.iSubItem, pDispInfo->item.pszText);

	GetParent()->SendMessage(WM_VALIDATE, GetDlgCtrlID(), (LPARAM)pDispInfo);

	*pResult = 0;
	return FALSE;
}


void CComboListCtrl::OnLvnBeginlabeledit(NMHDR *pNMHDR, LRESULT *pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	// TODO: Add your control notification handler code here
	if (IsReadOnly(pDispInfo->item.iSubItem))
	{
		*pResult = 1;
		return;
	}

	*pResult = 0;
	return;
}
