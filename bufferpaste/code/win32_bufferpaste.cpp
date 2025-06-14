#include <windows.h>

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

#include <stdint.h>
#include <stddef.h>
    
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef size_t memory_index;
    
typedef float real32;
typedef double real64;

#define internal static 
#define local_persist static 
#define global_variable static

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#if DEBUG
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

typedef struct debug_read_file_result
{
    uint32 ContentsSize;
    void *Contents;
} debug_read_file_result;

inline uint32
SafeTruncateUInt64(uint64 Value)
{
    // TODO(casey): Defines for maximum values
    Assert(Value <= 0xFFFFFFFF);
    uint32 Result = (uint32)Value;
    return(Result);
}


inline FILETIME
Win32GetLastWriteTime(char *Filename)
{
    FILETIME LastWriteTime = {};

    WIN32_FILE_ATTRIBUTE_DATA Data;
    if(GetFileAttributesEx(Filename, GetFileExInfoStandard, &Data))
    {
        LastWriteTime = Data.ftLastWriteTime;
    }

    return(LastWriteTime);
}

LRESULT CALLBACK MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);
debug_read_file_result ReadFromFile(char *filename);
bool32 ReadBuffersFromFiles(HWND Window);
bool32 WriteBufferToFile(int editId, char *buffer, uint32 textLength);
void SaveClipboardToBuffer(HWND Window, int editId);
void CopyBufferToClipboard(HWND Window, int editId);
void ClearAllBuffers(HWND Window);

//Filename = "bufferpaste.txt";

LRESULT CALLBACK
MainWindowCallback(HWND Window,
        UINT Message,
        WPARAM WParam,
        LPARAM LParam)
{
    LRESULT Result = 0;
    int startup = 1;

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
                CreateWindow("BUTTON", "Copy to Clipboard",
                    WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                    510, yPos, 150, 30,
                    Window, (HMENU)(UINT_PTR)(ID_BUTTON_COPY1 + i), NULL, NULL);
                
                yPos += rowHeight;
            }
            
            // Clear All button
            CreateWindow("BUTTON", "Clear All Buffers",
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                250, 320, 120, 35,
                Window, (HMENU)(UINT_PTR)ID_BUTTON_CLEAR_ALL, NULL, NULL);

            // Read Buffers from Files
            if(startup)
            {
                ReadBuffersFromFiles(Window);
                startup = 0;
            }
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

debug_read_file_result
ReadFromFile(char *Filename)
{
    debug_read_file_result Result = {};
    
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize))
        {
            uint32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
            Result.Contents = VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(Result.Contents)
            {
                DWORD BytesRead;
                if(ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) &&
                   (FileSize32 == BytesRead))
                {
                    // NOTE(casey): File read successfully
                    Result.ContentsSize = FileSize32;
                }
                else
                {                    
                    // TODO(casey): Logging
                    Result.Contents = 0;
                }
            }
            else
            {
                // TODO(casey): Logging
            }
        }
        else
        {
            // TODO(casey): Logging
        }

        CloseHandle(FileHandle);
    }
    else
    {
        // TODO(casey): Logging
    }

    return(Result);
}

bool32
ReadBuffersFromFiles(HWND hwnd)
{
    char Filename[32];
    debug_read_file_result Result = {};
    for(int i = 0; i < 5; i++)
    {
        wsprintf(Filename, "bufferpaste-%d.txt", ID_EDIT1 + i);
        Result = ReadFromFile(Filename);
        SetWindowText(GetDlgItem(hwnd, ID_EDIT1 + i), (char*)Result.Contents);
    }

    return 1;
}

bool32
WriteBufferToFile(int editId, char *buffer, uint32 textLength)
{
    bool32 Result = false;

    char filename[32];
    wsprintf(filename, "bufferpaste-%d.txt", editId);
    
    HANDLE FileHandle = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        if(WriteFile(FileHandle, buffer, textLength, &BytesWritten, 0))
        {
            // NOTE(casey): File read successfully
            Result = (BytesWritten == textLength);
        }
        else
        {
            // TODO(casey): Logging
        }

        CloseHandle(FileHandle);
    }
    else
    {
        // TODO(casey): Logging
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

        int textLength = GetWindowTextLengthA(GetDlgItem(hwnd, editId));
        GetWindowTextA(GetDlgItem(hwnd, editId), pszText, textLength + 1);
        WriteBufferToFile(editId, pszText, textLength + 1);
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
        strcpy_s(pMem, textLength + 1, buffer);
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
                    WS_EX_TOPMOST,
                    WindowClass.lpszClassName,
                    "BufferPaste",
                    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    700, 400,
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
