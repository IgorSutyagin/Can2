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

#include "RingNode.h"

namespace can2
{
	#define IVSRINGDIST_CLASSNAME "IvsRingDist"

	#define IVSRINGDIST_SEL_CHANGED	0x10

	class RingDistCtrl : public CWnd
	{
	// Construction:
	public:
		RingDistCtrl();
		virtual ~RingDistCtrl();

	// Attributes:
	public:
		RingNode* m_node;
		CFont m_fontX;
		CFont m_fontY;


	// Operations:
	public:
		void onInitialUpdate(RingNode * node, CWnd * pParent);
		std::pair<int, int> getSel() const { return m_sel;	}
		void setSel(int nr, int nc) {
			m_sel = std::pair<int, int>(nr, nc);
			Invalidate();
		}

	protected:
		DECLARE_MESSAGE_MAP()
		afx_msg void OnPaint();
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

	// Implementation:
	protected:
		std::vector<CRect> m_cols; // Column titles
		std::vector<CRect> m_rows; // Row titles
		std::pair<int, int> m_sel; // Selected row, col
		LOGFONT m_lfx;
		LOGFONT m_lfy;
		void layout(CDC& dc);
		COLORREF getColor(double val, double minVal, double maxVal) const;
		std::pair<int, int> hitTest(CPoint pt) const;

	};
}