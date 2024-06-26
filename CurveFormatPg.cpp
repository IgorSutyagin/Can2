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
#include "afxdialogex.h"
#include "CurveFormatPg.h"
#include "Tools.h"
#include "Settings.h"

// CCurveFormatPg dialog

IMPLEMENT_DYNAMIC(CCurveFormatPg, CMFCPropertyPage)

CCurveFormatPg::CCurveFormatPg()
	: CPropertyPage(IDD_PROPPAGE_CURVE_FORMAT), m_pPlot(nullptr)
{
}

CCurveFormatPg::~CCurveFormatPg()
{
}

void CCurveFormatPg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST_CURVES, m_lstCurves);
}


BEGIN_MESSAGE_MAP(CCurveFormatPg, CPropertyPage)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_CURVES, OnCustomdrawListCurves)
    ON_MESSAGE(WM_SET_ITEMS, PopulateComboList)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST_CURVES, &CCurveFormatPg::OnEndlabeleditListCurves)
END_MESSAGE_MAP()


// CCurveFormatPg message handlers


BOOL CCurveFormatPg::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	CListCtrlEx& lst = (CListCtrlEx&)m_lstCurves;
	while (lst.DeleteColumn(0));
	lst.AddColumn("ID", 0);
	lst.AddColumn("Name", 1);
	lst.AddColumn("Color", 2);
    m_lstCurves.SetComboColumns(2);
    m_lstCurves.SetReadOnlyColumns(0);
    m_lstCurves.SetReadOnlyColumns(1);
    m_lstCurves.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_AUTOSIZECOLUMNS);

    m_cis.resize(m_pPlot->m_curves.size());
	for (int i = 0; i < (int)m_pPlot->m_curves.size(); i++)
	{
        CurveInfo ci;
		can2::Curve2d* pc = m_pPlot->m_curves[i];
        ci.id = pc->m_id;
        ci.name = pc->m_name;
        ci.rgb = pc->m_rgb;
        m_cis[i] = ci;
        std::string strId = can2::stringFormat("%i", pc->m_id);
		lst.AddItem(i, 0, strId.c_str());
		lst.AddItem(i, 1, pc->m_name.c_str());
		lst.SetItemData(i, (DWORD_PTR)pc);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CCurveFormatPg::OnCustomdrawListCurves(NMHDR* pNMHDR, LRESULT* pResult)
{
    NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);

    // Take the default processing unless 
    // we set this to something else below.
    *pResult = CDRF_DODEFAULT;

    // First thing - check the draw stage. If it's the control's prepaint
    // stage, then tell Windows we want messages for every item.

    if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
    {
        *pResult = CDRF_NOTIFYITEMDRAW;
    }
    else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
    {
        // This is the notification message for an item.  We'll request
        // notifications before each subitem's prepaint stage.

        *pResult = CDRF_NOTIFYSUBITEMDRAW;
    }
    else if ((CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage)
    {
        if (2 == pLVCD->iSubItem)
        {
            //can2::Curve2d* pc = (can2::Curve2d*)pLVCD->nmcd.lItemlParam;
            pLVCD->clrTextBk = m_cis[pLVCD->nmcd.dwItemSpec].rgb;
            LVITEM rItem;
            int    nItem = static_cast<int>(pLVCD->nmcd.dwItemSpec);

            // Get the image index and state of this item.  Note that we need to
            // check the selected state manually.  The docs _say_ that the
            // item's state is in pLVCD->nmcd.uItemState, but during my testing
            // it was always equal to 0x0201, which doesn't make sense, since
            // the max CDIS_* constant in commctrl.h is 0x0100.

            ZeroMemory(&rItem, sizeof(LVITEM));
            rItem.mask = LVIF_IMAGE | LVIF_STATE;
            rItem.iItem = nItem;
            rItem.stateMask = LVIS_SELECTED;
            m_lstCurves.GetItem(&rItem);

            // If this item is selected, redraw the icon with its normal colors.
            if (rItem.state & LVIS_SELECTED)
            {
                CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
                CListCtrlEx& lst = (CListCtrlEx&)m_lstCurves;
                // Get the rect that holds the item's icon.
                CRect rc = lst.GetSubItemRect(nItem, 2, LVIR_LABEL);

                pDC->FillSolidRect (rc, pLVCD->clrTextBk);
                *pResult = CDRF_SKIPDEFAULT;
            }
            else
            {
                *pResult = CDRF_DODEFAULT;
            }
        }
        else
        {
            *pResult = CDRF_DODEFAULT;
        }
    }
    else if (CDDS_ITEMPOSTPAINT == pLVCD->nmcd.dwDrawStage)
    {
        // If this item is selected, re-draw the icon in its normal
        // color (not blended with the highlight color).

        LVITEM rItem;
        int    nItem = static_cast<int>(pLVCD->nmcd.dwItemSpec);

        // Get the image index and state of this item.  Note that we need to
        // check the selected state manually.  The docs _say_ that the
        // item's state is in pLVCD->nmcd.uItemState, but during my testing
        // it was always equal to 0x0201, which doesn't make sense, since
        // the max CDIS_* constant in commctrl.h is 0x0100.

        ZeroMemory(&rItem, sizeof(LVITEM));
        rItem.mask = LVIF_IMAGE | LVIF_STATE;
        rItem.iItem = nItem;
        rItem.stateMask = LVIS_SELECTED;
        m_lstCurves.GetItem(&rItem);

        // If this item is selected, redraw the icon with its normal colors.
        if (rItem.state & LVIS_SELECTED)
        {
            CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
            CRect rc;

            // Get the rect that holds the item's icon.
            m_lstCurves.GetItemRect(nItem, &rc, LVIR_BOUNDS);

            //pDC->DrawFocusRect(&rc);
            *pResult = CDRF_SKIPDEFAULT;
        }
    }
}

void CCurveFormatPg::OnOK()
{
    for (int i = 0; i < (int)m_cis.size(); i++)
    {
        m_pPlot->m_curves[i]->m_rgb = m_cis[i].rgb;
        m_pPlot->m_curves[i]->createPens();
        can2::gl_settings.plot2dColors[m_cis[i].name] = m_cis[i].rgb;
    }
    can2::gl_settings.save();

    CPropertyPage::OnOK();
}

LRESULT CCurveFormatPg::PopulateComboList(WPARAM wParam, LPARAM lParam)
{
	CComboListCtrl::ComboInfo* pci = (CComboListCtrl::ComboInfo*)lParam;
	// Get the Combobox window pointer
	CComboBox* pInPlaceCombo = static_cast<CComboBox*> (GetFocus());

	// Get the inplace combbox top left
	CRect obWindowRect;

	pInPlaceCombo->GetWindowRect(&obWindowRect);

	CPoint obInPlaceComboTopLeft(obWindowRect.TopLeft());

	// Get the active list
	// Get the control window rect
	// If the inplace combobox top left is in the rect then
	// The control is the active control
	pci->pListCtrl->GetWindowRect(&obWindowRect);

	int iColIndex = pci->col;

	CStringList* pComboList = &(pci->lstStrings);
	pComboList->RemoveAll();

	if (obWindowRect.PtInRect(obInPlaceComboTopLeft))
	{
		if (pci->pListCtrl == &m_lstCurves)
		{
			if (2 == iColIndex)
			{
				pci->colors = m_colors;
                pci->curSelColor = m_cis[pci->row].rgb;
			}
		}
	}

	return true;
}



void CCurveFormatPg::OnEndlabeleditListCurves(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVDISPINFO* pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	m_cis[pDispInfo->item.iItem].rgb = pDispInfo->item.lParam;
	m_lstCurves.Invalidate(TRUE);
	*pResult = 0;
}
