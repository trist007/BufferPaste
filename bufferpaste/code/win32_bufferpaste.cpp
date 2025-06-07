#include <windows.h>

#define internal static

#define ID_EDIT1 1001
#define ID_EDIT2 1002
#define ID_EDIT3 1003
#define ID_EDIT4 1004
#define ID_EDIT5 1005
#define ID_BUTTON_SAVE1 1006
#define ID_BUTTON_SAVE2 1007
#define ID_BUTTON_SAVE3 1008
#define ID_BUTTON_SAVE4 1009
#define ID_BUTTON_SAVE5 1010
#define ID_BUTTON_COPY1 1011
#define ID_BUTTON_COPY2 1012
#define ID_BUTTON_COPY3 1013
#define ID_BUTTON_COPY4 1014
#define ID_BUTTON_COPY5 1015
#define ID_BUTTON_CLEAR_ALL 1016
#define ID_STATIC1 1017

LRESULT CALLBACK MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);
void SaveClipboardToBuffer(HWND Window, int editId);
void CopyBufferToClipboard(HWND Window, int editId);
void ClearAllBuffers(HWND Window);

LRESULT CALLBACK
MainWindowCallback(HWND Window,
        UINT Message,
        WPARAM WParam,
        LPARAM LParam)
{
    LRESULT Result = 0;
    switch(Message)
    {
        case WM_CREATE:
        {
            // Create title
            CreateWindow("STATIC", "BufferPaste - Save clipboard content to buffers, then copy back when needed",
                WS_VISIBLE | WS_CHILD,
                10, 10, 600, 20,
                Window, (HMENU)(UINT_PTR)ID_STATIC1, NULL, NULL);
            
            // Create 5 buffer rows
            int yPos = 50;
            int rowHeight = 50;
            
            for (int i = 0; i < 5; i++) {
                char label[20];
                wsprintf(label, "Buffer %d:", i + 1);
                
                // Label
                CreateWindow("STATIC", label,
                    WS_VISIBLE | WS_CHILD,
                    10, yPos + 5, 60, 20,
                    Window, NULL, NULL, NULL);
                
                // Text field (larger, multiline)
                CreateWindow("EDIT", "",
                    WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_MULTILINE | WS_VSCROLL,
                    80, yPos, 350, 35,
                    Window, (HMENU)(UINT_PTR)(ID_EDIT1 + i), NULL, NULL);
                
                // Save button (save clipboard to this buffer)
                CreateWindow("BUTTON", "Save",
                    WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                    440, yPos, 60, 30,
                    Window, (HMENU)(UINT_PTR)(ID_BUTTON_SAVE1 + i), NULL, NULL);
                
                // Copy button (copy this buffer to clipboard)
                CreateWindow("BUTTON", "Copy",
                    WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                    510, yPos, 60, 30,
                    Window, (HMENU)(UINT_PTR)(ID_BUTTON_COPY1 + i), NULL, NULL);
                
                yPos += rowHeight;
            }
            
            // Clear All button
            CreateWindow("BUTTON", "Clear All Buffers",
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                250, 320, 120, 35,
                Window, (HMENU)(UINT_PTR)ID_BUTTON_CLEAR_ALL, NULL, NULL);
        } break;

        case WM_COMMAND:
        {
            int wmId = LOWORD(WParam);

            // Handle Save buttons (save clipboard to buffer)
            if(wmId >= ID_BUTTON_SAVE1 && wmId <= ID_BUTTON_SAVE5)
            {
                int bufferIndex = wmId - ID_BUTTON_SAVE1;
                SaveClipboardToBuffer(Window, ID_EDIT1 + bufferIndex);
            }

            // Handle Copy buttons (copy buffer to clipboard)
            else if(wmId >= ID_BUTTON_COPY1 && wmId <= ID_BUTTON_COPY5)
            {
                int bufferIndex = wmId - ID_BUTTON_COPY1;
                CopyBufferToClipboard(Window, ID_EDIT1 +  bufferIndex);
            }

            // Handle Clear All button
            else if(wmId == ID_BUTTON_CLEAR_ALL)
            {
                ClearAllBuffers(Window);
            }
        } break;

        case WM_SIZE:
        {
            OutputDebugStringA("WM_SIZE\n");
        } break;

        case WM_CLOSE:
        {
            DestroyWindow(Window);   
            return 0;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            EndPaint(Window, &Paint);
        } break;

        case WM_DESTROY:
        {
          PostQuitMessage(0);  
          return 0;
        }

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;

        default:
        {
            // OutputDebugStringA("default\n");
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }

    return(Result);
}

void SaveClipboardToBuffer(HWND hwnd, int editId)
{
    if (!OpenClipboard(hwnd)) {
        MessageBox(hwnd, "Cannot open clipboard!", "Error", MB_OK | MB_ICONERROR);
        return;
    }
    
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == NULL) {
        CloseClipboard();
        MessageBox(hwnd, "No text data in clipboard!", "Info", MB_OK | MB_ICONINFORMATION);
        return;
    }
    
    char* pszText = (char*)GlobalLock(hData);
    if (pszText != NULL) {
        // Set the text in the edit control
        SetWindowText(GetDlgItem(hwnd, editId), pszText);
        GlobalUnlock(hData);
        
        // Show confirmation
        char msg[100];
        wsprintf(msg, "Clipboard content saved to Buffer %d", editId - ID_EDIT1 + 1);
        SetWindowText(GetDlgItem(hwnd, ID_STATIC1), msg);
    }
    
    CloseClipboard();
}

void CopyBufferToClipboard(HWND hwnd, int editId)
{
    // Get text from edit control
    int textLength = GetWindowTextLength(GetDlgItem(hwnd, editId));
    if (textLength == 0) {
        MessageBox(hwnd, "Buffer is empty!", "Info", MB_OK | MB_ICONINFORMATION);
        return;
    }
    
    char* buffer = (char*)malloc(textLength + 1);
    GetWindowText(GetDlgItem(hwnd, editId), buffer, textLength + 1);
    
    if (!OpenClipboard(hwnd)) {
        free(buffer);
        MessageBox(hwnd, "Cannot open clipboard!", "Error", MB_OK | MB_ICONERROR);
        return;
    }
    
    EmptyClipboard();
    
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, textLength + 1);
    if (hMem != NULL) {
        char* pMem = (char*)GlobalLock(hMem);
        strcpy_s(pMem, sizeof(buffer), buffer);
        GlobalUnlock(hMem);
        
        SetClipboardData(CF_TEXT, hMem);
        
        // Show confirmation
        char msg[100];
        wsprintf(msg, "Buffer %d copied to clipboard", editId - ID_EDIT1 + 1);
        SetWindowText(GetDlgItem(hwnd, ID_STATIC1), msg);
    }
    
    CloseClipboard();
    free(buffer);
}

void ClearAllBuffers(HWND hwnd)
{
    for (int i = 0; i < 5; i++) {
        SetWindowText(GetDlgItem(hwnd, ID_EDIT1 + i), "");
    }
    SetWindowText(GetDlgItem(hwnd, ID_STATIC1), "All buffers cleared");
}

int CALLBACK
WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CommandLine,
    int ShowCode)
{
    WNDCLASS WindowClass = {};
    // TODO(trist007): Check if HREDRAW/VREDRAW/OWNDC still matter

    WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = Instance;
    //WindowClass.hIcon;
    WindowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WindowClass.lpszClassName = "BufferPaste";

    if(RegisterClass(&WindowClass))
    {
        HWND WindowHandle =
            CreateWindowExA(
                    0,
                    WindowClass.lpszClassName,
                    "BufferPaste",
                    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    650, 400,
                    0,
                    0,
                    Instance,
                    0);

        ShowWindow(WindowHandle, ShowCode);
        UpdateWindow(WindowHandle);


        if(WindowHandle)
        {
            MSG Message;
            for(;;)
            {
                BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
                if(MessageResult > 0)
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            // TODO(trist007): Logging
        }
    }
    else
    {
        // TODO(trist007): Logging
    }

    return(0);
}
