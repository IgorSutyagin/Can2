#pragma once
#include "afxdialogex.h"


// CExportAntexPg dialog

class CExportAntexPg : public CPropertyPage
{
	DECLARE_DYNAMIC(CExportAntexPg)

public:
	CExportAntexPg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CExportAntexPg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROPPAGE_EXPORT_ANTEX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_strComment;
	CString m_strAntType;
	CString m_strAgency;
	CString m_strFile;
	CString m_strDome;
	CString m_strSn;
	CString m_strAntNum;
	afx_msg void OnClickedButtonBrowse();
	COleDateTime m_tDate;
};
