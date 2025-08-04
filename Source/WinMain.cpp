#include <Windows.h>
#include <gl/GL.h>
#include <glcorearb.h>
#include <wglext.h>

static HDC                                  user_device_context         = nullptr;
static HGLRC                                program_opengl_context      = nullptr;

static float triangle_data[] =
{
    -0.5f, -0.5f,
    0.0f, 0.5f,
    0.5f, -0.5f
};

static GLuint triangle_vao = 0;
static GLuint triangle_vbo = 0;
static GLuint triangle_program = 0;

const char* vertex_shader = R"(
#version 330 core

layout (location = 0) in vec3 vertex_positions;

void main(void)
{
    gl_Position = vec4(vertex_positions, 1.0f);
}
)";

const char* fragment_shader = R"(
#version 330 core

out vec4 output_data;

void main(void)
{
    output_data = vec4(1.0, 0.0, 0.0, 1.0f);
}
)";


//
// WGL Function Pointers
// 

static PFNWGLCHOOSEPIXELFORMATARBPROC       wglChoosePixelFormatARB     = nullptr;
static PFNWGLCREATECONTEXTATTRIBSARBPROC    wglCreateContextAttribsARB  = nullptr;

//
// OpenGL Function Pointer
//

static PFNGLGENVERTEXARRAYSPROC             glGenVertexArrays           = nullptr;
static PFNGLBINDVERTEXARRAYPROC             glBindVertexArray           = nullptr;
static PFNGLDELETEVERTEXARRAYSPROC          glDeleteVertexArrays        = nullptr;

static PFNGLGENBUFFERSPROC                  glGenBuffers                = nullptr;
static PFNGLDELETEBUFFERSPROC               glDeleteBuffers             = nullptr;
static PFNGLBINDBUFFERPROC                  glBindBuffer                = nullptr;
static PFNGLBUFFERDATAPROC                  glBufferData                = nullptr;

static PFNGLVERTEXATTRIBPOINTERPROC         glVertexAttribPointer       = nullptr;
static PFNGLENABLEVERTEXATTRIBARRAYPROC     glEnableVertexAttribArray   = nullptr;

static PFNGLCREATESHADERPROC                glCreateShader              = nullptr;
static PFNGLDELETESHADERPROC                glDeleteShader              = nullptr;
static PFNGLSHADERSOURCEPROC                glShaderSource              = nullptr;
static PFNGLCOMPILESHADERPROC               glCompileShader             = nullptr;
static PFNGLGETSHADERIVPROC                 glGetShaderiv               = nullptr;
static PFNGLGETSHADERINFOLOGPROC            glGetShaderInfoLog          = nullptr;
static PFNGLATTACHSHADERPROC                glAttachShader              = nullptr;

static PFNGLCREATEPROGRAMPROC               glCreateProgram             = nullptr;
static PFNGLDELETEPROGRAMPROC               glDeleteProgram             = nullptr;
static PFNGLLINKPROGRAMPROC                 glLinkProgram               = nullptr;
static PFNGLGETPROGRAMIVPROC                glGetProgramiv              = nullptr;
static PFNGLGETPROGRAMINFOLOGPROC           glGetProgramInfoLog         = nullptr;
static PFNGLUSEPROGRAMPROC                  glUseProgram                = nullptr;

LRESULT CALLBACK WindowProc( HWND window_handle, UINT window_event, WPARAM window_param01, LPARAM window_param02 )
{
    switch ( window_event )
    {
        case WM_CLOSE:
        {
            glDeleteVertexArrays( 1, &triangle_vao );
            glDeleteBuffers( 1, &triangle_vbo );
            glDeleteProgram( triangle_program );
            wglDeleteContext( program_opengl_context );
            DestroyWindow( window_handle );
        } return 0;

        case WM_DESTROY:
        {
            PostQuitMessage( 0 );
        } return 0;

        case WM_PAINT:
        {
            glClearColor( 0.2f, 0.3f, 0.3f, 1.0f );
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            glBindVertexArray( triangle_vao );
            glDrawArrays( GL_TRIANGLES, 0, 3 );
            SwapBuffers( user_device_context );
        } return 0;

        case WM_SIZE:
        {
            glViewport( 0, 0, LOWORD( window_param02 ), HIWORD( window_param02 ) );
        } return 0;

    }

    return DefWindowProc( window_handle, window_event, window_param01, window_param02 );
}

int WINAPI WinMain
(
    [[maybe_unused]] _In_        HINSTANCE   hInstance,
    [[maybe_unused]] _In_opt_    HINSTANCE   hPrevInstance,
    [[maybe_unused]] _In_        PSTR        lpCmdLine,
    [[maybe_unused]] _In_        int         nCmdShow
)
{
    RECT                    window_frame                = { };
    WNDCLASS                window_class                = { };
    MSG                     window_event                = { };
    HWND                    window_handle               = nullptr;
    PIXELFORMATDESCRIPTOR   pixel_format_descriptor     = { };

    const char*             window_name                 = "WinOpenGL\0";
    int                     window_width                = 1024;
    int                     window_height               = 768;

    int                     color_depth_bits            = 32;
    int                     depth_buffer_bits           = 24;
    int                     stencil_buffer_bits         = 8;

    int                     dummy_pixel_format          = 0;

    int                     extension_format            = 0;
    UINT                    format_count                = 0;
    int                     pixel_format_attributes[]   =
    {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
        WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,     32,
        WGL_DEPTH_BITS_ARB,     24,
        WGL_STENCIL_BITS_ARB,   8,
        0
    };

    int                     core_extension_attributes[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB,  3,
        WGL_CONTEXT_MINOR_VERSION_ARB,  3,
        WGL_CONTEXT_PROFILE_MASK_ARB,   WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };


    window_class.style              = CS_OWNDC;
    window_class.lpfnWndProc        = &WindowProc;
    window_class.cbClsExtra         = 0;
    window_class.cbWndExtra         = 0;
    window_class.hInstance          = hInstance;
    window_class.hIcon              = nullptr;
    window_class.hCursor            = nullptr;
    window_class.hbrBackground      = nullptr;
    window_class.lpszMenuName       = nullptr;
    window_class.lpszClassName      = window_name;

    RegisterClass( &window_class );

    window_handle = CreateWindowEx
    (
        0,
        window_class.lpszClassName,
        window_class.lpszClassName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        window_width,
        window_height,
        nullptr,
        nullptr,
        window_class.hInstance,
        nullptr
    );

    if ( window_handle == nullptr )
    {
        return EXIT_FAILURE;
    }

    pixel_format_descriptor.nSize           = sizeof( PIXELFORMATDESCRIPTOR );
    pixel_format_descriptor.nVersion        = 1;
    pixel_format_descriptor.dwFlags         = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pixel_format_descriptor.iPixelType      = PFD_TYPE_RGBA;
    pixel_format_descriptor.cColorBits      = static_cast<BYTE>( color_depth_bits );
    pixel_format_descriptor.cRedBits        = 0;
    pixel_format_descriptor.cRedShift       = 0;
    pixel_format_descriptor.cGreenBits      = 0;
    pixel_format_descriptor.cGreenShift     = 0;
    pixel_format_descriptor.cBlueBits       = 0;
    pixel_format_descriptor.cBlueShift      = 0;
    pixel_format_descriptor.cAlphaBits      = 0;
    pixel_format_descriptor.cAlphaShift     = 0;
    pixel_format_descriptor.cAccumBits      = 0;
    pixel_format_descriptor.cAccumRedBits   = 0;
    pixel_format_descriptor.cAccumGreenBits = 0;
    pixel_format_descriptor.cAccumBlueBits  = 0;
    pixel_format_descriptor.cAccumAlphaBits = 0;
    pixel_format_descriptor.cDepthBits      = static_cast<BYTE>( depth_buffer_bits );
    pixel_format_descriptor.cStencilBits    = static_cast<BYTE>( stencil_buffer_bits );
    pixel_format_descriptor.cAuxBuffers     = 0;
    pixel_format_descriptor.iLayerType      = PFD_MAIN_PLANE;
    pixel_format_descriptor.bReserved       = 0;
    pixel_format_descriptor.dwLayerMask     = 0;
    pixel_format_descriptor.dwVisibleMask   = 0;
    pixel_format_descriptor.dwDamageMask    = 0;

    user_device_context = GetDC( window_handle );

    dummy_pixel_format = ChoosePixelFormat( user_device_context, &pixel_format_descriptor );
    SetPixelFormat( user_device_context, dummy_pixel_format, &pixel_format_descriptor );
    program_opengl_context = wglCreateContext( user_device_context );
    wglMakeCurrent( user_device_context, program_opengl_context );

    wglChoosePixelFormatARB     = reinterpret_cast< PFNWGLCHOOSEPIXELFORMATARBPROC >( wglGetProcAddress( "wglChoosePixelFormatARB" ) );
    wglCreateContextAttribsARB  = reinterpret_cast< PFNWGLCREATECONTEXTATTRIBSARBPROC >( wglGetProcAddress( "wglCreateContextAttribsARB" ) );

    wglMakeCurrent( nullptr, nullptr );
    wglDeleteContext( program_opengl_context );

    wglChoosePixelFormatARB( user_device_context, pixel_format_attributes, nullptr, 1, &extension_format, &format_count );
    ZeroMemory( &pixel_format_descriptor, sizeof( PIXELFORMATDESCRIPTOR ) );
    DescribePixelFormat( user_device_context, extension_format, sizeof( PIXELFORMATDESCRIPTOR ), &pixel_format_descriptor );
    SetPixelFormat( user_device_context, extension_format, &pixel_format_descriptor );
    program_opengl_context = wglCreateContextAttribsARB( user_device_context, nullptr, core_extension_attributes );
    wglMakeCurrent( user_device_context, program_opengl_context );

    glGenVertexArrays = reinterpret_cast<PFNGLGENVERTEXARRAYSPROC>( wglGetProcAddress( "glGenVertexArrays" ) );
    glBindVertexArray = reinterpret_cast<PFNGLBINDVERTEXARRAYPROC>( wglGetProcAddress( "glBindVertexArray" ) );
    glDeleteVertexArrays = reinterpret_cast< PFNGLDELETEVERTEXARRAYSPROC > ( wglGetProcAddress( "glDeleteVertexArrays" ) );

    glGenBuffers = reinterpret_cast<PFNGLGENBUFFERSPROC>( wglGetProcAddress( "glGenBuffers" ) );
    glDeleteBuffers = reinterpret_cast< PFNGLDELETEBUFFERSPROC > ( wglGetProcAddress( "glDeleteBuffers" ) );
    glBindBuffer = reinterpret_cast<PFNGLBINDBUFFERPROC>( wglGetProcAddress( "glBindBuffer" ) );
    glBufferData = reinterpret_cast<PFNGLBUFFERDATAPROC>( wglGetProcAddress( "glBufferData" ) );
    
    glVertexAttribPointer = reinterpret_cast<PFNGLVERTEXATTRIBPOINTERPROC>( wglGetProcAddress( "glVertexAttribPointer" ) );
    glEnableVertexAttribArray = reinterpret_cast<PFNGLENABLEVERTEXATTRIBARRAYPROC>( wglGetProcAddress( "glEnableVertexAttribArray" ) );
    
    glCreateShader = reinterpret_cast<PFNGLCREATESHADERPROC>( wglGetProcAddress( "glCreateShader" ) );
    glDeleteShader = reinterpret_cast<PFNGLDELETESHADERPROC>( wglGetProcAddress( "glDeleteShader" ) );
    glShaderSource = reinterpret_cast< PFNGLSHADERSOURCEPROC >( wglGetProcAddress( "glShaderSource" ) );
    glCompileShader = reinterpret_cast<PFNGLCOMPILESHADERPROC>( wglGetProcAddress( "glCompileShader" ) );
    glGetShaderiv = reinterpret_cast<PFNGLGETSHADERIVPROC>( wglGetProcAddress( "glGetShaderiv" ) );
    glGetShaderInfoLog = reinterpret_cast<PFNGLGETSHADERINFOLOGPROC>( wglGetProcAddress( "glGetShaderInfoLog" ) );
    glAttachShader = reinterpret_cast<PFNGLATTACHSHADERPROC>( wglGetProcAddress( "glAttachShader" ) );
    
    glCreateProgram = reinterpret_cast<PFNGLCREATEPROGRAMPROC>( wglGetProcAddress( "glCreateProgram" ) );
    glDeleteProgram = reinterpret_cast< PFNGLDELETEPROGRAMPROC > ( wglGetProcAddress( "glDeleteProgram" ) );
    glLinkProgram = reinterpret_cast<PFNGLLINKPROGRAMPROC>( wglGetProcAddress( "glLinkProgram" ) );
    glGetProgramiv = reinterpret_cast<PFNGLGETPROGRAMIVPROC>( wglGetProcAddress( "glGetProgramiv" ) );
    glGetProgramInfoLog = reinterpret_cast<PFNGLGETPROGRAMINFOLOGPROC>( wglGetProcAddress( "glGetProgramInfoLog" ) );
    glUseProgram = reinterpret_cast< PFNGLUSEPROGRAMPROC >( wglGetProcAddress( "glUseProgram" ) );

    glGenVertexArrays( 1, &triangle_vao );
    glBindVertexArray( triangle_vao );

    glGenBuffers( 1, &triangle_vbo );

    glBindBuffer( GL_ARRAY_BUFFER, triangle_vbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof( triangle_data ), triangle_data, GL_STATIC_DRAW );

    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( float ), reinterpret_cast< void* >( 0 * sizeof(void*) ) );
    glEnableVertexAttribArray( 0 );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );

    GLuint vertex_shader_id = glCreateShader( GL_VERTEX_SHADER );
    glShaderSource( vertex_shader_id, 1, &vertex_shader, nullptr );
    glCompileShader( vertex_shader_id );

    int shader_status = 0;
    glGetShaderiv( vertex_shader_id, GL_COMPILE_STATUS, &shader_status );
    char buffer[4096];

    if ( shader_status == 0 )
    {
        glGetShaderInfoLog( vertex_shader_id, sizeof(buffer), nullptr, buffer);
        MessageBox( nullptr, buffer, "WinOpenGL - Engine Error", MB_ICONERROR | MB_OK );
        return 1;
    }

    GLuint fragment_shader_id = glCreateShader( GL_FRAGMENT_SHADER );
    glShaderSource( fragment_shader_id, 1, &fragment_shader, nullptr );
    glCompileShader( fragment_shader_id );

    glGetShaderiv( fragment_shader_id, GL_COMPILE_STATUS, &shader_status );

    if ( shader_status == 0 )
    {
        glGetShaderInfoLog( fragment_shader_id, sizeof( buffer ), nullptr, buffer );
        MessageBox( nullptr, buffer, "WinOpenGL - Engine Error", MB_ICONERROR | MB_OK );
        return 1;
    }

    triangle_program = glCreateProgram( );
    glAttachShader( triangle_program, vertex_shader_id );
    glAttachShader( triangle_program, fragment_shader_id );
    glLinkProgram( triangle_program );

    glGetProgramiv( triangle_program, GL_LINK_STATUS, &shader_status );
    if ( shader_status == 0 )
    {
        glGetProgramInfoLog( triangle_program, sizeof( buffer ), nullptr, buffer );
        MessageBox( nullptr, buffer, "WinOpenGL - Engine Error", MB_ICONERROR | MB_OK );
        return 1;
    }

    glDeleteShader( vertex_shader_id );
    glDeleteShader( fragment_shader_id );

    glUseProgram( triangle_program );

    ShowWindow( window_handle, nCmdShow );

    while ( GetMessage( &window_event, nullptr, 0, 0 ) > 0 )
    {
        TranslateMessage( &window_event );
        DispatchMessage( &window_event );
    }

    return EXIT_SUCCESS;
}