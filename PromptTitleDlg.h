#pragma once
#include "afxdialogex.h"


// CPromptTitleDlg dialog

class CPromptTitleDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPromptTitleDlg)

public:
	CPromptTitleDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CPromptTitleDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PROMPT_TITLE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_strText;
};
