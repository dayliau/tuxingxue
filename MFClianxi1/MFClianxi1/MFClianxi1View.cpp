
// MFClianxi1View.cpp: CMFClianxi1View 类的实现
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "MFClianxi1.h"
#endif

#include "MFClianxi1Doc.h"
#include "MFClianxi1View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFClianxi1View

IMPLEMENT_DYNCREATE(CMFClianxi1View, CView)

BEGIN_MESSAGE_MAP(CMFClianxi1View, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMFClianxi1View::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_MDRAW, &CMFClianxi1View::OnMdraw)
END_MESSAGE_MAP()

// CMFClianxi1View 构造/析构

CMFClianxi1View::CMFClianxi1View() noexcept
{
	// TODO: 在此处添加构造代码

}
 
CMFClianxi1View::~CMFClianxi1View()
{
}

BOOL CMFClianxi1View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CMFClianxi1View 绘图

void CMFClianxi1View::OnDraw(CDC* pDC)
{
	CMFClianxi1Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码

	
	// TODO: add draw code for native data here	
	//自定义坐标系
	CRect rect;
	GetClientRect(&rect);
	pDC->SetMapMode(MM_ANISOTROPIC);//设置映射模式
	pDC->SetWindowExt(rect.Width(), rect.Height());//设置窗口
	pDC->SetViewportExt(rect.Width(), -2 * rect.Height());//设置视区:x轴水平向右，y轴垂直向上
	pDC->SetViewportOrg(rect.Width() / 2, rect.Height() / 2);//客户区中心为坐标系原点
	//绘制坐标轴
	CPen NewPen, * pOldPen;
	NewPen.CreatePen(PS_SOLID, 3, RGB(128, 128, 128));
	pOldPen = pDC->SelectObject(&NewPen);
	pDC->MoveTo(-rect.right / 2, 0);//绘制x轴
	pDC->LineTo(rect.right / 2, 0);
	pDC->MoveTo(rect.right / 2 - 10, 5);//绘制右箭头　
	pDC->LineTo(rect.right / 2, 0);
	pDC->LineTo(rect.right / 2 - 10, -5);
	pDC->LineTo(rect.right / 2, 0);
	pDC->TextOut(rect.right / 2 - 20, 30, L"x");
	pDC->MoveTo(0, -rect.bottom / 2);//绘制y轴
	pDC->LineTo(0, rect.bottom / 2);
	pDC->MoveTo(-5, rect.bottom / 2 - 10);//绘制上箭头
	pDC->LineTo(0, rect.bottom / 2);
	pDC->LineTo(5, rect.bottom / 2 - 10);
	pDC->LineTo(0, rect.bottom / 2);
	pDC->TextOut(-30, rect.bottom / 2 - 20, L"y");
	pDC->TextOut(-20, -10, L"O");
	//测试坐标系	
	pDC->TextOut(300, 200, L"点(300,200)位于第一象限");
	pDC->TextOut(-300, 200, L"点(-300,200)位于第二象限");
	pDC->TextOut(-300, -200, L"点(-300,-200)位于第三象限");
	pDC->TextOut(300, -200, L"点(300,-200)位于第四象限");
	pDC->Rectangle(CRect(50, 50, 100, 100));
	pDC->SelectObject(pOldPen);
	NewPen.DeleteObject();
}


// CMFClianxi1View 打印


void CMFClianxi1View::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CMFClianxi1View::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CMFClianxi1View::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CMFClianxi1View::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}

void CMFClianxi1View::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CMFClianxi1View::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CMFClianxi1View 诊断

#ifdef _DEBUG
void CMFClianxi1View::AssertValid() const
{
	CView::AssertValid();
}

void CMFClianxi1View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMFClianxi1Doc* CMFClianxi1View::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFClianxi1Doc)));
	return (CMFClianxi1Doc*)m_pDocument;
}
#endif //_DEBUG


// CMFClianxi1View 消息处理程序

void CMFClianxi1View::OnMdraw()
{
	// TODO: 在此添加命令处理程序代码
	CDC* pDC = GetDC();
	CPoint p0(50, 150), p1(300, 300);
	CPen NewPen, * pOldPen; //声明画笔对象
	NewPen.CreatePen(PS_SOLID, 1, RGB(0, 0, 255)); //选择蓝色画笔
	pOldPen = pDC->SelectObject(&NewPen); //CDC与画笔对象关联
	pDC->MoveTo(p0); //画线
	pDC->LineTo(p1); //画线
	pDC->SelectObject(pOldPen); //恢复原来画笔对象
	ReleaseDC(pDC);
}
