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
#pragma once
#include "afxdialogex.h"

#include "TreeCtrlEx.h"
#include "RingNode.h"

// CManualClusterDlg dialog

class CManualClusterDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CManualClusterDlg)

public:
	CManualClusterDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CManualClusterDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_MANUAL_CLUSTER };
#endif
	can2::RingNode* m_prn;
	can2::TreeCtrlEx m_tree;
	can2::TreeCursor m_tDrag;
	can2::TreeCursor m_tDrop;
	HCURSOR cursor_hand;
	HCURSOR cursor_arr;
	HCURSOR cursor_no;
	bool	isCursorArrow;
	bool m_bDragging;
	CImageList* m_pDragImage;	//Image list to be used for dragging
	CImageList m_imgList;
	std::vector<std::vector<can2::RingAntenna*>> m_cls;
	std::vector<std::string> m_names;
	can2::TreeCursor m_tClicked;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnTvnBegindragTreeCluster(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	void setDefaultCursor();
	can2::TreeCursor copyChildItem(can2::TreeCursor tItem, can2::TreeCursor tNewParent, HTREEITEM hAfter, bool bDelOrg);
	bool canDrop(can2::TreeCursor t) const;
	bool canDrag(can2::TreeCursor t) const;
	afx_msg void OnBnClickedButtonUp();
	afx_msg void OnBnClickedButtonDown();
	afx_msg void OnRclickTreeCluster(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnManualclusterSetclustername();
	afx_msg void OnUpdateManualclusterSetclustername(CCmdUI* pCmdUI);
	afx_msg void OnManualclusterRemove();
};
