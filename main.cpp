#include <windows.h>
#include <iostream>
#include <unordered_map>

/*
����������⣺�ڶ����������ʱ�򣬰���ݼ������ڣ���������ǲ�����Ӧ(ͨ������ų�)����ʵ�ʳ���BUG��
*/

#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)
#define MAXWIN 4 // ��󴰿�����
#define shortcut_key1 VK_CONTROL // �����ڵĿ�ݼ�1 Control
#define shortcut_key2 MOUSE_MOVED // �����ڵĿ�ݼ�2 ������(MOUSE_MOVED)
#define shortcut_key3 VK_SPACE // �����ڵĿ�ݼ�3 �ո�

using namespace std;

unordered_map<HWND, int> map; // ͨ������õ����
unordered_map<int, HWND> mapInd; // ͨ����ŵõ����

// ȫ�ֱ���
int maxX = GetSystemMetrics(SM_CXSCREEN);// ��ȡ��Ļ��С
int maxY = GetSystemMetrics(SM_CYSCREEN) - 50; // ȥ���ײ��˵�����50px
double p = 0.55; // ���ڱ���
HWND desktopHWND = GetDesktopWindow(); // ��ȡ�������������ų�����֪��Ϊɶ����BUG

// ���Ѽ�¼���ڵ�״̬���м��
void inspectAll()
{
    HWND tmp;
    for (int i = 1; i <= map.size(); i++)
    {
        tmp = mapInd[i];
        // �Ƿ�����or��С��
        if ( !IsWindow(tmp) || IsIconic(tmp))
        {
            map.erase(tmp);// ɾ��
            for (int j = i+1; j <= map.size(); j++) // ����Ĵ���ǰ��
            {
                map[mapInd[j]] = j - 1; 
                mapInd[j - 1] = mapInd[j];

            }
            continue;
        }
    }
    return;
}

// �����д��ڵ����򷽰�
void sortAll(HWND target)
 {
    
    // �鿴map���Ƿ������ֵ
    if (map.find(target) == map.end())
    {// δ�ҵ���˵�����´���
        // ��֮ǰ�Ѿ��ﵽ��󴰿���
        if (map.size() == MAXWIN) {
            // ����ĩβ�Ĵ�����С�������Ƴ�map
            HWND tmp = mapInd[MAXWIN];
            ShowWindow(tmp,SW_MINIMIZE); // ��С��
            map.erase(tmp); // �Ƴ�
        }
        // �����������κ���
        for (int i = map.size()+1; i >1; i--) {
                mapInd[i] = mapInd[i - 1];
                map[mapInd[i - 1]] = i;
        }

        //���㴰���Ƶ���ǰ
        mapInd[1] = target; 
        map[target] = 1;

        //�������
        return;
    }
    else 
    {// ��֮ǰ���ڵĴ���
        int ind=map[target];
        for (int i = ind-1; i >0; i--)
        {
            mapInd[i + 1] = mapInd[i];
            map[mapInd[i]] = i + 1;
        }
        mapInd[1] = target;
        map[target] = 1;

        //�������
        return;
    }
}

// �����д��ڵ��ƶ�����
void moveAll() 
{
      int width = maxX, height = maxY;
      int startPointX = 0;// ��ʼ���������
      int startPointY = 0;
    // �Է����һ�����ڽ��д���
    for (int i = 1; i < map.size(); i++)
    {
        // iΪ�������������
        // iΪż���������߶�
        if (i % 2) 
        {
            width = width * p;
        }
        else
        {
            height = height * p;
        }

        MoveWindow(mapInd[i], startPointX, startPointY, width, height, false);

        // iΪ������startPointX�ƶ�
        // iΪż����startPointY�ƶ�
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

    // �����һ�����ڽ��д���
    MoveWindow(mapInd[map.size()], startPointX, startPointY, width, height, true);

}

int main()
{
    printf("Ctr+Space����ʾ/���ش���\nCtr+��������ڹ���\n");
    //��ȡ����̨���,����֮��Կ���̨���ڵĿ��ơ�
    HWND hwnd;
    int show = 1;
    hwnd = GetConsoleWindow();	// ��ȡ����̨���ھ��
    
    POINT point;        // �������λ��
    HWND parent;        // Ŀ�길���ھ��
    HWND target;        // Ŀ�괰�ھ��
    while (1) // ѭ����ȡ����
    {
        //��ⰴ��(��ݼ�1+3)����������or��ʾ����̨
        if (KEY_DOWN(shortcut_key1))
        {
            Sleep(100); // �ӳ�100ms
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
                Sleep(1000); // �ӳ�100ms
            }
        }

        // ��ⰴ��(��ݼ�1+2)������������
        if (KEY_DOWN(shortcut_key1))
        {
            Sleep(100); // �ӳ�100ms
            if (KEY_DOWN(shortcut_key2)) //ctr + ���
            {
                target = GetForegroundWindow(); // ��ȡ��ǰ���ھ��
                if (target != NULL && target != INVALID_HANDLE_VALUE && target != desktopHWND)
                {
                    // ѭ����ȡ�������
                    parent = GetParent(target);
                    while (parent != NULL && parent != INVALID_HANDLE_VALUE && parent != desktopHWND)
                    {
                        target = parent;
                        parent = GetParent(target);
                    }

                    inspectAll(); // ����¼�еĴ���״̬
                    sortAll(target);// �����д��ڽ�������
                    moveAll();// �����д��ڽ����ƶ�

                    Sleep(1000); // �ӳ�1000ms

                }
            }
        }
        
    } 


    return 0;
}