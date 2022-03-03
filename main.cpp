#include <windows.h>
#include <iostream>
#include <unordered_map>

/*
待解决的问题：在对象是桌面的时候，按快捷键整理窗口，理想情况是不做反应(通过句柄排除)，但实际出现BUG。
*/

#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)
#define MAXWIN 4 // 最大窗口数量
#define shortcut_key1 VK_CONTROL // 整理窗口的快捷键1 Control
#define shortcut_key2 MOUSE_MOVED // 整理窗口的快捷键2 鼠标左键(MOUSE_MOVED)
#define shortcut_key3 VK_SPACE // 整理窗口的快捷键3 空格

using namespace std;

unordered_map<HWND, int> map; // 通过句柄得到编号
unordered_map<int, HWND> mapInd; // 通过编号得到句柄

// 全局变量
int maxX = GetSystemMetrics(SM_CXSCREEN);// 获取屏幕大小
int maxY = GetSystemMetrics(SM_CYSCREEN) - 50; // 去除底部菜单栏的50px
double p = 0.55; // 窗口比例
HWND desktopHWND = GetDesktopWindow(); // 获取桌面句柄，用于排除，不知道为啥出现BUG

// 对已记录窗口的状态进行检测
void inspectAll()
{
    HWND tmp;
    for (int i = 1; i <= map.size(); i++)
    {
        tmp = mapInd[i];
        // 是否销毁or最小化
        if ( !IsWindow(tmp) || IsIconic(tmp))
        {
            map.erase(tmp);// 删除
            for (int j = i+1; j <= map.size(); j++) // 后面的窗口前移
            {
                map[mapInd[j]] = j - 1; 
                mapInd[j - 1] = mapInd[j];

            }
            continue;
        }
    }
    return;
}

// 对所有窗口的排序方案
void sortAll(HWND target)
 {
    
    // 查看map里是否有这个值
    if (map.find(target) == map.end())
    {// 未找到，说明是新窗口
        // 若之前已经达到最大窗口数
        if (map.size() == MAXWIN) {
            // 把最末尾的窗口最小化，并移除map
            HWND tmp = mapInd[MAXWIN];
            ShowWindow(tmp,SW_MINIMIZE); // 最小化
            map.erase(tmp); // 移除
        }
        // 窗口排序依次后移
        for (int i = map.size()+1; i >1; i--) {
                mapInd[i] = mapInd[i - 1];
                map[mapInd[i - 1]] = i;
        }

        //焦点窗口移到最前
        mapInd[1] = target; 
        map[target] = 1;

        //排序结束
        return;
    }
    else 
    {// 是之前存在的窗口
        int ind=map[target];
        for (int i = ind-1; i >0; i--)
        {
            mapInd[i + 1] = mapInd[i];
            map[mapInd[i]] = i + 1;
        }
        mapInd[1] = target;
        map[target] = 1;

        //排序结束
        return;
    }
}

// 对所有窗口的移动方案
void moveAll() 
{
      int width = maxX, height = maxY;
      int startPointX = 0;// 初始化窗口起点
      int startPointY = 0;
    // 对非最后一个窗口进行处理
    for (int i = 1; i < map.size(); i++)
    {
        // i为奇数，缩进宽度
        // i为偶数，缩进高度
        if (i % 2) 
        {
            width = width * p;
        }
        else
        {
            height = height * p;
        }

        MoveWindow(mapInd[i], startPointX, startPointY, width, height, false);

        // i为奇数，startPointX移动
        // i为偶数，startPointY移动
        if (i % 2)
        {
            startPointX += width;
            width = width / p * (1 - p);
            
        }
        else
        {   
            startPointY += height;
            height = height / p * (1 - p);
        }
    }

    // 对最后一个窗口进行处理
    MoveWindow(mapInd[map.size()], startPointX, startPointY, width, height, true);

}

int main()
{
    printf("Ctr+Space：显示/隐藏窗口\nCtr+左键：窗口管理\n");
    //获取控制台句柄,用于之后对控制台窗口的控制。
    HWND hwnd;
    int show = 1;
    hwnd = GetConsoleWindow();	// 获取控制台窗口句柄
    
    POINT point;        // 鼠标所在位置
    HWND parent;        // 目标父窗口句柄
    HWND target;        // 目标窗口句柄
    while (1) // 循环获取焦点
    {
        //检测按键(快捷键1+3)，用于隐藏or显示控制台
        if (KEY_DOWN(shortcut_key1))
        {
            Sleep(100); // 延迟100ms
            if (KEY_DOWN(shortcut_key3)) // ctr + SPACE
            {
                if (show)
                {
                    ShowWindow(hwnd, SW_HIDE);
                }
                else
                {
                    ShowWindow(hwnd, SW_SHOW);
                }
                show = 1 - show;
                Sleep(1000); // 延迟100ms
            }
        }

        // 检测按键(快捷键1+2)，用于整理窗口
        if (KEY_DOWN(shortcut_key1))
        {
            Sleep(100); // 延迟100ms
            if (KEY_DOWN(shortcut_key2)) //ctr + 左键
            {
                target = GetForegroundWindow(); // 获取当前窗口句柄
                if (target != NULL && target != INVALID_HANDLE_VALUE && target != desktopHWND)
                {
                    // 循环获取最外层句柄
                    parent = GetParent(target);
                    while (parent != NULL && parent != INVALID_HANDLE_VALUE && parent != desktopHWND)
                    {
                        target = parent;
                        parent = GetParent(target);
                    }

                    inspectAll(); // 检测记录中的窗口状态
                    sortAll(target);// 对所有窗口进行排序
                    moveAll();// 对所有窗口进行移动

                    Sleep(1000); // 延迟1000ms

                }
            }
        }
        
    } 


    return 0;
}