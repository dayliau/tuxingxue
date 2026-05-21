#pragma once

// 必须放在 OpenGL 头文件之前！
#include <windows.h> 
#include <gl/GL.h>
#include <gl/GLU.h>

class CdazuoyeDoc; // 前置声明

class CdazuoyeView : public CView
{
protected: // 仅从序列化创建
    CdazuoyeView() noexcept;
    DECLARE_DYNCREATE(CdazuoyeView)

public:
    CdazuoyeDoc* GetDocument() const;
    virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

private:
    HGLRC m_hRC;        // OpenGL 渲染上下文
    GLuint texCloth;    // 纹理对象ID

    void SetupTexture();
    void DrawTorus(GLfloat R, GLfloat r, int numMajor, int numMinor);

    // 生成的消息映射函数
protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnDestroy();
    DECLARE_MESSAGE_MAP()
};

inline CdazuoyeDoc* CdazuoyeView::GetDocument() const
{
    return reinterpret_cast<CdazuoyeDoc*>(m_pDocument);
}