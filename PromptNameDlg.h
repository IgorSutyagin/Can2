#pragma once
#include "afxdialogex.h"


// CPromptNameDlg dialog

class CPromptNameDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPromptNameDlg)

public:
	CPromptNameDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CPromptNameDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PROMPT_NAME };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_strName;
};
