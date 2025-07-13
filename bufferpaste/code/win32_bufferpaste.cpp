#include <windows.h>
#include <stdio.h>
#include "resource.h"

enum ButtonIDs {
    ID_EDIT1 = 1001,
    ID_EDIT2,
    ID_EDIT3,
    ID_EDIT4,
    ID_EDIT5,
    ID_EDIT6,
    ID_EDIT7,
    ID_EDIT8,
    ID_EDIT9,
    ID_EDIT10,
    ID_BUTTON_SAVE1,
    ID_BUTTON_SAVE2,
    ID_BUTTON_SAVE3,
    ID_BUTTON_SAVE4,
    ID_BUTTON_SAVE5,
    ID_BUTTON_SAVE6,
    ID_BUTTON_SAVE7,
    ID_BUTTON_SAVE8,
    ID_BUTTON_SAVE9,
    ID_BUTTON_SAVE10,
    ID_BUTTON_COPY1,
    ID_BUTTON_COPY2,
    ID_BUTTON_COPY3,
    ID_BUTTON_COPY4,
    ID_BUTTON_COPY5,
    ID_BUTTON_COPY6,
    ID_BUTTON_COPY7,
    ID_BUTTON_COPY8,
    ID_BUTTON_COPY9,
    ID_BUTTON_COPY10,
    ID_BUTTON_CLEAR_ALL,
    ID_STATIC1,
    ID_CHECKBOX_ALWAYS_ON_TOP,
    ID_DOUBLE_THE_BUFFERS
};

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

int g_NumOfBuffers = 10;

typedef struct debug_read_file_result
{
    uint32 ContentsSize;
    void *Contents;
} debug_read_file_result;

inline uint32
SafeTruncateUInt64(uint64 Value)
{
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
debug_read_file_result ReadFromFile(WCHAR *filename);
bool32 ReadBuffersFromFiles(HWND Window);
bool32 WriteBufferToFileUTF16(int editId, WCHAR *buffer, uint32 textLength);
bool32 DeleteBufferFiles();
bool32 SaveClipboardToBufferUTF16(HWND Window, int editId);
bool32 CopyBufferToClipboard(HWND Window, int editId);
bool32 ClearAllBuffers(HWND Window);

//Filename = "bufferpaste-%d.txt";

local_persist int startup = 1;

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
            
            // Create g_NumOfBuffers buffer rows
            
            int yPos = g_NumOfBuffers * 4;
            int rowHeight = 50;
            
            for (int i = 0; i < g_NumOfBuffers; i++) {
                char label[20];
                snprintf(label, sizeof(label), "Buffer %d:", i + 1);
                
                // Label
                CreateWindow("STATIC", label,
                             WS_VISIBLE | WS_CHILD,
                             10, yPos + 5, 65, 20,
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
                         250, (g_NumOfBuffers * 54), 120, 35,
                         Window, (HMENU)(UINT_PTR)ID_BUTTON_CLEAR_ALL, NULL, NULL);
            
            // Always on Top checkbox
            CreateWindow("BUTTON", "Always on Top",
                         WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                         400, (g_NumOfBuffers * 54), 120, 35,
                         Window, (HMENU)(UINT_PTR)ID_CHECKBOX_ALWAYS_ON_TOP, NULL, NULL);
            
            // Read Buffers from Files
            if(startup)
            {
                if(!ReadBuffersFromFiles(Window))
                {
                    MessageBoxW(Window, L"Could not read some files into the buffers!!", L"Error", MB_OK | MB_ICONERROR);
                }
                
                // start program with checkbox Always On Top checked
                SendMessage(GetDlgItem(Window, ID_CHECKBOX_ALWAYS_ON_TOP), BM_SETCHECK, BST_CHECKED, 0);
                startup = 0;
            }
        } break;
        
        case WM_COMMAND:
        {
            int wmId = LOWORD(WParam);
            int wmHiId = HIWORD(WParam);
            
            // Handle Save buttons (save clipboard to buffer)
            if(wmId >= ID_BUTTON_SAVE1 && wmId <= ID_BUTTON_SAVE10)
            {
                int bufferIndex = wmId - ID_BUTTON_SAVE1;
                SaveClipboardToBufferUTF16(Window, ID_EDIT1 + bufferIndex);
            }
            
            // Handle Copy buttons (copy buffer to clipboard)
            else if(wmId >= ID_BUTTON_COPY1 && wmId <= ID_BUTTON_COPY10)
            {
                int bufferIndex = wmId - ID_BUTTON_COPY1;
                if(!CopyBufferToClipboard(Window, ID_EDIT1 +  bufferIndex))
                {
                    MessageBoxW(Window, L"Unable to copy buffer to clipboard!!", L"Error", MB_OK | MB_ICONERROR);
                }
            }
            
            // Handle Clear All button
            else if(wmId == ID_BUTTON_CLEAR_ALL)
            {
                ClearAllBuffers(Window);
                if(!DeleteBufferFiles())
                {
                    MessageBoxW(Window, L"Cannot delete Files!!", L"Error", MB_OK | MB_ICONERROR);
                }
            }
            
            // Handle Always On Top
            else if( wmId == ID_CHECKBOX_ALWAYS_ON_TOP && wmHiId == BN_CLICKED )
            {
                // Get checkbox state
                BOOL isChecked = (BOOL)SendMessage(GetDlgItem(Window, ID_CHECKBOX_ALWAYS_ON_TOP), BM_GETCHECK, 0, 0);
                
                if (isChecked)
                {
                    SetWindowPos(Window, HWND_TOPMOST, 0, 0, 0, 0,
                                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                }
                else
                {
                    SetWindowPos(Window, HWND_NOTOPMOST, 0, 0, 0, 0,
                                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                }
            }
        } break;
        
        case WM_SIZE:
        {
            OutputDebugStringA("WM_SIZE\n");
        } break;
        
        case WM_CLOSE:
        {
            DestroyWindow(Window);   
            Result = 0;
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
            Result = 0;
        }
        
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;
        
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }
    
    return(Result);
}

debug_read_file_result
ReadFromFile(WCHAR *Filename)
{
    debug_read_file_result Result = {};
    
    HANDLE FileHandle = CreateFileW(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
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
                    Result.ContentsSize = FileSize32;
                }
                else
                {                    
                    VirtualFree(Result.Contents, 0, MEM_RELEASE);
                }
            }
            else
            {
            }
        }
        else
        {
        }
        
        CloseHandle(FileHandle);
    }
    else
    {
    }
    
    return(Result);
}

bool32
DeleteBufferFiles()
{
    bool32 Result = 1;
    WCHAR Filename[64];
    
    for(int i = 0; i < g_NumOfBuffers; i++)
    {
        _snwprintf_s(Filename, ArrayCount(Filename), _TRUNCATE, L"bufferpaste-%d.txt", ID_EDIT1 + i);
        
        // Check if file exists before trying to delete
        DWORD attributes = GetFileAttributesW(Filename);
        if(attributes != INVALID_FILE_ATTRIBUTES)
        {
            
            // File exists delete it
            if(!DeleteFileW(Filename))
            {
                Result = 0;
            }
            else
            {
                // If file does not exist it's ok nothing to delete
            }
        }
    }
    
    return(Result);
    
}

bool32
ReadBuffersFromFiles(HWND hwnd)
{
    bool32 Result = 1;
    
    WCHAR Filename[64];
    for(int i = 0; i < g_NumOfBuffers; i++)
    {
        _snwprintf_s(Filename, ArrayCount(Filename), _TRUNCATE, L"bufferpaste-%d.txt", ID_EDIT1 + i);
        
        // Check if file exists before reading the file
        DWORD attributes = GetFileAttributesW(Filename);
        if(attributes != INVALID_FILE_ATTRIBUTES)
        {
            debug_read_file_result File_Read_Result = ReadFromFile(Filename);
            
            if(File_Read_Result.Contents && File_Read_Result.ContentsSize > 0)
            {
                SetWindowTextW(GetDlgItem(hwnd, ID_EDIT1 + i), (WCHAR*)File_Read_Result.Contents);
                VirtualFree(File_Read_Result.Contents, 0, MEM_RELEASE);
            }
            else
            {
                // Could not read any data from file!!
                Result = 0;
            }
        }
        else
        {
            // File does not exist so no read to read it
        }
    }
    
    return(Result);
}

bool32
WriteBufferToFileUTF16(int editId, WCHAR *buffer, uint32 textLength)
{
    bool32 Result = 1;
    
    WCHAR filename[64];
    _snwprintf_s(filename, ArrayCount(filename), _TRUNCATE, L"bufferpaste-%d.txt", editId);
    
    HANDLE FileHandle = CreateFileW(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        DWORD BytesToWrite = textLength * sizeof(WCHAR);
        
        if(WriteFile(FileHandle, buffer, BytesToWrite, &BytesWritten, 0))
        {
            Result = (BytesWritten == BytesToWrite);
        }
        else
        {
            Result = 0;
        }
        
        CloseHandle(FileHandle);
    }
    else
    {
        Result = 0;
    }
    
    return(Result);
}


bool32
SaveClipboardToBufferUTF16(HWND hwnd, int editId)
{
    bool32 Result = 1;
    
    if (!OpenClipboard(hwnd)) {
        MessageBoxW(hwnd, L"Cannot open clipboard!", L"Error", MB_OK | MB_ICONERROR);
        Result = 0;
    }
    
    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData == NULL) {
        CloseClipboard();
        MessageBoxW(hwnd, L"No text data in clipboard!", L"Info", MB_OK | MB_ICONINFORMATION);
        Result = 0;
    }
    
    WCHAR* pszText = (WCHAR *)GlobalLock(hData);
    if (pszText != NULL) {
        // Set the text in the edit control
        SetWindowTextW(GetDlgItem(hwnd, editId), pszText);
        GlobalUnlock(hData);
        
        // Show confirmation
        WCHAR msg[100];
        _snwprintf_s(msg, ArrayCount(msg), _TRUNCATE, L"Clipboard content saved to Buffer %d", editId - ID_EDIT1 + 1);
        SetWindowTextW(GetDlgItem(hwnd, ID_STATIC1), msg);
        
        int textLength = GetWindowTextLengthW(GetDlgItem(hwnd, editId));
        void* fileBuffer = VirtualAlloc(0, ((textLength + 1) * sizeof(WCHAR)), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        GetWindowTextW(GetDlgItem(hwnd, editId), (WCHAR *)fileBuffer, textLength + 1);
        WriteBufferToFileUTF16(editId, (WCHAR *)fileBuffer, textLength);
        VirtualFree(fileBuffer, 0, MEM_RELEASE);
    }
    
    CloseClipboard();
    
    return(Result);
}

bool32
CopyBufferToClipboard(HWND hwnd, int editId)
{
    bool32 Result = 1;
    
    // Get text from edit control
    int textLength = GetWindowTextLengthW(GetDlgItem(hwnd, editId));
    if (textLength == 0) {
        MessageBoxW(hwnd, L"Buffer is empty!", L"Info", MB_OK | MB_ICONINFORMATION);
        Result = 0;
    }
    
    void* buffer = VirtualAlloc(0, ((textLength + 1) * sizeof(WCHAR)), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    GetWindowTextW(GetDlgItem(hwnd, editId), (WCHAR *)buffer, textLength + 1);
    
    if (!OpenClipboard(hwnd)) {
        MessageBoxW(hwnd, L"Cannot open clipboard!", L"Error", MB_OK | MB_ICONERROR);
        Result = 0;
    }
    
    EmptyClipboard();
    
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (textLength + 1) * sizeof(WCHAR));
    if (hMem != NULL) {
        WCHAR* pMem = (WCHAR*)GlobalLock(hMem);
        int actualLength = GetWindowTextW(GetDlgItem(hwnd, editId), pMem, textLength + 1);
        pMem[actualLength] = L'\0';
        GlobalUnlock(hMem);
        
        SetClipboardData(CF_UNICODETEXT, hMem);
        
        // Show confirmation
        WCHAR msg[100];
        _snwprintf_s(msg, ArrayCount(msg), _TRUNCATE, L"Buffer %d copied to clipboard", editId - ID_EDIT1 + 1);
        SetWindowTextW(GetDlgItem(hwnd, ID_STATIC1), msg);
    }
    
    CloseClipboard();
    VirtualFree(buffer, 0, MEM_RELEASE);
    
    return(Result);
}

bool32
ClearAllBuffers(HWND hwnd)
{
    bool32 Result = 1;
    
    for (int i = 0; i < g_NumOfBuffers; i++) {
        SetWindowText(GetDlgItem(hwnd, ID_EDIT1 + i), "");
    }
    SetWindowText(GetDlgItem(hwnd, ID_STATIC1), "All buffers cleared");
    
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
    WindowClass.hIcon = LoadIcon(Instance, MAKEINTRESOURCE(IDI_MYICON));
    WindowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WindowClass.lpszClassName = "BufferPaste";
    
    if(RegisterClass(&WindowClass))
    {
        HWND WindowHandle =
            CreateWindowEx(
                           WS_EX_TOPMOST,
                           WindowClass.lpszClassName,
                           TEXT("BufferPaste"),
                           WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           700, 625,
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
