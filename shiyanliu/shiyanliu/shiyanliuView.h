// shiyanliuView.h: CshiyanliuView 类的接口
//

#pragma once

// ==================== 图形学基础数学结构 ====================
#include <cmath>
#include <algorithm>

#define PI 3.14159265358979323846

// 定义三维向量结构
struct Vector3 {
	double x, y, z;
	Vector3() : x(0), y(0), z(0) {}
	Vector3(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}

	// 向量归一化
	void Normalize() {
		double len = ::sqrt(x * x + y * y + z * z);
		if (len > 0) { x /= len; y /= len; z /= len; }
	}
};

// 计算两个向量的点乘
inline double DotProduct(Vector3 a, Vector3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
// ============================================================

class CshiyanliuView : public CView
{
protected: // 仅从序列化创建
	CshiyanliuView() noexcept;
	DECLARE_DYNCREATE(CshiyanliuView)

	// 特性
public:
	CshiyanliuDoc* GetDocument() const;

	// 操作
public:

	// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

	// 实现
public:
	virtual ~CshiyanliuView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	// 生成的消息映射函数
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // shiyanliuView.cpp 中的调试版本
inline CshiyanliuDoc* CshiyanliuView::GetDocument() const
{
	return reinterpret_cast<CshiyanliuDoc*>(m_pDocument);
}
#endif