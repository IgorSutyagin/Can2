#pragma once


// CComboListCtrl
class CInPlaceCombo;
class CInPlaceEdit;

class CInPlaceEdit : public CEdit
{

public:

	// Implementation

	// Returns the instance of the class
	static CInPlaceEdit* GetInstance();

	// Deletes the instance of the class
	static void DeleteInstance();

	// Creates the Windows edit control and attaches it to the object
	// Shows the edit ctrl
	BOOL ShowEditCtrl(DWORD dwStyle, const RECT& rCellRect, CWnd* pParentWnd,
		UINT uiResourceID, int iRowIndex, int iColumnIndex,
		CString& strValidChars, CString& rstrCurSelection);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInPlaceEdit)
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL


	// Attributes

protected:
	// Generated message map functions
	//{{AFX_MSG(CInPlaceEdit)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	LRESULT OnPaste(WPARAM wParam, LPARAM lParam);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:

	// Implementation

	// Constructor
	CInPlaceEdit();

	// Hide the copy constructor and operator =
	CInPlaceEdit(CInPlaceEdit&) {}

	CInPlaceEdit operator=(CInPlaceEdit) {}

	// Destructor
	virtual ~CInPlaceEdit();

	// Attributes

	// Index of the item in the list control
	int m_iRowIndex;

	// Index of the subitem in the list control
	int m_iColumnIndex;

	// To indicate whether ESC key was pressed
	BOOL m_bESC;

	// Valid characters
	CString m_strValidChars;

	// Singleton instance
	static CInPlaceEdit* m_pInPlaceEdit;

	// Previous string value in the edit control
	CString m_strWindowText;
};

class CInPlaceCombo : public CComboBox
{
public:

	// Implementation

	// Returns the instance of the class
	static CInPlaceCombo* GetInstance();

	// Deletes the instance of the class
	static void DeleteInstance();

	// Creates the Windows combo control and attaches it to the object, if needed and shows the combo ctrl
	BOOL ShowComboCtrl(DWORD dwStyle, const CRect& rCellRect, CWnd* pParentWnd, UINT uiResourceID,
		int iRowIndex, int iColumnIndex, CStringList* pDropDownList, CString strCurSelecetion = "", int iCurSel = -1);
	// Creates the Windows combo control and attaches it to the object, if needed and shows the combo ctrl
	BOOL ShowComboCtrl(DWORD dwStyle, const CRect& rCellRect, CWnd* pParentWnd, UINT uiResourceID,
		int iRowIndex, int iColumnIndex, std::vector<COLORREF>& colors, COLORREF curSelColor, int iCurSel = -1);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInPlaceCombo)
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);

protected:

	// Generated message map functions
	//{{AFX_MSG(CInPlaceCombo)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnCloseup();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	// Implementation
	// Constructor
	CInPlaceCombo();

	// Hide the copy constructor and operator =
	CInPlaceCombo(CInPlaceCombo&) {}

	CInPlaceCombo operator=(CInPlaceCombo) {}

	// Destructor
	virtual ~CInPlaceCombo();

	// Attributes

	// Index of the item in the list control
	int m_iRowIndex;

	// Index of the subitem in the list control
	int m_iColumnIndex;

	// To indicate whether ESC key was pressed
	BOOL m_bESC;

	// Singleton instance
	static CInPlaceCombo* m_pInPlaceCombo;

	// Previous selected string value in the combo control
	CString m_strWindowText;

	// List of items to be shown in the drop down
	CStringList m_DropDownList;

	std::vector<COLORREF> m_colors;
	COLORREF m_curSelColor;
};

/////////////////////////////////////////////////////////////////////////////

#include "ListCtrlEx.h"

// User define message 
// This message is posted to the parent
// The message can be handled to make the necessary validations, if any
#define WM_VALIDATE		WM_USER + 0x7FFD

// User define message 
// This message is posted to the parent
// The message should be handled to spcify the items to the added to the combo
#define WM_SET_ITEMS	WM_USER + 0x7FFC


class CComboListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CComboListCtrl)

public:
	CComboListCtrl();
	virtual ~CComboListCtrl();

	// Sets/Resets the column which support the in place combo box
	void SetComboColumns(int iColumnIndex, bool bSet = true);

	// Sets/Resets the column which support the in place edit control
	void SetReadOnlyColumns(int iColumnIndex, bool bSet = true);

	// Sets the valid characters for the edit ctrl
	void SetValidEditCtrlCharacters(CString& rstrValidCharacters);

	// Sets the vertical scroll
	void EnableVScroll(bool bEnable = true);

	// Sets the horizontal scroll
	void EnableHScroll(bool bEnable = true);

	struct ComboInfo
	{
		CComboListCtrl * pListCtrl;
		int col;
		int row;
		std::vector<COLORREF> colors;
		COLORREF curSelColor;
		CStringList lstStrings; // String to fill combo
	};

	int getSelRow();
	void setSelRow(int nr);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnLvnEndlabeledit(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnBeginlabeledit(NMHDR *pNMHDR, LRESULT *pResult);

private:

	// Implementation

	// Returns the row & column index of the column on which mouse click event has occured
	bool HitTestEx(CPoint& rHitPoint, int* pRowIndex, int* pColumnIndex, bool * pbCheckBox) const;

	// Creates and displays the in place combo box
	CInPlaceCombo* ShowInPlaceList(int iRowIndex, int iColumnIndex, CStringList& rComboItemsList,
		CString strCurSelecetion = "", int iSel = -1);
	CInPlaceCombo* ShowInPlaceList(int iRowIndex, int iColumnIndex, std::vector<COLORREF>& colors,
		COLORREF curSelColor, int iSel = -1);

	// Creates and displays the in place edit control
	CInPlaceEdit* ShowInPlaceEdit(int iRowIndex, int iColumnIndex, CString& rstrCurSelection);

	// Calculates the cell rect
	void CalculateCellRect(int iColumnIndex, int iRowIndex, CRect& robCellRect);

	// Checks whether column supports in place combo box
	bool IsCombo(int iColumnIndex);

	// Checks whether column is read only
	bool IsReadOnly(int iColumnIndex);

	// Scrolls the list ctrl to bring the in place ctrl to the view
	void ScrollToView(int iColumnIndex, /*int iOffSet, */CRect& obCellRect);

	// Attributes

	// List of columns that support the in place combo box
	CList<int, int> m_ComboSupportColumnsList;

	// List of columns that are read only
	CList<int, int> m_ReadOnlyColumnsList;

	// Valid characters
	CString m_strValidEditCtrlChars;

	// The window style of the in place edit ctrl
	DWORD m_dwEditCtrlStyle;

	// The window style of the in place combo ctrl
	DWORD m_dwDropDownCtrlStyle;

};


