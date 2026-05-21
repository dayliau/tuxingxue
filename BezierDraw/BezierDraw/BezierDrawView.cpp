#include "pch.h"
#include "framework.h"
#include "BezierDraw.h"
#include "BezierDrawDoc.h"
#include "BezierDrawView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 点结构
struct CP2 {
    double x, y;
    CP2(double x = 0, double y = 0) : x(x), y(y) {}
};

// 全局变量
CP2 pp[10][10];
int n = 3;//三次贝塞尔曲线
CP2 pt[4];//存当前正在画的四个点
double angle = 0;//初始化旋转角度

// 31个控制点
CP2 originalPts[31] = {
    CP2(253, 374),CP2(256, 352),CP2(236, 334),CP2(189, 319),
    CP2(149, 301),CP2(121, 278),CP2(108, 223),CP2(122, 196),
    CP2(162, 163),CP2(225, 148),CP2(206, 135),CP2(152, 160),
    CP2(82, 151),CP2(118, 127),CP2(172, 120),CP2(231, 133),
    CP2(272, 121),CP2(303, 104),CP2(356, 103),CP2(332, 131),
    CP2(291, 135),CP2(239, 145),CP2(296, 148),CP2(341, 163),
    CP2(370, 195),CP2(381, 223),CP2(377, 273),CP2(350, 312),
    CP2(320, 339),CP2(292, 364),CP2(253, 374)
};

// 函数声明
void deCasteljau(double t, CP2* p);
void DrawBezierSegment(CDC* pDC, CP2 P[4]);
void DrawPeach(CDC* pDC, CP2 pts[]);

IMPLEMENT_DYNCREATE(CBezierDrawView, CView)

BEGIN_MESSAGE_MAP(CBezierDrawView, CView)
    ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CBezierDrawView::OnFilePrintPreview)
END_MESSAGE_MAP()

CBezierDrawView::CBezierDrawView() noexcept {}
CBezierDrawView::~CBezierDrawView() {}

BOOL CBezierDrawView::PreCreateWindow(CREATESTRUCT& cs) {
    return CView::PreCreateWindow(cs);
}

// 旋转 + 平移
void CBezierDrawView::OnDraw(CDC* pDC)
{
    CBezierDrawDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (!pDoc) return;
    //获取窗口大小，存在rect里
    CRect rect;
    GetClientRect(&rect);
    //定义逻辑坐标系大小和窗口大小一样，Y轴正方向原来是向下的，把Y轴方向翻转，并且把坐标原点移动到窗口左下角
    pDC->SetMapMode(MM_ANISOTROPIC);
    pDC->SetWindowExt(rect.Width(), rect.Height());
    pDC->SetViewportExt(rect.Width(), -rect.Height());
    pDC->SetViewportOrg(0, rect.Height());

    // 清屏，清除上一个步骤留下来的画面
    pDC->FillSolidRect(0, 0, rect.Width(), rect.Height(), RGB(255, 255, 255));
    //旋转角度
    angle += 0.05;

    // 计算窗口正中心坐标
    double centerX = rect.Width() / 2.0;
    double centerY = rect.Height() / 2.0;

    // 平移幅度，限制在窗口内，不碰边界
    //左右移动,左右各留出220的边界
    double moveRange = (rect.Width() / 2.0) - 220; 
    double moveX = moveRange * sin(angle * 0.3); 
    //创建一个新数组，存放旋转 + 平移后的新坐标
    CP2 transformedPts[31];
    for (int i = 0; i < 31; i++)
    {
        // 先平移：以桃子图形原始中心为基准移动，把桃子旋转点定义为（260,240）
        // 把桃子中心点移到坐标原点 (0,0)，使桃子绕自己的中心进行旋转
        double baseX = originalPts[i].x - 260; 
        double baseY = originalPts[i].y - 240;

        // 再旋转
        double rotatedX = baseX * cos(angle) - baseY * sin(angle);
        double rotatedY = baseX * sin(angle) + baseY * cos(angle);

        //最终点的 X 坐标 = 窗口中心 + 旋转后的偏移 + 左右摆动偏移
        transformedPts[i].x = centerX + rotatedX + moveX;
        transformedPts[i].y = centerY + rotatedY;
    }

    DrawPeach(pDC, transformedPts);

    Sleep(40);
    Invalidate(FALSE);
}
//贝赛尔曲线，n=3，四个点一组
void deCasteljau(double t, CP2* p) {
    for (int k = 0;k <= n;k++) pp[k][0] = p[k];//循环把四个点放在二维数组的第0列
    for (int r = 1;r <= n;r++)//4->3->2->1
        for (int i = 0;i <= n - r;i++) {//第r层的第几个点
            pp[i][r].x = (1 - t) * pp[i][r - 1].x + t * pp[i + 1][r - 1].x;
            pp[i][r].y = (1 - t) * pp[i][r - 1].y + t * pp[i + 1][r - 1].y;
        }
}

void DrawBezierSegment(CDC* pDC, CP2 P[4]) {
    pDC->MoveTo((int)P[0].x, (int)P[0].y);//把画笔移动到起点 P [0]
    for (int k = 0;k <= n;k++) pt[k] = P[k];//把 4 个控制点复制到 pt 数组里
    for (double t = 0; t <= 1; t += 0.01) {//t 从 0 → 1，每次加 0.01
        deCasteljau(t, pt);//调用上述函数，计算100个新点
        pDC->LineTo((int)pp[0][n].x, (int)pp[0][n].y);//连点画直线，连接上即为曲线
    }
}

void DrawPeach(CDC* pDC, CP2 pts[]) {
    DrawBezierSegment(pDC, &pts[0]);
    DrawBezierSegment(pDC, &pts[3]);
    DrawBezierSegment(pDC, &pts[6]);
    DrawBezierSegment(pDC, &pts[9]);
    DrawBezierSegment(pDC, &pts[12]);
    DrawBezierSegment(pDC, &pts[15]);
    DrawBezierSegment(pDC, &pts[18]);
    DrawBezierSegment(pDC, &pts[21]);
    DrawBezierSegment(pDC, &pts[24]);
    DrawBezierSegment(pDC, &pts[27]);
}

void CBezierDrawView::OnFilePrintPreview() { AFXPrintPreview(this); }
BOOL CBezierDrawView::OnPreparePrinting(CPrintInfo* pInfo) { return DoPreparePrinting(pInfo); }
void CBezierDrawView::OnBeginPrinting(CDC*, CPrintInfo*) {}
void CBezierDrawView::OnEndPrinting(CDC*, CPrintInfo*) {}

#ifdef _DEBUG
void CBezierDrawView::AssertValid() const { CView::AssertValid(); }
void CBezierDrawView::Dump(CDumpContext& dc) const { CView::Dump(dc); }
CBezierDrawDoc* CBezierDrawView::GetDocument() const {
    return (CBezierDrawDoc*)m_pDocument;
}
#endif