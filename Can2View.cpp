
// Can2View.cpp : implementation of the CCan2View class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Can2.h"
#endif

#include "Can2Doc.h"
#include "Can2View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCan2View

IMPLEMENT_DYNCREATE(CCan2View, CView)

BEGIN_MESSAGE_MAP(CCan2View, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CCan2View::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CCan2View construction/destruction

CCan2View::CCan2View() noexcept
{
	// TODO: add construction code here

}

CCan2View::~CCan2View()
{
}

BOOL CCan2View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CCan2View drawing

void CCan2View::OnDraw(CDC* /*pDC*/)
{
	CCan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CCan2View printing


void CCan2View::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CCan2View::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CCan2View::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CCan2View::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CCan2View::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CCan2View::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CCan2View diagnostics

#ifdef _DEBUG
void CCan2View::AssertValid() const
{
	CView::AssertValid();
}

void CCan2View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCan2Doc* CCan2View::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCan2Doc)));
	return (CCan2Doc*)m_pDocument;
}
#endif //_DEBUG


// CCan2View message handlers
