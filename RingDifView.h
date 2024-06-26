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

class CRingDoc;
class CRingBar;

#include "RingNode.h"
#include "Plot2d.h"

// CRingPcoView form view

class CRingDifView : public CFormView
{
	DECLARE_DYNCREATE(CRingDifView)

protected:
	CRingDifView();           // protected constructor used by dynamic creation
	virtual ~CRingDifView();

public:
	enum { IDD = IDD_RING_DIF };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
	bool m_bNeedInit;
	CRect m_rCrt;
	can2::Plot2d m_wndPlot;
	CFont m_font;
	CString m_strFileName;

	double m_yMax;
	double m_yMin;
	double m_xMin;
	double m_xMax;
	BOOL m_byAuto;
	BOOL m_bxAuto;
	int m_yDivs;
	int m_xDivs;

	CRingDoc* GetDocument() const;
	CRingBar* getRingBar() const;
	void updateCurves();
	void updateSpreadCurves();

	int m_nRingOffsetMode;
	BOOL m_bSimpleMode;
	int m_nWhatToPlot;
	BOOL m_bClusterSpread;
	BOOL m_bClusterRects;

	void enableControls();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnInitialUpdate();
	afx_msg void OnClickedRadioOffsetMode();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	afx_msg void OnBnClickedCheckAuto();
	afx_msg void OnChangeEditXMinMax();
	afx_msg void OnChangeEditYMinMax();
	afx_msg void OnCbnSelchangeComboDataType();
	afx_msg void OnBnClickedButtonAddComment();
	afx_msg void OnBnClickedButtonRemoveComments();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonLoad();
	afx_msg void OnCurveFormat();
};


