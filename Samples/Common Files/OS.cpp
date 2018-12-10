// MIT License
//
// Copyright( c ) 2017 Packt
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Vulkan Cookbook
// ISBN: 9781786468154
// © Packt Publishing Limited
//
// Author:   Pawel Lapinski
// LinkedIn: https://www.linkedin.com/in/pawel-lapinski-84522329
//
// OS

#include "CookbookSampleFramework.h"

namespace VulkanCookbook {

#ifdef VK_USE_PLATFORM_WIN32_KHR

  namespace {
    enum UserMessage {
      USER_MESSAGE_RESIZE = WM_USER + 1,
      USER_MESSAGE_QUIT,
      USER_MESSAGE_MOUSE_CLICK,
      USER_MESSAGE_MOUSE_MOVE,
      USER_MESSAGE_MOUSE_WHEEL
    };
  }

  LRESULT CALLBACK WindowProcedure( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
    switch( message ) {
    case WM_LBUTTONDOWN:
      PostMessage( hWnd, USER_MESSAGE_MOUSE_CLICK, 0, 1 );
      break;
    case WM_LBUTTONUP:
      PostMessage( hWnd, USER_MESSAGE_MOUSE_CLICK, 0, 0 );
      break;
    case WM_RBUTTONDOWN:
      PostMessage( hWnd, USER_MESSAGE_MOUSE_CLICK, 1, 1 );
      break;
    case WM_RBUTTONUP:
      PostMessage( hWnd, USER_MESSAGE_MOUSE_CLICK, 1, 0 );
      break;
    case WM_MOUSEMOVE:
      PostMessage( hWnd, USER_MESSAGE_MOUSE_MOVE, LOWORD( lParam ), HIWORD( lParam ) );
      break;
    case WM_MOUSEWHEEL:
      PostMessage( hWnd, USER_MESSAGE_MOUSE_WHEEL, HIWORD( wParam ), 0 );
      break;
    case WM_SIZE:
    case WM_EXITSIZEMOVE:
      PostMessage( hWnd, USER_MESSAGE_RESIZE, wParam, lParam );
      break;
    case WM_KEYDOWN:
      if( VK_ESCAPE == wParam ) {
        PostMessage( hWnd, USER_MESSAGE_QUIT, wParam, lParam );
      }
      break;
    case WM_CLOSE:
      PostMessage( hWnd, USER_MESSAGE_QUIT, wParam, lParam );
      break;
    default:
      return DefWindowProc( hWnd, message, wParam, lParam );
    }
    return 0;
  }

  WindowFramework::WindowFramework( const char               * window_title,
                                    int                        x,
                                    int                        y,
                                    int                        width,
                                    int                        height,
                                    VulkanCookbookSampleBase & sample ) :
    WindowParams(),
    Sample( sample ),
    Created( false ) {
    WindowParams.HInstance = GetModuleHandle( nullptr );

    WNDCLASSEX window_class = {
      sizeof( WNDCLASSEX ),             // UINT         cbSize
                                        /* Win 3.x */
      CS_HREDRAW | CS_VREDRAW,          // UINT         style
      WindowProcedure,                  // WNDPROC      lpfnWndProc
      0,                                // int          cbClsExtra
      0,                                // int          cbWndExtra
      WindowParams.HInstance,           // HINSTANCE    hInstance
      nullptr,                          // HICON        hIcon
      LoadCursor( nullptr, IDC_ARROW ), // HCURSOR      hCursor
      (HBRUSH)(COLOR_WINDOW + 1),       // HBRUSH       hbrBackground
      nullptr,                          // LPCSTR       lpszMenuName
      "VulkanCookbook",                 // LPCSTR       lpszClassName
                                        /* Win 4.0 */
      nullptr                           // HICON        hIconSm
    };

    if( !RegisterClassEx( &window_class ) ) {
      return;
    }

    WindowParams.HWnd = CreateWindow( "VulkanCookbook", window_title, WS_OVERLAPPEDWINDOW, x, y, width, height, nullptr, nullptr, WindowParams.HInstance, nullptr );
    if( !WindowParams.HWnd ) {
      return;
    }

    Created = true;
  }

  WindowFramework::~WindowFramework() {
    if( WindowParams.HWnd ) {
      DestroyWindow( WindowParams.HWnd );
    }

    if( WindowParams.HInstance ) {
      UnregisterClass( "VulkanCookbook", WindowParams.HInstance );
    }
  }

  void WindowFramework::Render() {
    if( Created &&
        Sample.Initialize( WindowParams ) ) {

      ShowWindow( WindowParams.HWnd, SW_SHOWNORMAL );
      UpdateWindow( WindowParams.HWnd );

      MSG message;
      bool loop = true;

      while( loop ) {
        if( PeekMessage( &message, NULL, 0, 0, PM_REMOVE ) ) {
          switch( message.message ) {
          case USER_MESSAGE_MOUSE_CLICK:
            Sample.MouseClick( static_cast<size_t>(message.wParam), message.lParam > 0 );
            break;
          case USER_MESSAGE_MOUSE_MOVE:
            Sample.MouseMove( static_cast<int>(message.wParam), static_cast<int>(message.lParam) );
            break;
          case USER_MESSAGE_MOUSE_WHEEL:
            Sample.MouseWheel( static_cast<short>(message.wParam) * 0.002f );
            break;
          case USER_MESSAGE_RESIZE:
            if( !Sample.Resize() ) {
              loop = false;
            }
            break;
          case USER_MESSAGE_QUIT:
            loop = false;
            break;
          }
          TranslateMessage( &message );
          DispatchMessage( &message );
        } else {
          if( Sample.IsReady() ) {
            Sample.UpdateTime();
            Sample.Draw();
            Sample.MouseReset();
          }
        }
      }
    }

    Sample.Deinitialize();
  }


#elif defined VK_USE_PLATFORM_XCB_KHR

  WindowFramework::WindowFramework( const char               * window_title,
                                    int                        x,
                                    int                        y,
                                    int                        width,
                                    int                        height,
                                    VulkanCookbookSampleBase & sample ) :
    WindowParams(),
    Sample( sample ),
    Created( false ) {
    
    xcb_connection_t *c;
    xcb_screen_t     *screen;
    xcb_window_t      win;
    uint32_t          mask = 0;
    uint32_t          values[2];

    c = xcb_connect (NULL, NULL);

    screen = xcb_setup_roots_iterator (xcb_get_setup (c)).data;

    win = xcb_generate_id(c);
    
    mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    values[0] = screen->white_pixel;
    values[1] = XCB_EVENT_MASK_EXPOSURE       | XCB_EVENT_MASK_BUTTON_PRESS   |
                XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
                XCB_EVENT_MASK_KEY_PRESS      | XCB_EVENT_MASK_KEY_RELEASE;

    xcb_create_window(c,
                      XCB_COPY_FROM_PARENT,
                      win,
                      screen->root,
                      x, y,
                      width, height,
                      10,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      screen->root_visual,
                      mask, values);

    xcb_map_window (c, win);
    
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, win, XCB_ATOM_WM_NAME,
                      XCB_ATOM_STRING, 8, strlen(window_title), window_title);

    xcb_flush (c);
    
    WindowParams.Connection = c;
    WindowParams.Window = win;

    Created = true;
  }
  
  WindowFramework::~WindowFramework() {
    if( WindowParams.Window ) {
      xcb_destroy_window(WindowParams.Connection, WindowParams.Window);
    }

    if( WindowParams.Connection ) {
      xcb_disconnect(WindowParams.Connection);
    }
  }
  
  void WindowFramework::Render() {
    
    if( Created &&
        Sample.Initialize( WindowParams ) ) {
      
      xcb_generic_event_t *e;
      bool loop = true;
      
      while ( loop ) {
        if (e = xcb_poll_for_event(WindowParams.Connection)) {
          switch (e->response_type & ~0x80) {
            
            case XCB_BUTTON_PRESS:
            {
              xcb_button_press_event_t *ev = (xcb_button_press_event_t *)e;
              
              switch (ev->detail) {
                case 4:
                  Sample.MouseWheel( 0.5f );
                  break;
                case 5:
                  Sample.MouseWheel( - 0.5f );
                  break;
                default:
                  Sample.MouseClick( static_cast<int>(ev->detail), true );
              }
              break;
            }
            case XCB_BUTTON_RELEASE:
            {
              xcb_button_release_event_t *ev = (xcb_button_release_event_t *)e;
              
              Sample.MouseClick( static_cast<int>(ev->detail), false );
              break;
            }
            case XCB_MOTION_NOTIFY:
            {
              xcb_motion_notify_event_t *ev = (xcb_motion_notify_event_t *)e;
              
              Sample.MouseMove( static_cast<int>(ev->event_x), static_cast<int>(ev->event_y) );
              break;
            }
            case XCB_KEY_PRESS:
            {
              xcb_key_press_event_t *ev = (xcb_key_press_event_t *)e;
              
              if (ev->detail == 9) { // key = ESCAPE
                loop = false;
                xcb_change_property(WindowParams.Connection, XCB_PROP_MODE_REPLACE, WindowParams.Window,
                                    XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
                                    strlen("CLOSING..."), "CLOSING...");
              }
              break;
            }
            default:
              printf("Uncatched event: %d\n", e->response_type);
              break;
        }
        free (e);
    } else {
          if( Sample.IsReady() ) {
            Sample.UpdateTime();
            Sample.Draw();
            Sample.MouseReset();
          }
        }
    }

    Sample.Deinitialize();
    }
  }

#endif

} // namespace VulkanCookbook
