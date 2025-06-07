#include <windows.h>

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
            CreateWindow("STATIC", "BUF0:", WS_VISIBLE | WS_CHILD,
                    20, 20, 80, 20, Window, (HMENU)1004, NULL, NULL);
            CreateWindow("STATIC", "BUF1:", WS_VISIBLE | WS_CHILD,
                    20, 60, 80, 20, Window, (HMENU)1004, NULL, NULL);
            CreateWindow("STATIC", "BUF2:", WS_VISIBLE | WS_CHILD,
                    20, 100, 80, 20, Window, (HMENU)1004, NULL, NULL);
            CreateWindow("STATIC", "BUF3:", WS_VISIBLE | WS_CHILD,
                    20, 140, 80, 20, Window, (HMENU)1004, NULL, NULL);
        }
        case WM_SIZE:
        {
            OutputDebugStringA("WM_SIZE\n");
        } break;

        case WM_DESTROY:
        {
          PostQuitMessage(0);  
          return 0;
        }

        case WM_CLOSE:
        {
            DestroyWindow(Window);   
        }

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            static DWORD Operation = WHITENESS;
            PatBlt(DeviceContext, X, Y, Width, Height, Operation);
            if(Operation == WHITENESS)
            {
                Operation = BLACKNESS;
            }
            else
            {
                Operation = WHITENESS;
            }

            EndPaint(Window, &Paint);
        } break;

        default:
        {
            // OutputDebugStringA("default\n");
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }

    return(Result);
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
    WindowClass.lpszClassName = "CopyPasta";

    if(RegisterClass(&WindowClass))
    {
        HWND WindowHandle =
            CreateWindowExA(
                    0,
                    WindowClass.lpszClassName,
                    "CopyPasta",
                    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    400,
                    225,
                    0,
                    0,
                    Instance,
                    0);
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
