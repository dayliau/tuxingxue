#pragma once

// 三个opengl要依赖的头文件
#include <windows.h> 
#include <gl/GL.h>// 包含 OpenGL 核心头文件
#include <gl/GLU.h>// 包含 OpenGL 实用工具库头文件

class CdazuoyeDoc; // 前置声明

class CdazuoyeView : public CView
{
protected: // 仅从序列化创建
    CdazuoyeView() noexcept;
    DECLARE_DYNCREATE(CdazuoyeView)
//重写
public:
    CdazuoyeDoc* GetDocument() const;

    virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);//在窗口真正创建前，修改窗口属性，使更适合OpenGL渲染
   

private:
    HGLRC m_hRC;        // 创建一个OpenGL 环境变量m_hRC
    GLuint texCloth;    // 纹理对象ID，粗布纹理的编号，opengl把我的纹理创立了一个opengl对象并分配一个id，在想使用这个纹理时直接调用这个id就行

	void SetupTexture();//生成粗布纹理的函数
	void DrawTorus(GLfloat R, GLfloat r, int numMajor, int numMinor);//绘制圆环的函数，参数分别是大半径、小半径、大圈分段数和小圈分段数

    // 生成的消息映射函数
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);//窗口创建时的初始化函数，设置像素格式、创建OpenGL上下文、启用深度测试、设置光照和纹理环境等
	afx_msg void OnSize(UINT nType, int cx, int cy);//窗口大小改变时的处理函数，调整OpenGL视口和投影矩阵以适应新的窗口尺寸
	afx_msg void OnDestroy();//窗口销毁时的清理函数，删除OpenGL渲染上下文等资源
    DECLARE_MESSAGE_MAP()
};

inline CdazuoyeDoc* CdazuoyeView::GetDocument() const
{
    return reinterpret_cast<CdazuoyeDoc*>(m_pDocument);
}