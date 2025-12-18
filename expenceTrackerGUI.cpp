#include <windows.h>
#include <vector>
#include <string>
#include <fstream>

struct Expense {
    std::string category;
    std::string desc;
    std::string amount;
    std::string date;
};

std::vector<Expense> expenses;
HWND hCat, hDesc, hAmt, hList, hTotal, hMonth, hMonthly;

std::string today() {
    SYSTEMTIME t;
    GetLocalTime(&t);
    char buf[20];
    wsprintfA(buf, "%02d-%02d-%04d", t.wDay, t.wMonth, t.wYear);
    return buf;
}

int currentMonth() {
    SYSTEMTIME t;
    GetLocalTime(&t);
    return t.wMonth;
}

void RefreshTotal() {
    double sum = 0;
    for (auto &e : expenses)
        sum += atof(e.amount.c_str());

    char buf[50];
    sprintf(buf, "Total: Rs. %.2f", sum);
    SetWindowTextA(hTotal, buf);
}

void RefreshMonthly() {
    int sel = SendMessage(hMonth, CB_GETCURSEL, 0, 0);

    if (sel == CB_ERR || sel == 0) {
        SetWindowTextA(hMonthly, "Monthly Expense: Rs. 0");
        return;
    }

    char monStr[3];
    sprintf(monStr, "%02d", sel);

    double sum = 0;
    for (auto &e : expenses) {
        std::string m = e.date.substr(3, 2);
        if (m == monStr)
            sum += atof(e.amount.c_str());
    }

    char buf[60];
    sprintf(buf, "Monthly Expense (%s): Rs. %.2f", monStr, sum);
    SetWindowTextA(hMonthly, buf);
}

void SaveFile() {
    std::ofstream f("expenses.txt");
    for (auto &e : expenses)
        f << e.category << "|" << e.desc << "|" << e.amount << "|" << e.date << "\n";
}

void LoadFile() {
    std::ifstream f("expenses.txt");
    if (!f) return;

    std::string line;
    while (getline(f, line)) {
        size_t p1 = line.find("|");
        size_t p2 = line.find("|", p1 + 1);
        size_t p3 = line.find("|", p2 + 1);

        Expense e;
        e.category = line.substr(0, p1);
        e.desc = line.substr(p1 + 1, p2 - p1 - 1);
        e.amount = line.substr(p2 + 1, p3 - p2 - 1);
        e.date = line.substr(p3 + 1);

        expenses.push_back(e);

        std::string show =
            e.date + " | " + e.category + " | " + e.desc + " | Rs." + e.amount;

        SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)show.c_str());
    }

    RefreshTotal();
    RefreshMonthly();
}

void AddExpense() {
    int idx = SendMessage(hCat, CB_GETCURSEL, 0, 0);
    if (idx <= 0) return;

    char c[100];
    SendMessageA(hCat, CB_GETLBTEXT, idx, (LPARAM)c);

    char d[100], a[100];
    GetWindowTextA(hDesc, d, 100);
    GetWindowTextA(hAmt, a, 100);

    if (strlen(d) == 0 || strlen(a) == 0) return;

    Expense e;
    e.category = c;
    e.desc = d;
    e.amount = a;
    e.date = today();

    expenses.push_back(e);

    std::string show =
        e.date + " | " + e.category + " | " + e.desc + " | Rs." + e.amount;

    SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)show.c_str());

    SetWindowTextA(hDesc, "");
    SetWindowTextA(hAmt, "");

    RefreshTotal();
    RefreshMonthly();
    SaveFile();
}

void DeleteExpense() {
    int index = SendMessage(hList, LB_GETCURSEL, 0, 0);
    if (index < 0) return;

    SendMessage(hList, LB_DELETESTRING, index, 0);
    expenses.erase(expenses.begin() + index);

    RefreshTotal();
    RefreshMonthly();
    SaveFile();
}

void ClearAll() {
    expenses.clear();
    SendMessage(hList, LB_RESETCONTENT, 0, 0);

    RefreshTotal();
    RefreshMonthly();
    SaveFile();
}

LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    switch (m) {

    case WM_CREATE:

        CreateWindowA("STATIC", "Category:", WS_VISIBLE | WS_CHILD,
            20, 20, 100, 20, h, 0, 0, 0);

        hCat = CreateWindowA("COMBOBOX", "",
            WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
            120, 20, 200, 200, h, 0, 0, 0);

        SendMessageA(hCat, CB_ADDSTRING, 0, (LPARAM)"Select Category");
        SendMessageA(hCat, CB_ADDSTRING, 0, (LPARAM)"Food");
        SendMessageA(hCat, CB_ADDSTRING, 0, (LPARAM)"Travel");
        SendMessageA(hCat, CB_ADDSTRING, 0, (LPARAM)"Shopping");
        SendMessageA(hCat, CB_ADDSTRING, 0, (LPARAM)"Bills");
        SendMessageA(hCat, CB_ADDSTRING, 0, (LPARAM)"Others");
        SendMessage(hCat, CB_SETCURSEL, 0, 0);

        CreateWindowA("STATIC", "Description:", WS_VISIBLE | WS_CHILD,
            20, 60, 100, 20, h, 0, 0, 0);

        hDesc = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER,
            120, 60, 200, 20, h, 0, 0, 0);

        CreateWindowA("STATIC", "Amount:", WS_VISIBLE | WS_CHILD,
            20, 100, 100, 20, h, 0, 0, 0);

        hAmt = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER,
            120, 100, 200, 20, h, 0, 0, 0);

        CreateWindowA("BUTTON", "Add", WS_VISIBLE | WS_CHILD,
            20, 140, 80, 30, h, (HMENU)1, 0, 0);

        CreateWindowA("BUTTON", "Delete", WS_VISIBLE | WS_CHILD,
            120, 140, 80, 30, h, (HMENU)2, 0, 0);

        CreateWindowA("BUTTON", "Clear All", WS_VISIBLE | WS_CHILD,
            220, 140, 100, 30, h, (HMENU)3, 0, 0);

        CreateWindowA("STATIC", "Month:", WS_VISIBLE | WS_CHILD,
            20, 180, 80, 20, h, 0, 0, 0);

        hMonth = CreateWindowA("COMBOBOX", "",
            WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
            120, 180, 200, 200, h, (HMENU)10, 0, 0);

        SendMessage(hMonth, CB_ADDSTRING, 0, (LPARAM)"Select Month");
        for (int i = 1; i <= 12; i++) {
            char temp[3];
            sprintf(temp, "%02d", i);
            SendMessageA(hMonth, CB_ADDSTRING, 0, (LPARAM)temp);
        }

        SendMessage(hMonth, CB_SETCURSEL, currentMonth(), 0);

        hMonthly = CreateWindowA("STATIC", "Monthly Expense: Rs. 0",
            WS_VISIBLE | WS_CHILD, 20, 210, 300, 20, h, 0, 0, 0);

        hTotal = CreateWindowA("STATIC", "Total: Rs. 0",
            WS_VISIBLE | WS_CHILD, 20, 240, 200, 20, h, 0, 0, 0);

        hList = CreateWindowA("LISTBOX", "",
            WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOTIFY,
            20, 270, 440, 230, h, 0, 0, 0);

        LoadFile();
        break;

    case WM_COMMAND:
        if (w == 1) AddExpense();
        if (w == 2) DeleteExpense();
        if (w == 3) ClearAll();
        if (HIWORD(w) == CBN_SELCHANGE && (HWND)l == hMonth)
            RefreshMonthly();
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(h, m, w, l);
}

int WINAPI WinMain(HINSTANCE hi, HINSTANCE, LPSTR, int) {
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hi;
    wc.lpszClassName = "ExWin";
    RegisterClassA(&wc);

    CreateWindowA("ExWin", "Expense Tracker - Win32 GUI",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        200, 100, 500, 550, 0, 0, hi, 0);

    MSG msg;
    while (GetMessage(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
