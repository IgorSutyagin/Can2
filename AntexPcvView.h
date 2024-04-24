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


#include "Plot3d.h"

class CCan2Doc;
class CAntexPcvView;

class CAntexPcvCtrl : public can2::Plot3d
{
	// Construction:
public:
	CAntexPcvCtrl(CAntexPcvView* pv) : m_pView(pv) {}
	virtual ~CAntexPcvCtrl() {}

	// Attributes:
public:
	CAntexPcvView* m_pView;

	// Overrides:
public:
	virtual can2::OglSurface::Grid getGrid() const;
	virtual can2::Node* getNode() const;
	virtual can2::Gnss::Signal getSignal() const;
	virtual double getData(const can2::Node* node, can2::Gnss::Signal es, double x, double y) const;
	virtual void onViewChanged();

};

class CAntexPcvView : public CFormView
{
	DECLARE_DYNCREATE(CAntexPcvView)

protected:
	CAntexPcvView();           // protected constructor used by dynamic creation
	virtual ~CAntexPcvView();

	enum { IDD = IDD_RING_PCV };
	can2::Plot3dParams m_viewParams;
	CComboBox m_cmbSignal;
	CComboBox m_cmbViewAt;

	// Attributes:
public:
	CAntexPcvCtrl m_wndPlot;
	bool m_bNeedInit;
	CRect m_rCrt;

	CCan2Doc* GetDocument() const;

	// Operations:
public:
	void regenAll();
	void onViewChanged();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnInitialUpdate();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSelchangeComboSignal();
	afx_msg void OnSelchangeComboViewAt();
	afx_msg void OnEnChangeEditIso();
	afx_msg void OnChangeEditZMinMax();
	afx_msg void OnPolar();
	afx_msg void OnGrid0();
	afx_msg void OnRainbow();
};