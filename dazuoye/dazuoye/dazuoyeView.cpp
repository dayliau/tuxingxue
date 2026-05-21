
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



// 粗布纹理生成函数
void CdazuoyeView::SetupTexture()
{
	static GLubyte clothTexture[TEX_SIZE][TEX_SIZE][3];//定义纹理数组256*256的3个颜色RGB纹理

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

            float A = (float)rand() / RAND_MAX;
            // 应用公式：f(u,v) = A(cos(pu) + cos(qv))
            float f = A * (std::cos(p * u) + std::cos(q * v));
            //f[-2,2]
            // 对 f 进行归一化和缩放，映射到 GLubyte (0-255)
            // 我们通过 (f + 2) / 4 将其映射到 [0, 1]
            float normalized_f = (f + 2.0f ) / (4.0f );

            // 确保不越界 [0, 1]
            if (normalized_f > 1.0f) normalized_f = 1.0f;
            if (normalized_f < 0.0f) normalized_f = 0.0f;

            // 缩放到 0-255 opengl得颜色范围
            GLubyte gray = (GLubyte)(normalized_f * 255.0f);

            // 赋予颜色 (以灰度模式为主，贴近图片)
            clothTexture[i][j][0] = gray;               // R
            clothTexture[i][j][1] = gray;               // G
            clothTexture[i][j][2] = gray;               // B
        }
    }

    // 3. 绑定到 OpenGL
    if (texCloth == 0) {
		glGenTextures(1, &texCloth);// 生成一个纹理对象ID
    }
	glBindTexture(GL_TEXTURE_2D, texCloth);// 绑定纹理对象，后续的纹理操作都作用于这个对象

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
void CdazuoyeView::DrawTorus(GLfloat r1, GLfloat r2,
    int numMajor, int numMinor)
{
    const float PI = 3.1415926f;

    // 纹理平铺次数 纹理重复8次
    float tileFactor = 8.0f;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texCloth);

    // 材质设为白色
    GLfloat mat_diffuse[] = { 1,1,1,1 };
    GLfloat mat_ambient[] = { 1,1,1,1 };

    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);

    // α方向圆环大圈
    for (int i = 0; i < numMajor; i++)
    {
        float alpha0 = 2.0f * PI * i / numMajor;
        float alpha1 = 2.0f * PI * (i + 1) / numMajor;

        // v = α / 2π
        float v0 = (float)i / numMajor;
        float v1 = (float)(i + 1) / numMajor;

        glBegin(GL_QUAD_STRIP);

        // β方向小圈
        for (int j = 0; j <= numMinor; j++)
        {
            float beta = 2.0f * PI * j / numMinor;

            // u = β / 2π
            float u = (float)j / numMinor;

            // ================= 第一个顶点 =================

            // 教材公式
            float x0 = (r1 + r2 * sinf(beta)) * sinf(alpha0);
            float y0 = r2 * cosf(beta);
            float z0 = (r1 + r2 * sinf(beta)) * cosf(alpha0);

            // 法向量
            float nx0 = sinf(beta) * sinf(alpha0);
            float ny0 = cosf(beta);
            float nz0 = sinf(beta) * cosf(alpha0);

			glNormal3f(nx0, ny0, nz0);//法向量用于光照计算

            // uv映射 指定纹理贴到模型哪个位置
            glTexCoord2f(
                u * tileFactor,
                v0 * tileFactor
            );

            glVertex3f(x0, y0, z0);

            // ================= 第二个顶点 =================

            float x1 = (r1 + r2 * sinf(beta)) * sinf(alpha1);
            float y1 = r2 * cosf(beta);
            float z1 = (r1 + r2 * sinf(beta)) * cosf(alpha1);

            float nx1 = sinf(beta) * sinf(alpha1);
            float ny1 = cosf(beta);
            float nz1 = sinf(beta) * cosf(alpha1);

            glNormal3f(nx1, ny1, nz1);

            glTexCoord2f(
                u * tileFactor,
                v1 * tileFactor
            );

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
	GLfloat light_position[] = { 10.0f, 10.0f, 10.0f, 0.0f }; // 光源位置，从右上前方照射，w=0表示定向光
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
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);//最终颜色=纹理颜色×光照结果



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
	gluLookAt(0.0, 5.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);//设置摄像机位置和朝向，从上方和前方观察圆环，y轴向上

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
    gluPerspective(45.0f, (GLfloat)cx / (GLfloat)cy, 1.0f, 100.0f);//近大远小，产生3D效果
    glMatrixMode(GL_MODELVIEW);
}

void CdazuoyeView::OnDestroy()
{
    CView::OnDestroy();
    wglMakeCurrent(NULL, NULL);
    if (m_hRC) wglDeleteContext(m_hRC);
}