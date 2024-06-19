// PromptTitleDlg.cpp : implementation file
//

#include "pch.h"
#include "Can2.h"
#include "afxdialogex.h"
#include "PromptTitleDlg.h"


// CPromptTitleDlg dialog

IMPLEMENT_DYNAMIC(CPromptTitleDlg, CDialogEx)

CPromptTitleDlg::CPromptTitleDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_PROMPT_TITLE, pParent)
	, m_strText(_T(""))
{

}

CPromptTitleDlg::~CPromptTitleDlg()
{
}

void CPromptTitleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_TITLE, m_strText);
}


BEGIN_MESSAGE_MAP(CPromptTitleDlg, CDialogEx)
END_MESSAGE_MAP()


// CPromptTitleDlg message handlers
