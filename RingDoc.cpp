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
#include "pch.h"

#include "Can2.h"
#include "RingDoc.h"
#include "RingTabView.h"
#include "RingPcvView.h"
#include "RingPcoView.h"
#include "RingDifView.h"

///////////////////////////////////////////////////////////////////////////////////////////
// CRingDoc implementation

IMPLEMENT_DYNCREATE(CRingDoc, CDocument)

BEGIN_MESSAGE_MAP(CRingDoc, CDocument)
END_MESSAGE_MAP()


// CCan2Doc construction/destruction

CRingDoc::CRingDoc() noexcept : m_rn(nullptr), m_signal(can2::Gnss::esigInvalid)
{
	// TODO: add one-time construction code here

}

CRingDoc::~CRingDoc()
{
}

void CRingDoc::select(int nc1, int nc2)
{
	if (nc1 < 0 || nc2 < 0)
	{
		m_selAnt = nullptr;
	}
	else if (nc1 == nc2)
	{
		m_selAnt = m_rn->m_cls[nc1].atm;
	}
	else
	{
		can2::AntexAntenna* pa = (can2::AntexAntenna*)m_rn->m_cls[nc1].atm->subtract(m_rn->m_cls[nc2].atm.get());
		m_selAnt = std::shared_ptr<can2::AntexAntenna>(pa);
	}

	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		if (pView->IsKindOf(RUNTIME_CLASS(CRingTabView)))
		{
			CRingTabView* ptv = (CRingTabView*)pView;
			CRingPcvView* prv = ptv->m_pPcvView;
			if (m_selAnt != nullptr && (m_signal == can2::Gnss::esigInvalid || !m_selAnt->hasPcc(m_signal)) )
			{
				for (int ns = 0; ns < can2::Gnss::esigInvalid; ns++)
				{
					if (m_selAnt->hasPcc(ns))
					{
						m_signal = (can2::Gnss::Signal)ns;
						//prv->UpdateData(FALSE);
						break;
					}
				}
			}
			prv->regenAll();
			ptv->m_pPcoView->updateCurves();
			ptv->m_pDifView->updateCurves();
			//ptv->m_pStatView->updateCurves();
		}
	}
}


// End of CRingDoc implementation
///////////////////////////////////////////////////////////////////////////////////////////