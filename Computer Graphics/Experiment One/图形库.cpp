// Win32Project1.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "Win32Project1.h"
#include<math.h>
#include <list>
#define MAX_LOADSTRING 100

void DDA(HDC hdc, int x0, int y0, int xn, int yn)
{
	/*确定增量dx,dy*/
	int max;
	double dx, dy;
	int d_x = abs(xn - x0);
	int d_y = abs(yn - y0);
	if (d_x >= d_y)
		max = d_x;
	else
		max = d_y;
	dx = (xn - x0)*1.0 / max;
	dy = (yn - y0)*1.0 / max;
	/*绘点*/
	double xi = x0, yi = y0;
	int pix_x = floor(xi), pix_y = floor(yi);
	SetPixel(hdc, pix_x, pix_y, RGB(23, 24, 23));
	for (int i = 0; i != max; ++i) {
		xi += dx;
		yi += dy;
		pix_x = floor(xi);
		pix_y = floor(yi);
		SetPixel(hdc, pix_x, pix_y, RGB(23, 24, 23));
	}
}

void Bresenham(HDC hdc, int x1, int y1, int x2, int y2)
{
	int x, y, dx, dy, e;
	dx = abs(x2 - x1);
	dy = abs(y2 - y1);
	e = 2 * dy - dx;
	x = x1;
	y = y1;
	for (int i = 0; i < dx; i++) {
		SetPixel(hdc, x, y, RGB(0, 0, 0));
		if (e >= 0) {
			y++;
			e = e - 2 * dx;
		}
		x++;
		e = e + 2 * dy;
	}

}

void RoundArc(HDC hdc, int x, int y, int r)
{
	int X, Y, d;
	X = x;
	Y = y - r;
	d = (y - Y)*(y - Y) + (Y + 1 - y)*(Y + 1 - y) - 2 * r * r;

	while ((X - x) <= (y - Y))
	{
		if (d >= 0)
		{
			Y++;
			d = d + 4 * (X + Y) - 4 * (x + y) + 10;
		}
		else
		{
			d = d + 4 * X - 4 * x + 6;
		}

		SetPixel(hdc, X, Y, RGB(0, 0, 0));
		SetPixel(hdc, (2 * x - X), Y, RGB(0, 0, 0));
		SetPixel(hdc, X, (2 * y - Y), RGB(0, 0, 0));
		SetPixel(hdc, (2 * x - X), (2 * y - Y), RGB(0, 0, 0));
		SetPixel(hdc, (x + (y - Y)), (y - (X - x)), RGB(0, 0, 0));
		SetPixel(hdc, (x + (y - Y)), (y + (X - x)), RGB(0, 0, 0));
		SetPixel(hdc, (x - (y - Y)), (y - (X - x)), RGB(0, 0, 0));
		SetPixel(hdc, (x - (y - Y)), (y + (X - x)), RGB(0, 0, 0));

		X++;
	}
}

void EllipseArc(HDC hdc, int x, int y, int a, int b)
{
	float t = 0, dt = 0.001;
	int x1 = x + a, y1 = y, x2, y2;
	while (t < 2 * 3.14159) {
		t += dt;
		x2 = x + (int)(a * cos(t));
		y2 = y + (int)(b * sin(t));
		Bresenham(hdc, x1, y1, x2, y2);
		x1 = x2;
		y1 = y2;
	}
}

void bubbleSort(double array[][2], int size)
{
	bool changed = true;
	int n = 0;
	do {
		changed = false;
		for (int i = 0; i < size - 1 - n; i++) {
			if (array[i][0] > array[i + 1][0]) {
				int temp;
				temp = array[i][0];
				array[i][0] = array[i + 1][0];
				array[i + 1][0] = temp;
				temp = array[i][1];
				array[i][1] = array[i + 1][1];
				array[i + 1][1] = temp;
				changed = true;
			}
		}
		n++;
	} while (changed);
}

		void shadowFill(HDC hdc, int P[][2], int mn, int m, double h, double a)
		{
			const double k = 1.0;
			const double db = 3.0;
			double B[8][2];
			int i, j;
			for (i = 0; (i + 1) <= (m - 1); ++i) {
				B[i][0] = (double)(P[i][1] - k*P[i][0]);
				B[i][1] = (double)(P[i + 1][1] - k*P[i + 1][0]);
				if (B[i][0] > B[i][1]) {
					double temp = B[i][0];
					B[i][0] = B[i][1];
					B[i][1] = temp;
				}
			}
			B[m - 1][0] = (double)(P[m - 1][1] - k*P[m - 1][0]);
			B[m - 1][1] = (double)(P[0][1] - k*P[0][0]);
			if (B[m - 1][0] > B[m - 1][1]) {
				double temp = B[m - 1][0];
				B[m - 1][0] = B[m - 1][1];
				B[m - 1][1] = temp;
			}
			for (i = m; (i + 1) <= (mn - 1); ++i) {
				B[i][0] = (double)(P[i][1] - k*P[i][0]);
				B[i][1] = (double)(P[i + 1][1] - k*P[i + 1][0]);
				if (B[i][0] > B[i][1]) {
					double temp = B[i][0];
					B[i][0] = B[i][1];
					B[i][1] = temp;
				}
			}
			B[mn - 1][0] = (double)(P[mn - 1][1] - k*P[mn - 1][0]);
			B[mn - 1][1] = (double)(P[m][1] - k*P[m][0]);
			if (B[mn - 1][0] > B[mn - 1][1]) {
				double temp = B[mn - 1][0];
				B[mn - 1][0] = B[mn - 1][1];
				B[mn - 1][1] = temp;
			}

			double Bmin = B[0][0], Bmax = B[0][1];
			for (i = 1; i < mn; i++) {
				if (B[i][0] < Bmin)
					Bmin = B[i][0];
				if (B[i][1] > Bmax)
					Bmax = B[i][1];
			}

			double b = Bmin + db;
			double D[8][2];
			double xp, yp, xq, yq, x, y;
			while (b < Bmax) {
				for (i = 0; i <= mn - 1; ++i)
					for (j = 0; j <= 1; ++j)
						D[i][j] = 10000.0;
				for (i = 0; (i + 1) <= (m - 1); ++i) {
					if ((B[i][0] <= b) && (b < B[i][1])) {
						xp = P[i][0];
						yp = P[i][1];
						xq = P[i + 1][0];
						yq = P[i + 1][1];
						x = (xp*yq - yp*xq + b*(xq - xp)) / (yq - yp - k*(xq - xp));
						y = k*x + b;
						D[i][0] = x;
						D[i][1] = y;
					}
				}
				if ((B[m - 1][0] <= b) && (b < B[m - 1][1])) {
					xp = P[m - 1][0];
					yp = P[m - 1][1];
					xq = P[0][0];
					yq = P[0][1];
					x = (xp*yq - yp*xq + b*(xq - xp)) / (yq - yp - k*(xq - xp));
					y = k*x + b;
					D[i][0] = x;
					D[i][1] = y;
				}
				for (i = m; (i + 1) <= (mn - 1); ++i) {
					if ((B[i][0] <= b) && (b < B[i][1])) {
						xp = P[i][0];
						yp = P[i][1];
						xq = P[i + 1][0];
						yq = P[i + 1][1];
						x = (xp*yq - yp*xq + b*(xq - xp)) / (yq - yp - k*(xq - xp));
						y = k*x + b;
						D[i][0] = x;
						D[i][1] = y;
					}
				}
				if ((B[mn - 1][0] <= b) && (b < B[mn - 1][1])) {
					xp = P[mn - 1][0];
					yp = P[mn - 1][1];
					xq = P[m][0];
					yq = P[m][1];
					x = (xp*yq - yp*xq + b*(xq - xp)) / (yq - yp - k*(xq - xp));
					y = k*x + b;
					D[i][0] = x;
					D[i][1] = y;
				}

				bubbleSort(D, 8);
				for (int k = 0; (D[2 * k][0] != 10000) && (D[2 * k + 1][0] != 10000); k++) {
					DDA(hdc, D[2 * k][0], D[2 * k][1], D[2 * k + 1][0], D[2 * k + 1][1]);
				}
				b = b + db;
			}
		}
		void DrawPolygon(HDC hdc, int p[][2], int mn, int m)
		{
			int i;
			for (i = 0; (i + 1) <= (m - 1); ++i)
				DDA(hdc, p[i][0], p[i][1], p[i + 1][0], p[i + 1][1]);
			DDA(hdc, p[0][0], p[0][1], p[m - 1][0], p[m - 1][1]);
			for (i = m; (i + 1) <= (mn - 1); ++i)
				DDA(hdc, p[i][0], p[i][1], p[i + 1][0], p[i + 1][1]);
			DDA(hdc, p[m][0], p[m][1], p[mn - 1][0], p[mn - 1][1]);
		}

void ColorFill(HDC hdc, int x, int y, COLORREF old_color, COLORREF new_color)
{
	struct Pixel {
		int x;
		int y;
	};
	static std::list<Pixel> points;
	points.push_back(Pixel{ x,y });
	while (points.empty() != true) {
		Pixel tmp = points.front();
		points.pop_front();
		x = tmp.x;
		y = tmp.y;
		if (GetPixel(hdc, x, y) == old_color) {
			SetPixel(hdc, x, y, new_color);
			points.push_back(Pixel{ x + 1, y });
			points.push_back(Pixel{ x, y + 1 });
			points.push_back(Pixel{ x, y - 1 });
			points.push_back(Pixel{ x - 1, y });
		}
	}
	//if (GetPixel(hdc, x, y) == old_color) {
	//	SetPixel(hdc, x, y, new_color);
	//	ColorFill(hdc, x + 1, y, old_color, new_color);
	//	ColorFill(hdc, x, y + 1, old_color, new_color);
	//	ColorFill(hdc, x, y - 1, old_color, new_color);
	//	ColorFill(hdc, x - 1, y, old_color, new_color);
	//}
}

void WriteMyName(HDC hdc) {
	//写出“张有杰”
	//“张”字左半部分
	DDA(hdc, 100, 100, 200, 100);
	DDA(hdc, 200, 100, 200, 200);
	DDA(hdc, 200, 200, 100, 200);
	DDA(hdc, 100, 200, 100, 250);
	DDA(hdc, 100, 250, 200, 250);
	DDA(hdc, 200, 250, 200, 350);
	DDA(hdc, 125, 300, 200, 350);

	//“张”字右半部分
	DDA(hdc, 220, 100, 220, 350);
	DDA(hdc, 220, 225, 320, 125);
	DDA(hdc, 170, 225, 335, 225);
	DDA(hdc, 220, 350, 280, 320);
	DDA(hdc, 220, 225, 320, 330);

	//“有”
	DDA(hdc, 340, 140, 520, 140);
	DDA(hdc, 430, 100, 340, 240);
	DDA(hdc, 385, 175, 385, 350);
	DDA(hdc, 385, 175, 480, 175);
	DDA(hdc, 480, 175, 480, 350);
	DDA(hdc, 385, 235, 480, 235);
	DDA(hdc, 385, 270, 480, 270);
	DDA(hdc, 480, 350, 430, 330);

	//“杰字”
	//“木”
	DDA(hdc, 560, 150, 750, 150);
	DDA(hdc, 655, 100, 655, 300);
	DDA(hdc, 655, 150, 580, 280);
	DDA(hdc, 655, 150, 730, 280);
	//“四点”
	DDA(hdc, 560, 330, 580, 320);
	DDA(hdc, 600, 330, 620, 320);
	DDA(hdc, 690, 320, 710, 330);
	DDA(hdc, 730, 320, 750, 330);

}

// 全局变量: 
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 在此放置代码。

	// 初始化全局字符串
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_WIN32PROJECT1, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化: 
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32PROJECT1));

	MSG msg;

	// 主消息循环: 
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32PROJECT1));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WIN32PROJECT1);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 将实例句柄存储在全局变量中

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 分析菜单选择: 
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		//DDA(hdc, 200, 100, 100, 200);
		//Bresenham(hdc, 200,100, 100, 200);

		//		
		//		DDA(hdc,20,400,400,20);
		//		RoundArc(hdc,100,100,100);
		WriteMyName(hdc);
		//		ColorFill(hdc, 100, 100, RGB(255, 255, 255), RGB(0, 0, 255));
		//EllipseArc(hdc, 200, 200, 150, 100);
		//int p[6][2] = { 100,100,200,100,300,200,200,300,100,300,0,200 };
		//DrawPolygon(hdc, p, 6, 5);
		//shadowFill(hdc, p, 6, 5, 45, 20);
		//EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
