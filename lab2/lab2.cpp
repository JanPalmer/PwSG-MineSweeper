// lab2.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "lab2.h"
#include <time.h>
#include <list>
#include <algorithm>
#include <random>
#include <map>
#include <queue>
#include <utility>

#define MAX_LOADSTRING 100
#define TITLETIMER 1

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
const int minWidth = 10, maxWidth = 30;
const int minHeigth = 10, maxHeight = 24;
const int BlockLen = 25, GapLen = 1;
const int MineRadius = 9;
const int TopPanel = 30;
const int startWidth = 10, startHeight = 10, startMineCount = 10;
static int screenWidth, screenHeight;
static int FieldWidth, FieldHeight, MineCount;
static int tmpFieldWidth, tmpFieldHeight, tmpMineCount;
static GameState gameState;
static double timeCount;
static int flagsRemaining = startMineCount;
static int flagsCorrect = startMineCount;
static int fieldsRemaining = startWidth * startHeight;

static Field fields[maxHeight][maxWidth];
static HWND mainWindow;
static RECT baseRect;
static HFONT font, fontArial;
static COLORREF colors[9];
static HBRUSH brushes[9];
static bool BFSwork = false;

static std::map<HWND, POINT> mapHWND;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
ATOM                FieldClass(HINSTANCE);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    WndProcField(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                gameReload(HWND, int, int, int);
void                adjustMainWindowRect(int minefieldWidth, int minefieldHeight);
void                spawnMineField(HWND hWnd, int minefieldWidth, int minefieldHeight, int mineCount);
void                FieldBFS(HWND hWnd, int x, int y);
void                timerStart();
void                timerRestart();
void                WinLogic(HWND);
void                FailureLogic(HWND);
void                DebugRevealField(int, int, bool);
void                DebugRevealFields(bool);
void                PaintTile(HWND);
void                PaintFlag(HWND, bool);
void                PaintTopPanel();
void                RepaintWindow();
void                CustomSize();
BOOL CALLBACK       CustomSizeProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LAB2, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    FieldClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LAB2));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LAB2));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LAB2);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

ATOM FieldClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProcField;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LAB2));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_GRAYTEXT);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_LAB2);
    wcex.lpszClassName = L"Field";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    srand(time(NULL));
    FieldWidth = startWidth;
    FieldHeight = startHeight;
    MineCount = startMineCount;
    fontArial = CreateFont(25, 0, 0, 0, FW_BOLD, false, FALSE, 0, EASTEUROPE_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS,
        _T("Arial"));

    colors[0] = RGB(0, 0, 0);
    colors[1] = RGB(0, 0, 200);
    colors[2] = RGB(0, 175, 0);
    colors[3] = RGB(200, 0, 0);
    colors[4] = RGB(128, 128, 0);
    colors[5] = RGB(0, 128, 128);
    colors[6] = RGB(128, 0, 128);
    colors[7] = RGB(128, 128, 128);
    colors[8] = RGB(200, 128, 0);

    for (int i = 0; i < 9; i++)
        brushes[i] = CreateSolidBrush(colors[i]);
    
    screenWidth = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);

    adjustMainWindowRect(FieldWidth, FieldHeight);

    // https://social.msdn.microsoft.com/Forums/vstudio/en-US/916baf08-b1aa-4e6f-a26b-4e25ab8714a7/how-to-disable-a-window-from-resizing?forum=vcgeneral
    mainWindow = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
        baseRect.left, baseRect.top, baseRect.right - baseRect.left, baseRect.bottom - baseRect.top, nullptr, nullptr, hInst, nullptr);

    if (!mainWindow)
    {
        return FALSE;
    }

    ShowWindow(mainWindow, nCmdShow);
    UpdateWindow(mainWindow);

    spawnMineField(mainWindow, FieldWidth, FieldHeight, MineCount);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    const int bufSize = 100;
    TCHAR buf[bufSize] = { NULL };

    switch (message)
    {
    case WM_CREATE:
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case ID_GAME_NEW:
                {
                    // NEW option in MENU
                    gameReload(hWnd, FieldWidth, FieldHeight, MineCount);
                }
                break;
            case ID_GAME_CUSTOMSIZE:
			    {
				    CustomSize();
			    }
			    break;
            case ID_HELP_DEBUG:
                {
                    // DEBUG MODE
                    if (GetMenuState(GetMenu(hWnd), ID_HELP_DEBUG, MF_BYCOMMAND) == MF_UNCHECKED) {
                        CheckMenuItem(GetMenu(hWnd), ID_HELP_DEBUG, MF_CHECKED);
                        DebugRevealFields(true);
                    }
                    else
                    {
                        CheckMenuItem(GetMenu(hWnd), ID_HELP_DEBUG, MF_UNCHECKED);
                        DebugRevealFields(false);
                    }
                }
            break;
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
    case WM_TIMER:
    {
        if (wParam == TITLETIMER)
        {
            timeCount += 0.1f;
            PaintTopPanel();
        }
    }
    break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(mainWindow, &ps);
            RepaintWindow();
            EndPaint(mainWindow, &ps);
        }
        break;
    case WM_CLOSE:
        {
            DeleteObject(font);
            DeleteObject(fontArial);
            for (int i = 0; i < 9; i++)
                DeleteObject(brushes[i]);

            DestroyWindow(hWnd);
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

// Processes messages for a single field in the minefield
LRESULT CALLBACK WndProcField(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ 
    const int bufSize = 100;
    TCHAR buf[bufSize];

    switch (message)
    {
    case WM_RBUTTONUP: // flags
    {
        timerStart();

        POINT p = mapHWND[hWnd];
        if (gameState == GameState::InProgress && flagsRemaining > 0 && fields[p.y][p.x].IsFlagged == false) {
            PaintFlag(hWnd, true);
            fields[p.y][p.x].IsFlagged = true;
            flagsRemaining--;
            UpdateWindow(mainWindow);

            if (fields[p.y][p.x].type == FieldState::BOMB)
                flagsCorrect--;

            if (flagsCorrect == 0) {
            SendMessage(mainWindow, WM_TIMER, TITLETIMER, NULL);
                WinLogic(mainWindow);
            }
        }
        else if (fields[p.y][p.x].IsFlagged == true) {
			PaintFlag(hWnd, false);
			fields[p.y][p.x].IsFlagged = false;
			flagsRemaining++;

			if (fields[p.y][p.x].type == FieldState::BOMB)
				flagsCorrect++;

			fieldsRemaining++;
        }
    }
        break;
    case WM_LBUTTONUP: // revealing tiles
    {
        timerStart();

        POINT t = mapHWND[hWnd];
        if (fields[t.y][t.x].IsClicked == false && fields[t.y][t.x].IsFlagged == false)
        {          
            fields[t.y][t.x].IsClicked = true;
            PaintTile(hWnd);

            fieldsRemaining--;
            if(gameState == GameState::InProgress && fieldsRemaining <= MineCount)
                WinLogic(mainWindow);

            if (gameState == GameState::InProgress && fields[t.y][t.x].type == FieldState::BOMB)
                FailureLogic(mainWindow);

            UpdateWindow(hWnd);
        }
    }
    break;
    case WM_ERASEBKGND:
    {
        POINT t = mapHWND[hWnd];
        HBRUSH brush;
        if (fields[t.y][t.x].IsClicked == false)
            brush = (HBRUSH)(COLOR_GRAYTEXT);
        else
            brush = CreateSolidBrush(RGB(0xC8, 0xC8, 0xC8));

        HDC hdc = GetDC(hWnd);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
        RECT rc;
        GetClientRect(hWnd, &rc);
        FillRect(hdc, &rc, brush);
        SelectObject(hdc, oldBrush);
        DeleteObject(brush);
        ReleaseDC(hWnd, hdc);
        return 1;
    }
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
        EndPaint(hWnd, &ps);
        UpdateWindow(hWnd);
    }
    break;
    case WM_CLOSE:
        // https://stackoverflow.com/questions/9166040/how-can-i-close-a-child-window-without-closing-the-parent
        DestroyWindow(hWnd);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
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

// Restarts the game to fit new parameters.
// The Main Window is initialized in the InitInstance function and not destroyed throughout the work time of the program
void gameReload(HWND hWnd, int minefieldWidth, int minefieldHeight, int mineCount)
{
    FieldWidth = minefieldWidth;
    FieldHeight = minefieldHeight;
    MineCount = mineCount;
    adjustMainWindowRect(minefieldWidth, minefieldHeight);
    MoveWindow(hWnd, baseRect.left, baseRect.top, baseRect.right - baseRect.left, baseRect.bottom - baseRect.top, TRUE);
    spawnMineField(hWnd, minefieldWidth, minefieldHeight, mineCount);
    timerRestart();
    flagsRemaining = mineCount;
    flagsCorrect = mineCount;
    gameState = GameState::ReadyToStart;
    fieldsRemaining = minefieldWidth * minefieldHeight;
    SendMessage(mainWindow, WM_CREATE, NULL, NULL);
    PaintTopPanel();
    return;
}

// Adjusts the rectangle parameters of the Main Window to fit new minefield size
// Does not apply those parameters to the window - to apply the parameters you still have to use the MoveWindow function or other
void adjustMainWindowRect(int minefieldWidth, int minefieldHeight)
{
    int SpaceWidth, SpaceHeight, LeftSpace, TopSpace;
    SpaceWidth = minefieldWidth * BlockLen + (minefieldWidth - 1) * GapLen;
    SpaceHeight = TopPanel + minefieldHeight * BlockLen + (minefieldHeight - 1) * GapLen;
    LeftSpace = screenWidth / 2 - SpaceWidth / 2;
    TopSpace = screenHeight / 2 - SpaceHeight / 2;

    baseRect.top = screenHeight / 2 - SpaceHeight / 2;
    baseRect.bottom = screenHeight / 2 + SpaceHeight / 2;
    baseRect.left = screenWidth / 2 - SpaceWidth / 2;
    baseRect.right = screenWidth / 2 + SpaceWidth / 2;
    AdjustWindowRect(&baseRect, WS_OVERLAPPEDWINDOW, TRUE);
    return;
}

void spawnMineField(HWND hWnd, int minefieldWidth, int minefieldHeight, int mineCount)
{
    int x, y, m;
    POINT tmp;

    if (fields[0][0].hWnd != NULL) {
        for (y = 0; y < maxHeight; y++)
            for (x = 0; x < maxWidth; x++)
                DestroyWindow(fields[y][x].hWnd);
    }
    mapHWND.clear();

    for (y = 0; y < minefieldHeight; y++) {
        for (x = 0; x < minefieldWidth; x++) {
            fields[y][x].type = FieldState::CLEAR;
            fields[y][x].IsClicked = false;
            fields[y][x].IsFlagged = false;
            fields[y][x].hWnd = CreateWindowW(L"Field", NULL, WS_CHILD | WS_VISIBLE, 26 * x, 26 * y + TopPanel, BlockLen, BlockLen, hWnd, nullptr, hInst, nullptr);
            ShowWindow(fields[y][x].hWnd, SW_NORMAL);
            UpdateWindow(fields[y][x].hWnd);
            tmp.x = x;
            tmp.y = y;
            mapHWND[fields[y][x].hWnd] = tmp;
        }
    }

    // choose mine sites
    int minesPlaced = 0;
    while (minesPlaced < mineCount) {
        x = rand() % minefieldWidth;
        y = rand() % minefieldHeight;

        if (fields[y][x].type != FieldState::BOMB) {
            fields[y][x].type = FieldState::BOMB;
			for (int a = -1; a <= 1; a++) {
				for (int b = -1; b <= 1; b++) {
                    if (a == 0 && b == 0)
                        continue;

					if (x + a >= 0 && y + b >= 0 && x + a < minefieldWidth && y + b < minefieldHeight)
                        if(fields[y + b][x + a].type != FieldState::BOMB)
						    fields[y + b][x + a].type++;
				}
			}
            minesPlaced++;
        }
    }
    if (GetMenuState(GetMenu(hWnd), ID_HELP_DEBUG, MF_BYCOMMAND) == MF_CHECKED) {
        DebugRevealFields(true);
    }
    else {
        DebugRevealFields(false);
    }

    return;
}

// Clears out the safe field when a CLEAR tile is clicked
void FieldBFS(HWND hWnd, int x, int y) {
    const int bufSize = 5;
    TCHAR buf[bufSize];
    std::queue<std::pair<int, int>> q;
    std::pair<int, int> tmp;
    bool visited[maxHeight][maxWidth] = { false };
    BFSwork = true;
    q.push(std::make_pair(y, x));
    while (!q.empty()) {
        tmp = q.front();
        q.pop();
        
        if (fields[tmp.first][tmp.second].type != FieldState::BOMB) {
            SendMessage(fields[tmp.first][tmp.second].hWnd, WM_LBUTTONUP, NULL, NULL);

            if (fields[tmp.first][tmp.second].type != FieldState::CLEAR)
                continue;

            for (int a = -1; a <= 1; a++) {
                for (int b = -1; b <= 1; b++) {
                    if (tmp.second + a < 0 || tmp.first + b < 0 || tmp.second + a >= FieldWidth || tmp.first + b >= FieldHeight)
                        continue;

                    if (visited[tmp.first + b][tmp.second + a] == false && fields[tmp.first + b][tmp.second + a].IsFlagged == false) {
                        visited[tmp.first + b][tmp.second + a] = true;
                        q.push(std::make_pair(tmp.first + b, tmp.second + a));
                    }
                }
            }
        }
    }
    BFSwork = false;
    return;
}

void timerStart() {
    if (gameState == GameState::ReadyToStart) {
        SetTimer(mainWindow, TITLETIMER, 100, NULL);
        gameState = GameState::InProgress;
    }
    return;
}

void timerRestart() {
    if (gameState == GameState::InProgress) {
			KillTimer(mainWindow, TITLETIMER);
            timeCount = 0;
    }
    return;
}

void WinLogic(HWND hWnd) {
    timerRestart();
    gameState = GameState::Finished;
    DebugRevealFields(true);
    int msgboxID = MessageBox(
        NULL,
        (LPCWSTR)L"Nicely done!",
        (LPCWSTR)L"Achieved success",
         MB_OK
    );
}

void FailureLogic(HWND hWnd) {
    timerRestart();
    gameState = GameState::Finished;
    DebugRevealFields(true);
    int msgboxID = MessageBox(
        NULL,
        (LPCWSTR)L"Happy accident",
        (LPCWSTR)L"Achieved failure",
        MB_ICONSTOP | MB_OK
    );
}

void DebugRevealField(int x, int y, bool reveal) {
    const int bufSize = 10;
    TCHAR buf[bufSize] = { NULL };

    if (reveal == true) {
        HDC hdc = GetDC(fields[y][x].hWnd);
        HBRUSH brush = brushes[0];
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
        RECT rc;
        switch (fields[y][x].type) {
        case BOMB:
            Ellipse(hdc, 5, 5, 20, 20);
            break;
        case CLEAR:
            break;
        default:
            SIZE psizl;
            HFONT oldFont = (HFONT)SelectObject(hdc, fontArial);
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, colors[fields[y][x].type]);
            _stprintf_s(buf, 2, _T("%c"), (fields[y][x].type + '0'));
            GetTextExtentPoint(hdc, buf, 1, &psizl);
            TextOut(hdc, psizl.cx / 2, 1, buf, 2);
            break;
        }
        ReleaseDC(fields[y][x].hWnd, hdc);
        UpdateWindow(fields[y][x].hWnd);
    }
    else
    {
        if (fields[y][x].IsFlagged == true) {
            PaintFlag(fields[y][x].hWnd, true);
            return;
        }

        if (fields[y][x].IsClicked == true)
            return;

        HDC hdc = GetDC(fields[y][x].hWnd);
        SendMessage(fields[y][x].hWnd, WM_ERASEBKGND, (WPARAM)hdc, NULL);
        HBRUSH brush = brushes[0];
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
        RECT rc;
        ReleaseDC(fields[y][x].hWnd, hdc);
        UpdateWindow(fields[y][x].hWnd);
    }
    return;
}

void DebugRevealFields(bool reveal) {
	for (int y = 0; y < FieldHeight; y++) {
		for (int x = 0; x < FieldWidth; x++) {
			DebugRevealField(x, y, reveal);
		}
	}
	return;
}

void PaintTopPanel()
{
    TCHAR buf[10];
    SIZE psizl;
    HDC hdc = GetDC(mainWindow);
    HFONT oldFont = (HFONT)SelectObject(hdc, fontArial);
    int SpaceWidth = FieldWidth * BlockLen + (FieldWidth - 1) * GapLen;
    SetTextColor(hdc, colors[3]);
    _stprintf_s(buf, 7, _T("%06.1f"), timeCount);
    GetTextExtentPoint(hdc, buf, 6, &psizl);
    TextOut(hdc, SpaceWidth / 4 - psizl.cx / 2, TopPanel / 2 - psizl.cy / 2, buf, 7);

    _stprintf_s(buf, 5, _T("%04d"), flagsRemaining);
    GetTextExtentPoint(hdc, buf, 4, &psizl);
    TextOut(hdc, SpaceWidth * 3 / 4 - psizl.cx / 2, TopPanel / 2 - psizl.cy / 2, buf, 5);

    SelectObject(hdc, oldFont);
    ReleaseDC(mainWindow, hdc);
    UpdateWindow(mainWindow);
    return;
}

void PaintTile(HWND hWndFld)
{
    TCHAR buf[3];
    POINT p = mapHWND[hWndFld];
    HDC hdc = GetDC(hWndFld);
    SendMessage(hWndFld, WM_ERASEBKGND, NULL, NULL);
    HBRUSH brush = brushes[0];
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    RECT rc;
    switch (fields[p.y][p.x].type) {
    case BOMB:
        Ellipse(hdc, 5, 5, 20, 20);
        break;
    case CLEAR:
        if (BFSwork == false)
            FieldBFS(hWndFld, p.x, p.y);
        break;
    default:
        SIZE psizl;
        HFONT oldFont = (HFONT)SelectObject(hdc, fontArial);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, colors[fields[p.y][p.x].type]);
        _stprintf_s(buf, 2, _T("%c"), (fields[p.y][p.x].type + '0'));
        GetTextExtentPoint(hdc, buf, 1, &psizl);
        TextOut(hdc, psizl.cx / 2, 1, buf, 2);

        break;
    }
    ReleaseDC(hWndFld, hdc);
}

void PaintFlag(HWND hWndFld, bool paint) {
    POINT p = mapHWND[hWndFld];

    if (paint == true)
    {
        PAINTSTRUCT ps;
        HDC hdc = GetDC(hWndFld);
        HBITMAP bitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1));
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bitmap);
        BitBlt(hdc, 0, 0, 48, 48, memDC, 0, 0, SRCCOPY);
        StretchBlt(hdc, 0, 0, 25, 25, memDC, 0, 0, 48, 48, SRCCOPY);
        SelectObject(memDC, oldBitmap);
        DeleteObject(bitmap);
        DeleteDC(memDC);
    }
    else
    {
        if (fields[p.y][p.x].IsClicked == true) {
            PaintTile(fields[p.y][p.x].hWnd);
        }
        else {
            SendMessage(hWndFld, WM_ERASEBKGND, NULL, NULL);
        }

    }

    if (GetMenuState(GetMenu(mainWindow), ID_HELP_DEBUG, MF_BYCOMMAND) == MF_CHECKED) {
        DebugRevealField(p.x, p.y, true);
    }

    UpdateWindow(hWndFld);
    return;
}

void RepaintWindow()
{
    PaintTopPanel();
    for (int y = 0; y < FieldHeight; y++) {
        for (int x = 0; x < FieldWidth; x++){
            if (fields[y][x].IsFlagged == true) {
                PaintFlag(fields[y][x].hWnd, true);
                continue;
            }

            if (fields[y][x].IsClicked == true) {
                PaintTile(fields[y][x].hWnd);
                continue;
            }

            if (GetMenuState(GetMenu(mainWindow), ID_HELP_DEBUG, MF_BYCOMMAND) == MF_CHECKED) {
                DebugRevealField(x, y, true);
            }
        }
    }

    return;
}

void CustomSize() {
    DialogBox(hInst, MAKEINTRESOURCE(IDD_CUSTOMSIZE), mainWindow, (DLGPROC)CustomSizeProc);
    return;
}

BOOL CALLBACK CustomSizeProc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_INITDIALOG:
	{
        tmpFieldHeight = FieldHeight;
        tmpFieldWidth = FieldWidth;
        tmpMineCount = MineCount;

        SetDlgItemInt(hWndDlg, IDC_EDIT1, tmpFieldHeight, false);
        SetDlgItemInt(hWndDlg, IDC_EDIT2, tmpFieldWidth, false);
        SetDlgItemInt(hWndDlg, IDC_EDIT3, tmpMineCount, false);

        // Centering dialog window
        //https://docs.microsoft.com/en-us/windows/win32/dlgbox/using-dialog-boxes#creating-a-modal-dialog-box
        RECT rc, rcDlg, rcOwner;
        GetWindowRect(mainWindow, &rcOwner);
        GetWindowRect(hWndDlg, &rcDlg);
        CopyRect(&rc, &rcOwner);
        OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
        OffsetRect(&rc, -rc.left, -rc.top);
        OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);
        SetWindowPos(hWndDlg,
            HWND_TOP,
            rcOwner.left + (rc.right / 2),
            rcOwner.top + (rc.bottom / 2),
            0, 0,          // Ignores size arguments. 
            SWP_NOSIZE);
	}
	break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_CANCEL:
			EndDialog(hWndDlg, LOWORD(wParam));
			break;
		case IDC_CREATE:
		{
			gameReload(mainWindow, tmpFieldWidth, tmpFieldHeight, tmpMineCount);
			EndDialog(hWndDlg, LOWORD(wParam));
		}
		break;
		case IDC_EDIT1:
		{
			tmpFieldHeight = GetDlgItemInt(hWndDlg, IDC_EDIT1, NULL, false);
			if (tmpFieldHeight < minHeigth)
				tmpFieldHeight = minHeigth;
			if (tmpFieldHeight > maxHeight)
				tmpFieldHeight = maxHeight;
		}
		break;
        case IDC_EDIT2:
        {
            tmpFieldWidth = GetDlgItemInt(hWndDlg, IDC_EDIT2, NULL, false);
            if (tmpFieldWidth < minWidth)
                tmpFieldWidth = minWidth;
            if (tmpFieldWidth > maxWidth)
                tmpFieldWidth = maxWidth;
        }
        break;
        case IDC_EDIT3:
        {
            tmpMineCount = GetDlgItemInt(hWndDlg, IDC_EDIT3, NULL, false);
            if (tmpMineCount < startMineCount)
                tmpMineCount = startMineCount;
            if (tmpMineCount > tmpFieldHeight * tmpFieldWidth)
                tmpMineCount = tmpFieldHeight * tmpFieldWidth;
        }
        break;
    }
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hWndDlg, &ps);
		EndPaint(hWndDlg, &ps);
		UpdateWindow(hWndDlg);
	}
	break;
	case WM_CLOSE:
        EndDialog(hWndDlg, LOWORD(wParam));
		break;
	default:
        break;
	}
	return 0;
}