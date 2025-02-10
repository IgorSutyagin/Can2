#pragma once


#include "RingNode.h"
#include "Plot2d.h"

class CRingNaPcvView : public CFormView
{
	DECLARE_DYNCREATE(CRingNaPcvView)

protected:
	CRingNaPcvView();           // protected constructor used by dynamic creation
	virtual ~CRingNaPcvView();

public:
	enum { IDD = IDD_RING_NA_PCV };
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
	BOOL m_useRo;
	CComboBox m_cmbSignal;
	int m_nPcvType;
	CComboBox m_cmbRef;
	CComboBox m_cmbTest;

	double m_yMax;
	double m_yMin;
	double m_xMin;
	double m_xMax;
	BOOL m_byAuto;
	BOOL m_bxAuto;
	int m_yDivs;
	int m_xDivs;
	BOOL m_bLegend;
	BOOL m_bBands;

	CRingDoc* GetDocument() const;
	CRingBar* getRingBar() const;
	void updateCurves();

	void enableControls();

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
	Reference getRef();
	Reference getTest();
	void fillRef();

	void fillSignals();
	can2::Gnss::Signal getSignal();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnInitialUpdate();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClickedCheckUseRo();
	afx_msg void OnSelchangeComboSignal();

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

