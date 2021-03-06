// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface
// (the "Fluent UI") and is provided only as referential material to supplement the
// Microsoft Foundation Classes Reference and related electronic documentation
// included with the MFC C++ library software.
// License terms to copy, use or distribute the Fluent UI are available separately.
// To learn more about our Fluent UI licensing program, please visit
// https://go.microsoft.com/fwlink/?LinkId=238214.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// ShadeControlView.cpp : implementation of the CShadeControlView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "ShadeControl.h"
#endif

#include "ShadeControlDoc.h"
#include "ShadeControlView.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#endif


// CShadeControlView

IMPLEMENT_DYNCREATE(CShadeControlView, CView)

BEGIN_MESSAGE_MAP(CShadeControlView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CShadeControlView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_WM_TIMER()
END_MESSAGE_MAP()

// CShadeControlView construction/destruction

CShadeControlView::CShadeControlView()
{
	// TODO: add construction code here

}

CShadeControlView::~CShadeControlView()
{
}

BOOL CShadeControlView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CShadeControlView drawing

void CShadeControlView::OnDraw(CDC* /*pDC*/)
{
	CShadeControlDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CShadeControlView printing


void CShadeControlView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CShadeControlView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CShadeControlView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CShadeControlView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CShadeControlView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CShadeControlView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CShadeControlView diagnostics

#ifdef _DEBUG
void CShadeControlView::AssertValid() const
{
	CView::AssertValid();
}

void CShadeControlView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CShadeControlDoc* CShadeControlView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CShadeControlDoc)));
	return (CShadeControlDoc*)m_pDocument;
}
#endif //_DEBUG


// CShadeControlView message handlers



void CShadeControlView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	CRect rect;
	GetClientRect(rect);
	rect += CRect(-2, -2, -2, -2);
	m_list.Create(WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_REPORT, rect, this, 0);
	m_list.SetExtendedStyle(m_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_GRIDLINES);
	m_list.InsertColumn(0, LPCTSTR(_T("楼层")), LVCFMT_CENTER, 80, -1);
	m_list.InsertColumn(1, LPCTSTR(_T("百叶位置")), LVCFMT_LEFT, 120, -1);
	m_list.InsertColumn(2, LPCTSTR(_T("通讯状态")), LVCFMT_CENTER, 120, -1);
	m_list.InsertColumn(3, LPCTSTR(_T("当前角度")), LVCFMT_CENTER, 120, -1);
	m_list.InsertColumn(4, LPCTSTR(_T("控制模式")), LVCFMT_CENTER, 120, -1);	
	m_list.InsertColumn(5, LPCTSTR(_T("是否启用")), LVCFMT_CENTER, 120, -1);	
	m_list.InsertColumn(6, LPCTSTR(_T("RS485地址")), LVCFMT_CENTER, 120, -1);
	m_list.InsertColumn(7, LPCTSTR(_T("更新时间")), LVCFMT_CENTER, 200, -1);
	m_list.ShowWindow(SW_SHOW);
	/*
	time_t timet;
	time(&timet);
	struct tm t;
	localtime_s(&t, &timet);
	std::string str_time = boost::str(boost::format("%d-%02d-%02d %02d:%02d:%02d") \
		% (t.tm_year + 1900) % (t.tm_mon + 1) % t.tm_mday \
		% t.tm_hour % t.tm_min % t.tm_sec);
		*/
	std::string str_time = get_current_time();
	std::string str;
	int i = 0;	
	for (auto controller : controller_list)//控制器列表
	{
		if (controller.addr == 0)
			continue; //不显示地址为0的控制器
		//str = boost::str(boost::format("%d层") % static_cast<int>(controller.floor));
		str = floor_name[controller.floor - 3];
		m_list.InsertItem(LVIF_TEXT | LVIF_STATE, i, str.c_str(), 0, LVIS_SELECTED, 0, 0);
		/*
		m_list.SetItemText(i, 1, controller.name.c_str());
		controller.mode == 0 ? str = "正常" : str = "通讯出错";
		m_list.SetItemText(i, 2, str.c_str());
		str = boost::str(boost::format("%d") % static_cast<int>(controller.angle));
		m_list.SetItemText(i, 3, str.c_str());		
		controller.mode == 0 ? str = "自动" : str = "手动";
		m_list.SetItemText(i, 4, str.c_str());		
		controller.enable == 1 ? str = "启用" : str = "禁用";
		m_list.SetItemText(i, 5, str.c_str());
		str = boost::str(boost::format("%02d") % static_cast<int>(controller.addr));
		m_list.SetItemText(i, 6, str.c_str());
		m_list.SetItemText(i, 7, str_time.c_str());
		*/
		i++;
	}
	SetTimer(1, 5000, NULL);
}


void CShadeControlView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if (m_list.m_hWnd == NULL)
		return;
	CRect rect;
	GetClientRect(rect);
	rect += CRect(-2, -2, -2, -2);
	m_list.MoveWindow(rect, TRUE);
}


void CShadeControlView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	std::string str_time = get_current_time();
	std::string str;
	int i = 0;
	for (auto controller : controller_list)//依次访问控制器列表中的每个对象
	{
		if (controller.addr == 0)
			continue; //不显示地址为0的控制器
					  //str = boost::str(boost::format("%d层") % static_cast<int>(controller.floor));
		str = floor_name[controller.floor - 3];		
		m_list.SetItemText(i, 1, controller.name.c_str());
		controller.state == RS485_OK ? str = "正常" : str = "通讯出错";
		m_list.SetItemText(i, 2, str.c_str());
		str = boost::str(boost::format("%d") % static_cast<int>(controller.angle));
		m_list.SetItemText(i, 3, str.c_str());
		controller.mode == 0 ? str = "自动" : str = "手动";
		m_list.SetItemText(i, 4, str.c_str());
		controller.enable == 1 ? str = "启用" : str = "禁用";
		m_list.SetItemText(i, 5, str.c_str());
		str = boost::str(boost::format("%02d") % static_cast<int>(controller.addr));
		m_list.SetItemText(i, 6, str.c_str());
		m_list.SetItemText(i, 7, controller.timestamp.c_str());
		i++;
	}
	CView::OnTimer(nIDEvent);
}
