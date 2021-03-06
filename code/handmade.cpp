#include <windows.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>

#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;


struct win32_offscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

struct win32_window_dimension
{
    int Width;
    int Height;
};

global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackbuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return(0);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN  pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal void
Win32LoadXInput(void)
{
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if(!XInputLibrary)
    {
	XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }
    if(XInputLibrary)
    {
	XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
	if(!XInputGetState)
	{
	    XInputGetState = XInputGetStateStub;
	}

	XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
	if(!XInputSetState)
	{
	    XInputSetState = XInputSetStateStub;
	}
    }
}

internal void
Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
    if(DSoundLibrary)
    {
	direct_sound_create *DirectSoundCreate = (direct_sound_create *) GetProcAddress(DSoundLibrary, "DirectSoundCreate");

	LPDIRECTSOUND DirectSound;
	if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
	{
	    WAVEFORMATEX WaveFormat = {};
	    WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	    WaveFormat.nChannels = 2;
	    WaveFormat.nSamplesPerSec = SamplesPerSecond;
	    WaveFormat.wBitsPerSample = 16;
	    WaveFormat.nBlockAlign = (WaveFormat.nChannels*WaveFormat.wBitsPerSample)/8;
	    WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec*WaveFormat.nBlockAlign;
	    WaveFormat.cbSize = 0;

	    if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
	    {
		DSBUFFERDESC BufferDescription = {};
		BufferDescription.dwSize = sizeof(BufferDescription);
		BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

		LPDIRECTSOUNDBUFFER PrimaryBuffer;
		if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
		{
		    HRESULT Error = PrimaryBuffer->SetFormat(&WaveFormat);
		    
		    if(SUCCEEDED(Error))
		    {
			OutputDebugStringA("Primary buffer format was set.\n");
		    } else
		    {
		    }
		}
		else
		{
		}
	    }
	    else
	    {
	    }
	    
	    DSBUFFERDESC BufferDescription = {};
	    BufferDescription.dwSize = sizeof(BufferDescription);
	    BufferDescription.dwFlags = 0;
	    BufferDescription.dwBufferBytes = BufferSize;
	    BufferDescription.lpwfxFormat = &WaveFormat;
	    HRESULT Error = DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0);
	    if(SUCCEEDED(Error))
	    {
		OutputDebugStringA("Secondary buffer created successfully.\n");
	    }
	}
	else
	{
	}
	
    }
    else
    {
    }
}

internal void
Win32InitDSound(void)
{
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");

    if(DSoundLibrary)
    {
	
    }
}

win32_window_dimension Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return(Result);
}

internal void Foo()
{
    //int a = 0;
    //int *b = &a;
    //OutputDebugStringA(b);
}

struct Structure
{

};

void testFunc()
{

}

internal void
RenderWeirdGradient(win32_offscreen_buffer Buffer,
		    int BlueOffset, int GreenOffset)
{
    uint8 *Row = (uint8 *)Buffer.Memory;
    for(int Y = 0; Y < Buffer.Height; ++Y)
    {
	uint32 *Pixel = (uint32 *)Row;
	for(int X = 0; X < Buffer.Width; ++X)
	{
	    uint8 Blue = (X + BlueOffset);
	    uint8 Green = (Y + GreenOffset);
	    *Pixel++ = ((Green << 8) | Blue);
	}
	Row += Buffer.Pitch;
    }
}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
    if(Buffer->Memory)
    {
	VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;
    int BytesPerPixel = 4;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = (Buffer->Width*Buffer->Height)*BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    Buffer->Pitch = Width*BytesPerPixel;

}

internal void
Win32CopyBufferToWindow(HDC DeviceContext,
			int WindowWidth, int WindowHeight,
			win32_offscreen_buffer Buffer,
			int X, int Y, int Width, int Height)
{
    StretchDIBits(DeviceContext,
		  0, 0, WindowWidth, WindowHeight,
		  0, 0, Buffer.Width, Buffer.Height,
		  Buffer.Memory,
		  &Buffer.Info,
		  DIB_RGB_COLORS, SRCCOPY);

}

LRESULT CALLBACK WindowProc(HWND Window,
			    UINT   Message,
			    WPARAM wParam,
			    LPARAM lParam)
{
    LRESULT Result = 0;
    switch(Message)
    {
	case WM_SIZE:
	{
	} break;
	case WM_DESTROY:
	{
	    GlobalRunning = false;
	    OutputDebugStringA("WM_DESTROY\n");
	} break;

	case WM_CLOSE:
	{
	    GlobalRunning = false;
	    OutputDebugStringA("WM_CLOSE\n");
	} break;

	case WM_ACTIVATEAPP:
	{
	    OutputDebugStringA("WM_ACTIVATEAPP\n");
	} break;

	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
	    uint32 VKCode = wParam;
	    bool WasDown = ((lParam & (1 << 30)) != 0);
	    bool IsDown = ((lParam & (1 << 31)) == 0);
	    if(WasDown != IsDown)
	    {
		if(VKCode == 'W')
		{
		    OutputDebugStringA("W: ");
		    if(IsDown)
		    {
			OutputDebugStringA("is down\n");
		    }
		    if(WasDown)
		    {
			OutputDebugStringA("was down\n");
		    }

		}
		if(VKCode == 'A')
		{
		    OutputDebugStringA("A\n");
		}
		if(VKCode == 'S')
		{
		    OutputDebugStringA("S\n");
		}
		if(VKCode == 'D')
		{
		    OutputDebugStringA("D\n");
		}
	    }
	}

	case WM_PAINT:
	{
	    PAINTSTRUCT Paint;
	    HDC DeviceContext = BeginPaint(Window, &Paint);
	    int X = Paint.rcPaint.left;
	    int Y = Paint.rcPaint.top;
	    int Width = Paint.rcPaint.right - Paint.rcPaint.left;
	    int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
	    win32_window_dimension Dimension = Win32GetWindowDimension(Window);
	    Win32CopyBufferToWindow(DeviceContext, Dimension.Width, Dimension.Height, GlobalBackbuffer, X, Y, Width, Height);
	    EndPaint(Window, &Paint);
	} break;

	default:
	{
	    Result = DefWindowProc(Window, Message, wParam, lParam);
	} break;
    }
    return(Result);
}

int CALLBACK WinMain(HINSTANCE hInstance,
		     HINSTANCE hPrevInstance,
		     PSTR lpCmdLine,
		     int nCmdShow)
{
    Win32LoadXInput();

    WNDCLASS WindowClass = {};
    Win32ResizeDIBSection(&GlobalBackbuffer,
			  1280, 720);


    WindowClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.hInstance = hInstance;
    // hIcon;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";
    if(RegisterClassA(&WindowClass))
    {
	HWND WindowHandle = CreateWindowExA(0,
					    WindowClass.lpszClassName,
					    "HandmadeHero",
					    WS_OVERLAPPEDWINDOW|WS_VISIBLE,
					    CW_USEDEFAULT,
					    CW_USEDEFAULT,
					    CW_USEDEFAULT,
					    CW_USEDEFAULT,
					    0,
					    0,
					    hInstance,
					    0);
	if(WindowHandle)
	{
	    HDC DeviceContext = GetDC(Window);
	    
	    int XOffset = 0;
	    int YOffset = 0;

	    
	    
	    GlobalRunning = true;
	    while(GlobalRunning)
	    {
		MSG Message;
		while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
		{
		    if(Message.message == WM_QUIT)
		    {
			GlobalRunning = false;
		    }

		    TranslateMessage(&Message);
		    DispatchMessageA(&Message);
		}

		for(DWORD ControllerIndex = 0;
		    ControllerIndex < XUSER_MAX_COUNT;
		    ++ControllerIndex)
		{
		    XINPUT_STATE ControllerState = {};
		    if(XInputGetState(ControllerIndex, &ControllerState)
		       == ERROR_SUCCESS)
		    {
			XINPUT_GAMEPAD *Gamepad = &ControllerState.Gamepad;
			bool Up = (Gamepad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
			bool Down = (Gamepad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
			bool Left = (Gamepad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
			bool Right = (Gamepad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
			bool Start = (Gamepad->wButtons & XINPUT_GAMEPAD_START);
			bool Back = (Gamepad->wButtons & XINPUT_GAMEPAD_BACK);
			bool LeftShoulder = (Gamepad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
			bool RightShoulder = (Gamepad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
			bool AButton = (Gamepad->wButtons & XINPUT_GAMEPAD_A);
			bool XButton = (Gamepad->wButtons & XINPUT_GAMEPAD_X);
			bool BButton = (Gamepad->wButtons & XINPUT_GAMEPAD_B);
			bool YButton = (Gamepad->wButtons & XINPUT_GAMEPAD_Y);

			int16 StickX = Gamepad->sThumbLX;
			int16 StickY = Gamepad->sThumbLY;

			XOffset += StickX >> 12;
			YOffset -= StickY >> 12;
		    } else
		    {
		    }
		}

		RenderWeirdGradient(GlobalBackbuffer, XOffset, YOffset);
		HDC DeviceContext = GetDC(WindowHandle);
		win32_window_dimension Dimension = Win32GetWindowDimension(WindowHandle);
		Win32CopyBufferToWindow(DeviceContext,
					Dimension.Width, Dimension.Height,
					GlobalBackbuffer, 0, 0,
					Dimension.Width,
					Dimension.Height);
		ReleaseDC(WindowHandle, DeviceContext);

		++XOffset;
		YOffset += 2;

	    }
	}
	else
	{
	}
    }
    return(0);
}
