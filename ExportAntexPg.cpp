// ExportAntexPg.cpp : implementation file
//

#include "pch.h"
#include "Can2.h"
#include "afxdialogex.h"
#include "ExportAntexPg.h"


// CExportAntexPg dialog

IMPLEMENT_DYNAMIC(CExportAntexPg, CMFCPropertyPage)

CExportAntexPg::CExportAntexPg(CWnd* pParent /*=nullptr*/)
	: CPropertyPage(IDD_PROPPAGE_EXPORT_ANTEX)
	, m_strComment(_T(""))
	, m_strAntType(_T(""))
	, m_strAgency(_T(""))
	, m_strFile(_T(""))
	, m_strDome(_T(""))
	, m_strSn(_T(""))
	, m_tDate(COleDateTime::GetCurrentTime())
{

}

CExportAntexPg::~CExportAntexPg()
{
}

void CExportAntexPg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_COMMENT, m_strComment);
	DDX_Text(pDX, IDC_EDIT_ANT_TYPE, m_strAntType);
	DDX_Text(pDX, IDC_EDIT_AGENCY, m_strAgency);
	DDX_Text(pDX, IDC_EDIT_FILE, m_strFile);
	DDX_Text(pDX, IDC_EDIT_DOME, m_strDome);
	DDX_Text(pDX, IDC_EDIT_SN, m_strSn);
	DDX_Text(pDX, IDC_EDIT_ANT_NUM, m_strAntNum);
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER1, m_tDate);
}


BEGIN_MESSAGE_MAP(CExportAntexPg, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CExportAntexPg::OnClickedButtonBrowse)
END_MESSAGE_MAP()


// CExportAntexPg message handlers


void CExportAntexPg::OnClickedButtonBrowse()
{
	UpdateData(TRUE);
	CFileDialog dlg(FALSE, "atx", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "Antex files (*.atx)|*.atx|All files (*.*)|*.*||", this);
	if (dlg.DoModal() != IDOK)
		return;
	m_strFile = dlg.GetPathName();
	UpdateData(FALSE);
}
