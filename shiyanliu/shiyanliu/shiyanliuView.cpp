// shiyanliuView.cpp: CshiyanliuView 类的实现

#include "pch.h"
#include "framework.h"
#ifndef SHARED_HANDLERS
#include "shiyanliu.h"
#endif
#include "shiyanliuDoc.h"
#include "shiyanliuView.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
IMPLEMENT_DYNCREATE(CshiyanliuView, CView)
BEGIN_MESSAGE_MAP(CshiyanliuView, CView)
    ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CshiyanliuView::OnFilePrintPreview)
    ON_WM_CONTEXTMENU()
    ON_WM_RBUTTONUP()
END_MESSAGE_MAP()
CshiyanliuView::CshiyanliuView() noexcept
{
    // TODO: 在此处添加构造代码
}
CshiyanliuView::~CshiyanliuView()
{
}
BOOL CshiyanliuView::PreCreateWindow(CREATESTRUCT& cs)
{
    return CView::PreCreateWindow(cs);
}

// CshiyanliuView 绘图
// CshiyanliuView 绘图代码概念大纲
// CshiyanliuView 绘图代码概念大纲
// CshiyanliuView 绘图
// CshiyanliuView 绘图
// CshiyanliuView 绘图
// CshiyanliuView 绘图
void CshiyanliuView::OnDraw(CDC* pDC)
{
    CshiyanliuDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (!pDoc) return;

    //光照与材质参数 (表10-3: 金材质)
    // 环境光强度 (RGB)，均匀照亮物体，无方向，模拟漫反射的基础光
    double Iar = 200, Iag = 200, Iab = 200;
    // 平行光（方向光）强度 (RGB)，平行光（太阳光），有固定方向，产生高光和明暗过渡
    double Ipr = 250, Ipg = 250, Ipb = 250;

    double ka_R = 0.247, ka_G = 0.200, ka_B = 0.075;// 环境光反射系数
    double kd_R = 0.752, kd_G = 0.606, kd_B = 0.226;// 漫反射系数
    double ks_R = 0.628, ks_G = 0.556, ks_B = 0.366;// 镜面反射系数
    int n = 50; // 高光指数

    // 方向光（平行光）
    Vector3 LightDir(1.0, 1.0, 1.0);//定义一束光线的方向：从(0, 0, 0) 指向(1, 1, 1)
    LightDir.Normalize();
    Vector3 ViewDir(0.0, 0.0, 1.0);

    // 圆柱几何参数
    //很多小平面拼起来假装是圆
    int slices = 200;// 圆柱有多圆（200段=超级圆）
    double radius = 100.0; // 圆柱有多粗（半径100）
    double height = 200.0;// 圆柱有多高（高200）

    CRect rect;
    GetClientRect(&rect);//获取当前窗口的大小和范围
    int centerX = rect.Width() / 2; // 水平中心点
    int centerY = rect.Height() / 2;// 垂直中心点

    // 倾斜角度：20度俯视，-30度侧视
    double angleX = 20.0 * PI / 180.0;
    double angleY = -30.0 * PI / 180.0;
    double cosX = cos(angleX), sinX = sin(angleX);
    double cosY = cos(angleY), sinY = sin(angleY);

    // 坐标旋转宏
#define ROTATE_3D(v) { \
        double tX = (v).x * cosY + (v).z * sinY; \
        double tZ1 = -(v).x * sinY + (v).z * cosY; \
        (v).x = tX; \
        double tY = (v).y * cosX - tZ1 * sinX; \
        double tZ2 = (v).y * sinX + tZ1 * cosX; \
        (v).y = tY; \
        (v).z = tZ2; \
    }

    //绘制顶部圆盘 ---
    Vector3 topNormal(0, 1, 0);//topNormal：顶面法向量（顶面朝向）
    ROTATE_3D(topNormal);//
    topNormal.Normalize();
    //DotProduct向量点积
    if (DotProduct(topNormal, ViewDir) >= 0) {//ViewDir：视线方向（眼睛看过去的方向）
        Vector3 H(LightDir.x + ViewDir.x, LightDir.y + ViewDir.y, LightDir.z + ViewDir.z);//Vector3 H(xyz相加)
        H.Normalize();//归一化
        double L_dot_N = max(0.0, DotProduct(LightDir, topNormal));
        double H_dot_N = max(0.0, DotProduct(H, topNormal));

        int R = ka_R * Iar + kd_R * Ipr * L_dot_N + ks_R * Ipr * pow(H_dot_N, n);
        int G = ka_G * Iag + kd_G * Ipg * L_dot_N + ks_G * Ipg * pow(H_dot_N, n);
        int B = ka_B * Iab + kd_B * Ipb * L_dot_N + ks_B * Ipb * pow(H_dot_N, n);

        POINT* topPts = new POINT[slices];
        for (int i = 0; i < slices; i++) {
            double theta = (2.0 * PI * i) / slices;
            Vector3 pt(radius * cos(theta), height / 2, radius * sin(theta));
            ROTATE_3D(pt);
            topPts[i].x = (int)(pt.x + centerX);
            topPts[i].y = (int)(-pt.y + centerY);
        }

        CBrush brush(RGB(R, G, B)); CPen pen(PS_SOLID, 1, RGB(R, G, B));
        CBrush* pOldB = pDC->SelectObject(&brush); CPen* pOldP = pDC->SelectObject(&pen);
        pDC->Polygon(topPts, slices);
        pDC->SelectObject(pOldB); pDC->SelectObject(pOldP);
        delete[] topPts;
    }

    // --- 3. 绘制底面圆盘 ---
    Vector3 bottomNormal(0, -1, 0);
    ROTATE_3D(bottomNormal);
    bottomNormal.Normalize();

    if (DotProduct(bottomNormal, ViewDir) >= 0) {
        Vector3 H(LightDir.x + ViewDir.x, LightDir.y + ViewDir.y, LightDir.z + ViewDir.z);
        H.Normalize();
        double L_dot_N = max(0.0, DotProduct(LightDir, bottomNormal));
        double H_dot_N = max(0.0, DotProduct(H, bottomNormal));

        int R = ka_R * Iar + kd_R * Ipr * L_dot_N + ks_R * Ipr * pow(H_dot_N, n);
        int G = ka_G * Iag + kd_G * Ipg * L_dot_N + ks_G * Ipg * pow(H_dot_N, n);
        int B = ka_B * Iab + kd_B * Ipb * L_dot_N + ks_B * Ipb * pow(H_dot_N, n);

        POINT* bottomPts = new POINT[slices];
        for (int i = 0; i < slices; i++) {
            double theta = (2.0 * PI * i) / slices;
            Vector3 pt(radius * cos(theta), -height / 2, radius * sin(theta));
            ROTATE_3D(pt);
            bottomPts[i].x = (int)(pt.x + centerX);
            bottomPts[i].y = (int)(-pt.y + centerY);
        }

        CBrush brush(RGB(R, G, B)); CPen pen(PS_SOLID, 1, RGB(R, G, B));
        CBrush* pOldB = pDC->SelectObject(&brush); CPen* pOldP = pDC->SelectObject(&pen);
        pDC->Polygon(bottomPts, slices);
        pDC->SelectObject(pOldB); pDC->SelectObject(pOldP);
        delete[] bottomPts;
    }

    // 绘制圆柱侧面 (使用平行光，恢复明亮的金色高光带) ---
    for (int i = 0; i < slices; i++)//循环：一圈画 150 个小矩形。把圆柱侧面分成 150 个小矩形，一个一个画
    {
        double theta1 = (2.0 * PI * i) / slices;//当前小矩形左边角度
        double theta2 = (2.0 * PI * (i + 1)) / slices;//当前小矩形右边角度//两个角度确定这个小矩形在圆柱上的位置

        Vector3 v[4];// 定义小矩形的 4 个 3D 顶点
        v[0] = Vector3(radius * cos(theta1), height / 2, radius * sin(theta1));
        v[1] = Vector3(radius * cos(theta2), height / 2, radius * sin(theta2));
        v[2] = Vector3(radius * cos(theta2), -height / 2, radius * sin(theta2));
        v[3] = Vector3(radius * cos(theta1), -height / 2, radius * sin(theta1));

        for (int k = 0; k < 4; k++) ROTATE_3D(v[k]);//对 4 个点做 3D 旋转（倾斜视角）

        Vector3 normal(cos(theta1 / 2 + theta2 / 2), 0, sin(theta1 / 2 + theta2 / 2));
        ROTATE_3D(normal);
        normal.Normalize();

        if (DotProduct(normal, ViewDir) < 0) continue;// 背面剔除：看不见的面不画

        Vector3 L = LightDir; // L = 平行光方向，H = 半角向量，用来算高光
        Vector3 H(L.x + ViewDir.x, L.y + ViewDir.y, L.z + ViewDir.z);
        H.Normalize();

        double L_dot_N = max(0.0, DotProduct(L, normal));//L_dot_N：漫反射
        double H_dot_N = max(0.0, DotProduct(H, normal));//H_dot_N：镜面高光

        int R = ka_R * Iar + kd_R * Ipr * L_dot_N + ks_R * Ipr * pow(H_dot_N, n);
        int G = ka_G * Iag + kd_G * Ipg * L_dot_N + ks_G * Ipg * pow(H_dot_N, n);
        int B = ka_B * Iab + kd_B * Ipb * L_dot_N + ks_B * Ipb * pow(H_dot_N, n);

        POINT rgnpoints[4];
        for (int k = 0; k < 4; k++) {
            rgnpoints[k].x = (int)(v[k].x + centerX);
            rgnpoints[k].y = (int)(-v[k].y + centerY);
        }
        //画这个小矩形
        CBrush brush(RGB(R, G, B)); CPen pen(PS_SOLID, 1, RGB(R, G, B));
        CBrush* pOldBrush = pDC->SelectObject(&brush); CPen* pOldPen = pDC->SelectObject(&pen);
        pDC->Polygon(rgnpoints, 4);
        pDC->SelectObject(pOldBrush); pDC->SelectObject(pOldPen);
    }
}

// CshiyanliuView 打印
void CshiyanliuView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
    AFXPrintPreview(this);
#endif
}
BOOL CshiyanliuView::OnPreparePrinting(CPrintInfo* pInfo)
{
  
    return DoPreparePrinting(pInfo);
}
void CshiyanliuView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{

}
void CshiyanliuView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
    
}
void CshiyanliuView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
    ClientToScreen(&point);
    OnContextMenu(this, point);
}
void CshiyanliuView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
    theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}
#ifdef _DEBUG
void CshiyanliuView::AssertValid() const
{
    CView::AssertValid();
}
void CshiyanliuView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}
CshiyanliuDoc* CshiyanliuView::GetDocument() const 
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CshiyanliuDoc)));
    return (CshiyanliuDoc*)m_pDocument;
}
#endif //_DEBUG

