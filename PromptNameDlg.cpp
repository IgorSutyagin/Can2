// PromptNameDlg.cpp : implementation file
//

#include "pch.h"
#include "Can2.h"
#include "afxdialogex.h"
#include "PromptNameDlg.h"


// CPromptNameDlg dialog

IMPLEMENT_DYNAMIC(CPromptNameDlg, CDialogEx)

CPromptNameDlg::CPromptNameDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_PROMPT_NAME, pParent)
	, m_strName(_T(""))
{

}

CPromptNameDlg::~CPromptNameDlg()
{
}

void CPromptNameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_NAME, m_strName);
}


BEGIN_MESSAGE_MAP(CPromptNameDlg, CDialogEx)
END_MESSAGE_MAP()


// CPromptNameDlg message handlers
