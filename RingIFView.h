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

#include "Plot2d.h"

class CRingDoc;

class CRingIFView : public CFormView
{
	DECLARE_DYNCREATE(CRingIFView)

protected:
	CRingIFView();           // protected constructor used by dynamic creation
	virtual ~CRingIFView();

public:
	enum { IDD = IDD_RING_IF_PCO };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
	bool m_bNeedInit;
	CRect m_rCrt;
	can2::Plot2d m_wndPlot;
	double m_eleMask;
	CFont m_font;
	BOOL m_bOffsetFromAntex;
	BOOL m_bMarker;
	CComboBox m_cmbPcvMode;

	double m_yMax;
	double m_yMin;
	double m_xMin;
	double m_xMax;
	BOOL m_byAuto;
	BOOL m_bxAuto;
	int m_yDivs;
	int m_xDivs;
	BOOL m_bOverlay;
	CString m_strOverlay;

	//int m_nCoord;
	can2::Gnss::Signal getSignal(); // Selected combination
	CComboBox m_cmbSignal;
	CComboBox m_cmbReference;
	enum RefType
	{
		ertCluster = 0,
		ertAntenna = 1,
		ertNone
	};
	struct Reference
	{
		Reference(RefType ert_, int n_) {
			ert = ert_, n = n_;
		}
		RefType ert; 
		int n; // Cluster number or Antenna number in RingNode depending on ert
	};
	std::vector <Reference> m_refs;
	Reference getReference();
	can2::Node::PcvMode getPcvMode();

	struct Show
	{
		Show() : pccS(FALSE), pccC(FALSE), pco (FALSE), u(TRUE), n(TRUE), e(TRUE), h(FALSE), pcv(FALSE), naPcv(FALSE) {}
		BOOL pccS;
		BOOL pccC;
		BOOL pco;
		BOOL e;
		BOOL n;
		BOOL u;
		BOOL h;
		BOOL naPcv;
		BOOL pcv;

		bool isNothing() const {
			return !pccS && !pccC && !pco && !e && !n && !u && !h && !naPcv && !pcv;
		}
	} m_show;

	struct PppItem
	{
		PppItem() : enu(NAN, NAN, NAN) {}
		std::string ant;
		std::string sys;
		std::string lab;
		can2::Point3d enu;
	};
	std::vector<PppItem> m_pppItems;
	bool selectPpp(const char * name, can2::Gnss::Signal sig, std::vector<PppItem>& pppItems) const;


	void fillSignals();
	void fillReferences();
	void fillPcvMode();

	CRingDoc* GetDocument() const;
	void updateCurves();
	void updateCurves2();
	void enableControls();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnInitialUpdate();
	afx_msg void OnClickedRadioPCC();
	afx_msg void OnRadioNorth();
	afx_msg void OnRadioUp();
	afx_msg void OnChangeEditEleMask();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	afx_msg void OnBnClickedCheckAuto();
	afx_msg void OnChangeEditXMinMax();
	afx_msg void OnChangeEditYMinMax();
	afx_msg void OnBnClickedButtonAddComment();
	afx_msg void OnBnClickedButtonRemoveComments();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonLoad();
	afx_msg void OnBnClickedCheckOverlay();

	afx_msg void OnBnClickedButtonSetPlotTitle();
	afx_msg void OnSelchangeComboSignal();
	afx_msg void OnSelchangeComboRef();

	afx_msg void OnCurveFormat();
};

