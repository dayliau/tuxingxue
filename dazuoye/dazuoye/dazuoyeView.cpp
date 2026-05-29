
#include "pch.h" 
#include "dazuoye.h"
#include "dazuoyeDoc.h"
#include "dazuoyeView.h"
// 纹理大小
#define TEX_SIZE 256 
// 导入数学库
#include <cmath>
#include <cstdlib> // 用于 rand()
#include <ctime>   // 用于 srand(time(NULL))
// 自动连接 OpenGL 库
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
//消息映射
BEGIN_MESSAGE_MAP(CdazuoyeView, CView)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_DESTROY()
END_MESSAGE_MAP()

// CdazuoyeView 构造/析构
//构造函数
CdazuoyeView::CdazuoyeView() noexcept
{
    //初始化OpenGL环境和纹理ID
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
	static GLubyte clothTexture[TEX_SIZE][TEX_SIZE][3];//定义纹理数组256*256的3个颜色RGB纹理图片

	// 1. 设置纹理参数（越大越细腻，越小越粗糙）
    const float p = 100.0f;
    const float q = 100.0f;

    // 生成 [0,1] 的基础随机变量 A
    float A = 1.0f; // A 控制强度基数

	// 2. 使用公式生成粗布纹理 ，一个像素一个像素地计算
    for (int i = 0; i < TEX_SIZE; i++) {
        for (int j = 0; j < TEX_SIZE; j++) {
            // 将像素坐标归一化为 (u, v) ∈ [0, 1]
            float u = (float)i / (float)(TEX_SIZE - 1);
            float v = (float)j / (float)(TEX_SIZE - 1);
			// 生成一个新的随机变量 A，增加纹理的随机性
            float A = (float)rand() / RAND_MAX;
            // 应用公式：f(u,v) = A(cos(pu) + cos(qv))
            float f = A * (std::cos(p * u) + std::cos(q * v));
            //f[-2,2]，但颜色必要要0-256
            // 对 f 进行归一化和缩放，映射到 (0-255)
            // 我们通过 (f + 2) / 4 将其映射到 [0, 1]
            float normalized_f = (f + 2.0f ) / (4.0f );

            // 确保不越界 [0, 1]
            if (normalized_f > 1.0f) normalized_f = 1.0f;
            if (normalized_f < 0.0f) normalized_f = 0.0f;

            // 再缩放到 0-255得颜色范围
            GLubyte gray = (GLubyte)(normalized_f * 255.0f);

            // 赋予颜色 ，都是灰色
            clothTexture[i][j][0] = gray;               // R
            clothTexture[i][j][1] = gray;               // G
            clothTexture[i][j][2] = gray;               // B
        }
    }

    // 3. 创建OpenGL纹理ID并绑定
    if (texCloth == 0) {
		glGenTextures(1, &texCloth);// 生成一个（对应1）纹理对象ID，是opengl系统分配的
    }
	glBindTexture(GL_TEXTURE_2D, texCloth);// GL_TEXTURE_2D二维纹理 绑定纹理对象，后续的纹理操作都作用于这个对象texCloth

	// 4. 设置纹理参数
    // 设置纹理过滤（使用线性插值平滑处理），使用 GL_LINEAR 使纹理显示更加平滑。
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);// 当纹理被缩小时
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);// 当纹理被放大时
    // 设置纹理重复模式，让粗布纹理一圈圈重复包裹圆环
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//横向重复
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);//纵向重复

    // 将计算好的纹理数据上传到显卡
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_SIZE, TEX_SIZE, 0,
		GL_RGB, GL_UNSIGNED_BYTE, clothTexture);
    // 将纹理数据传递给OpenGL，参数分别是：
    //目标纹理类型为二维纹理、纹理层级0，内部格式用rgb储存、宽度256、高度256、边框固定为0、
    // 提供的数据格式是RGB、每个颜色分量是0~255无符号字节、粗布纹理数组指针
}

// 圆环绘制核心函数
void CdazuoyeView::DrawTorus(GLfloat r1, GLfloat r2,
    int numMajor, int numMinor)
    //OpenGL不会直接画：数学圆环用很多小四边形拼接所以要分段
	//圆环大圈切64份，管道小圈切32份
{
    const float PI = 3.1415926f;

    // 纹理平铺次数 纹理重复8次
    float tileFactor = 8.0f;

	glEnable(GL_TEXTURE_2D);//启用二维纹理映射
	glBindTexture(GL_TEXTURE_2D, texCloth);//绑定粗布纹理，使后续绘制的圆环使用这个纹理

    // 材质设为白色 这样不会改变纹理本身颜色，使粗布纹理能够直接作为漫反射系数参与光照计算。
    GLfloat mat_diffuse[] = { 1,1,1,1 };//diffuse为漫反射 参数rgb+透明度 纯白色+完全不透明
    GLfloat mat_ambient[] = { 1,1,1,1 };//ambient为环境光 参数rgb+透明度 纯白色+完全不透明

	//交给OpenGL设置材质属性，漫反射颜色为纯白色，这样纹理颜色不被改变，环境光也设置为白色稍微提亮暗部
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);//设置前面材质的漫反射颜色为纯白色，这样纹理颜色不被改变
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);//设置前面材质的环境光颜色为纯白色，这样稍微提亮暗部，使纹理细节更清晰

	// 沿圆环大圈绕行分段
    for (int i = 0; i < numMajor; i++)
    {
        
        //一整圈是2Π被切成numMajor（64）份，每份的角度就是2Π/numMajor，i表示当前是第几份
		float alpha0 = 2.0f * PI * i / numMajor;//这一段的起始角度
		float alpha1 = 2.0f * PI * (i + 1) / numMajor;//这一段的结束角度

		// v = α / 2π沿大圈的纹理坐标，v0是这一段的起始纹理坐标，v1是这一段的结束纹理坐标
        //圆环面是二次曲面，通过下述线性变换将纹理空间[0, 1]x[0, 1]与物体空间[0, 2Π]x[0, 2Π]等同起来。
        float v0 = (float)i / numMajor;
        float v1 = (float)(i + 1) / numMajor;

		glBegin(GL_QUAD_STRIP);
        //把用于绘制圆环的顶点定义在glBegin和glEnd之间，GL_QUAD_STRIP表示使用四边形条带的方式连接顶点，形成连续的四边形带来绘制圆环表面

        // 沿管道小圈绕行分段
        for (int j = 0; j <= numMinor; j++)
        {
            float beta = 2.0f * PI * j / numMinor;
			// 圆环小圈切成numMinor（32）份，每份的角度就是2Π/numMinor，j表示当前是第几份
            // u = β / 2π
            float u = (float)j / numMinor;

            // ================= 第一个顶点 =================

            // 教材公式
            float x0 = (r1 + r2 * sinf(beta)) * sinf(alpha0);
            float y0 = r2 * cosf(beta);
            float z0 = (r1 + r2 * sinf(beta)) * cosf(alpha0);

            // 求法向量
            float nx0 = sinf(beta) * sinf(alpha0);
            float ny0 = cosf(beta);
            float nz0 = sinf(beta) * cosf(alpha0);

			glNormal3f(nx0, ny0, nz0);//设置法向量用于光照计算

            // uv映射 指定纹理贴到模型哪个位置
            glTexCoord2f(
				u * tileFactor,//表示当前顶点在纹理中的横向位置，u是当前小圈的角度占比，tileFactor控制纹理重复的次数
                //在u方向上重复8次
				v0 * tileFactor//表示当前顶点在纹理中的纵向位置，v0是当前大圈的角度占比，tileFactor控制纹理重复的次数
				//在v方向上重复8次
			);//表示当前顶点使用纹理中的哪个位置

			glVertex3f(x0, y0, z0);
            //真正定义第一个顶点的位置，OpenGL会根据之前设置的纹理坐标和法向量来计算这个顶点的最终颜色和光照效果

            // ================= 第二个顶点 =================
            //同理

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
	//告诉Windows：要什么样的OpenGL显示模式，
    // PFD_DRAW_TO_WINDOW画到窗口，PFD_SUPPORT_OPENGL支持OpenGL，PFD_DOUBLEBUFFER双缓冲（后台缓冲区和前台缓冲区），
    // PFD_TYPE_RGBA使用RGBA颜色模式，24位颜色深度（8r8g8b），32位深度缓冲
    PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA, 24, 0,0,0,0,0,0, 0, 0, 0, 0,0,0,0, 32, 0, 0, PFD_MAIN_PLANE, 0, 0,0,0 };
    //让系统选择最合适的像素格式
    int pixelFormat = ChoosePixelFormat(dc.GetSafeHdc(), &pfd);
    //把这种格式应用到窗口
    SetPixelFormat(dc.GetSafeHdc(), pixelFormat, &pfd);
    //真正创建OpenGL工作环境
    m_hRC = wglCreateContext(dc.GetSafeHdc());
	//激活这个环境，让它准备好接受OpenGL命令
	//把这个环境和窗口的DC绑定起来，这样OpenGL的命令就会作用在这个窗口上
    wglMakeCurrent(dc.GetSafeHdc(), m_hRC);
    //开启“谁近谁挡住后面”
    glEnable(GL_DEPTH_TEST);
	//生成粗布纹理
    SetupTexture();


    // 1. 设置 0号光源 的属性 (一盏白色的定向光)
	GLfloat light_position[] = { 10.0f, 10.0f, 10.0f, 0.0f }; // 光源位置，从右上前方照射，w=0表示定向光类似太阳光
    GLfloat light_diffuse[] = { 1.0f,  1.0f,  1.0f,  1.0f }; // 光源自身的漫反射强度 (Ip)
    GLfloat light_ambient[] = { 0.3f,  0.3f,  0.3f,  1.0f }; // 环境光，稍微提亮暗部
	//把这些属性传递给OpenGL系统，告诉它0号光源是什么样子的
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

    // 2. 开启光照
    glEnable(GL_LIGHTING);//整体光照
	glEnable(GL_LIGHT0);//0号光源

    // 3. 关键要求实现：设置纹理映射模式为 GL_MODULATE
    // 这行代码指示 OpenGL 将纹理像素值乘以光照计算结果，使纹理充当漫反射系数 (Kd)
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);//最终颜色=纹理颜色×光照结果
	//GL_TEXTURE_ENV纹理环境参数，GL_TEXTURE_ENV_MODE纹理环境模式参数，GL_MODULATE表示告诉opengl将纹理颜色与光照计算结果相乘，这样纹理就会根据光照条件变亮或变暗，产生更真实的效果。


    return 0;
}
//实现清屏，设置摄像机位置和朝向，旋转圆环便于观察，并调用DrawTorus函数绘制圆环
void CdazuoyeView::OnDraw(CDC* pDC)
{
    CdazuoyeDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);

	//激活openGL环境，准备接受OpenGL命令
    wglMakeCurrent(pDC->GetSafeHdc(), m_hRC);

	//设置背景颜色并清除颜色缓冲和深度缓冲，准备开始新的一帧绘制
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //恢复默认坐标系
    glLoadIdentity();
	//摄像机从前上方（0，5，10）观察圆环，目标点在原点（0，0，0），y轴向上（0，1，0）
	gluLookAt(0.0, 5.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);//设置摄像机位置和朝向，从上方和前方观察圆环，y轴向上

    // 旋转圆环便于观察
    static float angle = -60.0f;
	//绕x轴旋转-60度，让我们从上方斜视圆环，这样可以更清楚地看到纹理效果
    // （1.0f, 0.5f, 0.0f是旋转轴的方向，主要绕x轴旋转，稍微绕y轴旋转）
    glRotatef(angle, 1.0f, 0.5f, 0.0f);
    // 取消下面这行的注释可以让圆环动起来（配合定时器更好）
     angle += 0.5f; 
	//绘制圆环，参数分别是大半径3.0f、小半径1.0f、大圈分段数64和小圈分段数32
    DrawTorus(3.0f, 1.0f, 64, 32);
	//交换前后缓冲区（后台偷偷画，前台显示给用户），显示绘制结果
    SwapBuffers(pDC->GetSafeHdc());
}

void CdazuoyeView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);
    if (cy == 0) cy = 1;
    //激活环境
    wglMakeCurrent(GetDC()->GetSafeHdc(), m_hRC);
	//设置视口为整个窗口（左下角(0,0)，宽度cx，高度cy）
    glViewport(0, 0, cx, cy);
	//设置投影矩阵，3D到2D
    glMatrixMode(GL_PROJECTION);
	//恢复默认坐标系，清空之前的投影变换
    glLoadIdentity();
	//设置透视投影近大远小，
    // 参数分别是：摄像机视野角45度、窗口宽高比cx/cy、近裁剪面1.0f太近的不显示和远裁剪面100.0f太远的不显示
    gluPerspective(45.0f, (GLfloat)cx / (GLfloat)cy, 1.0f, 100.0f);//近大远小，产生3D效果
	//刚才在设置投影，现在切换回模型视图矩阵，准备进行模型变换和绘制
    glMatrixMode(GL_MODELVIEW);
}

void CdazuoyeView::OnDestroy()
{
    CView::OnDestroy();
	//取消OpenGL环境的激活，准备销毁资源
    wglMakeCurrent(NULL, NULL);
	//删除OpenGL渲染上下文，释放资源
    if (m_hRC) wglDeleteContext(m_hRC);
}