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

#include "RingNode.h"

// CRingSourcePg dialog

class CRingSourcePg : public CPropertyPage
{
	DECLARE_DYNAMIC(CRingSourcePg)

public:
	CRingSourcePg();   // standard constructor
	virtual ~CRingSourcePg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROPPAGE_RING_SOURCE };
#endif
	std::vector<can2::AntexFile*> m_afs;
	std::shared_ptr<can2::RingNode> m_prn;
	CListCtrl m_lstSource;
	CString m_strTitle;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnLvnEndlabeleditListSource(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
public:
	afx_msg void OnClickedButtonAdd();
	afx_msg void OnClickedButtonRemove();
	virtual BOOL OnKillActive();
	afx_msg void OnBnClickedButtonUp();
	afx_msg void OnBnClickedButtonDown();
};
