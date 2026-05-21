
#include "pch.h" 
#include "dazuoye.h"
#include "dazuoyeDoc.h"
#include "dazuoyeView.h"
// 纹理生成核心函数
#define TEX_SIZE 256
// 导入数学库
#include <cmath>
#include <cstdlib> // 用于 rand()
#include <ctime>   // 用于 srand(time(NULL))
// 告诉链接器自动引入 OpenGL 的静态库
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CdazuoyeView

IMPLEMENT_DYNCREATE(CdazuoyeView, CView)

BEGIN_MESSAGE_MAP(CdazuoyeView, CView)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_DESTROY()
END_MESSAGE_MAP()

// CdazuoyeView 构造/析构

CdazuoyeView::CdazuoyeView() noexcept
{
    m_hRC = NULL;
    texCloth = 0;
}

BOOL CdazuoyeView::PreCreateWindow(CREATESTRUCT& cs)
{
    // 确保窗口样式支持 OpenGL
    cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    return CView::PreCreateWindow(cs);
}



// 将原始 SetupTexture 函数替换为以下内容：
void CdazuoyeView::SetupTexture()
{
    static GLubyte clothTexture[TEX_SIZE][TEX_SIZE][3];

    // 初始化随机种子（建议在 OnCreate 中执行一次即可，这里为演示放在这）
    // static bool initialized = false;
    // if (!initialized) {
    //     srand((unsigned int)time(NULL));
    //     initialized = true;
    // }

    // 1. 设置纹理参数（您可以根据图片示例进行调整）
    // 对应示例 (a): p=100, q=100
    // 对应示例 (b): p=50,  q=100
    // 对应示例 (c): p=100, q=50
    const float p = 100.0f;
    const float q = 100.0f;

    // 生成 [0,1] 的基础随机变量 A
    float A = 1.0f; // A 控制强度基数

    // 2. 使用公式生成粗布纹理 (Procedural Cloth Texture)
    for (int i = 0; i < TEX_SIZE; i++) {
        for (int j = 0; j < TEX_SIZE; j++) {
            // 将像素坐标归一化为 (u, v) ∈ [0, 1]
            float u = (float)i / (float)(TEX_SIZE - 1);
            float v = (float)j / (float)(TEX_SIZE - 1);

            // 应用公式：f(u,v) = A(cos(pu) + cos(qv))
            float f = A * (std::cos(p * u) + std::cos(q * v));

            // 对 f 进行归一化和缩放，映射到 GLubyte (0-255)
            // f 的范围理论上从 -2A 到 2A (-2.0 到 2.0)
            // 我们通过 (f + 2A) / 4A 将其映射到 [0, 1]
            float normalized_f = (f + 2.0f * A) / (4.0f * A);

            // 确保不越界 [0, 1]
            if (normalized_f > 1.0f) normalized_f = 1.0f;
            if (normalized_f < 0.0f) normalized_f = 0.0f;

            // 缩放到 0-255
            GLubyte gray = (GLubyte)(normalized_f * 255.0f);

            // 赋予颜色 (以灰度模式为主，贴近图片)
            clothTexture[i][j][0] = gray;               // R
            clothTexture[i][j][1] = gray;               // G
            clothTexture[i][j][2] = gray;               // B
        }
    }

    // 3. 绑定到 OpenGL
    if (texCloth == 0) {
        glGenTextures(1, &texCloth);
    }
    glBindTexture(GL_TEXTURE_2D, texCloth);

    // 设置纹理过滤（线性过滤）
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 设置纹理重复模式（公式是周期性的，这里仍可设为重复）
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // 将计算好的纹理数据上传到显卡
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_SIZE, TEX_SIZE, 0,
        GL_RGB, GL_UNSIGNED_BYTE, clothTexture);
}

// 圆环绘制核心函数
void CdazuoyeView::DrawTorus(GLfloat R, GLfloat r, int numMajor, int numMinor)
{
    float PI = 3.1415926f;
    float tileFactor = 12.0f; // 纹理重复平铺次数

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texCloth);

    // 设置物体的基础材质为纯白
    // 配合 OnCreate 中的 GL_MODULATE，纹理颜色将直接作为漫反射系数计算光照
    GLfloat mat_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat mat_ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);

    for (int i = 0; i < numMajor; i++) {
        float theta0 = (float)i * 2.0f * PI / numMajor;
        float theta1 = (float)(i + 1) * 2.0f * PI / numMajor;

        float u0 = (float)i / numMajor;
        float u1 = (float)(i + 1) / numMajor;

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= numMinor; j++) {
            float phi = (float)j * 2.0f * PI / numMinor;
            float v = (float)j / numMinor;

            // ================== 第一个顶点 ==================
            // 1. 计算并绑定法线 (漫反射光照计算必须要有法线)
            float nx0 = cosf(phi) * cosf(theta0);
            float ny0 = cosf(phi) * sinf(theta0);
            float nz0 = sinf(phi);
            glNormal3f(nx0, ny0, nz0);

            // 2. 绑定纹理坐标并绘制顶点
            float x0 = (R + r * cosf(phi)) * cosf(theta0);
            float y0 = (R + r * cosf(phi)) * sinf(theta0);
            float z0 = r * sinf(phi);
            glTexCoord2f(u0 * tileFactor, v * tileFactor);
            glVertex3f(x0, y0, z0);

            // ================== 第二个顶点 ==================
            // 1. 计算并绑定法线
            float nx1 = cosf(phi) * cosf(theta1);
            float ny1 = cosf(phi) * sinf(theta1);
            float nz1 = sinf(phi);
            glNormal3f(nx1, ny1, nz1);

            // 2. 绑定纹理坐标并绘制顶点
            float x1 = (R + r * cosf(phi)) * cosf(theta1);
            float y1 = (R + r * cosf(phi)) * sinf(theta1);
            float z1 = r * sinf(phi);
            glTexCoord2f(u1 * tileFactor, v * tileFactor);
            glVertex3f(x1, y1, z1);
        }
        glEnd();
    }
    glDisable(GL_TEXTURE_2D);
}

// MFC 消息映射实现

int CdazuoyeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1) return -1;

    CClientDC dc(this);
    PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA, 24, 0,0,0,0,0,0, 0, 0, 0, 0,0,0,0, 32, 0, 0, PFD_MAIN_PLANE, 0, 0,0,0 };
    int pixelFormat = ChoosePixelFormat(dc.GetSafeHdc(), &pfd);
    SetPixelFormat(dc.GetSafeHdc(), pixelFormat, &pfd);

    m_hRC = wglCreateContext(dc.GetSafeHdc());
    wglMakeCurrent(dc.GetSafeHdc(), m_hRC);

    glEnable(GL_DEPTH_TEST);
    SetupTexture();


    // 1. 设置 0号光源 的属性 (一盏白色的定向光)
    GLfloat light_position[] = { 10.0f, 10.0f, 10.0f, 0.0f }; // 光源位置
    GLfloat light_diffuse[] = { 1.0f,  1.0f,  1.0f,  1.0f }; // 光源自身的漫反射强度 (Ip)
    GLfloat light_ambient[] = { 0.3f,  0.3f,  0.3f,  1.0f }; // 环境光，稍微提亮暗部

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

    // 2. 开启光照
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // 3. 关键要求实现：设置纹理映射模式为 GL_MODULATE
    // 这行代码指示 OpenGL 将纹理像素值乘以光照计算结果，使纹理充当漫反射系数 (Kd)
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);



    return 0;
}

void CdazuoyeView::OnDraw(CDC* pDC)
{
    CdazuoyeDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);

    wglMakeCurrent(pDC->GetSafeHdc(), m_hRC);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    gluLookAt(0.0, 5.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    // 旋转圆环便于观察
    static float angle = -60.0f;
    glRotatef(angle, 1.0f, 0.5f, 0.0f);
    // 取消下面这行的注释可以让圆环动起来（配合定时器更好）
    // angle += 0.5f; 

    DrawTorus(3.0f, 1.0f, 64, 32);

    SwapBuffers(pDC->GetSafeHdc());
}

void CdazuoyeView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);
    if (cy == 0) cy = 1;

    wglMakeCurrent(GetDC()->GetSafeHdc(), m_hRC);
    glViewport(0, 0, cx, cy);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)cx / (GLfloat)cy, 1.0f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

void CdazuoyeView::OnDestroy()
{
    CView::OnDestroy();
    wglMakeCurrent(NULL, NULL);
    if (m_hRC) wglDeleteContext(m_hRC);
}