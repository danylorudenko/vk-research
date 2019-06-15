#include "Keyboard.hpp"
#include <Windows.h>

Keys VKeyToKeys(std::uint32_t vKey)
{
    switch (vKey)
    {
    case 0x30:
        return Keys::Num0;
    case 0x31:
        return Keys::Num1;
    case 0x32:
        return Keys::Num2;
    case 0x33:
        return Keys::Num3;
    case 0x34:
        return Keys::Num4;
    case 0x35:
        return Keys::Num5;
    case 0x36:
        return Keys::Num6;
    case 0x37:
        return Keys::Num7;
    case 0x38:
        return Keys::Num8;
    case 0x39:
        return Keys::Num9;

    case VK_NUMPAD0:
        return Keys::NumPad0;
    case VK_NUMPAD1:
        return Keys::NumPad1;
    case VK_NUMPAD2:
        return Keys::NumPad2;
    case VK_NUMPAD3:
        return Keys::NumPad3;
    case VK_NUMPAD4:
        return Keys::NumPad4;
    case VK_NUMPAD5:
        return Keys::NumPad5;
    case VK_NUMPAD6:
        return Keys::NumPad6;
    case VK_NUMPAD7:
        return Keys::NumPad7;
    case VK_NUMPAD8:
        return Keys::NumPad8;
    case VK_NUMPAD9:
        return Keys::NumPad9;


    case 0x41:
        return Keys::A;
    case 0x42:
        return Keys::B;
    case 0x43:
        return Keys::C;
    case 0x44:
        return Keys::D;
    case 0x45:
        return Keys::E;
    case 0x46:
        return Keys::F;
    case 0x47:
        return Keys::G;
    case 0x48:
        return Keys::H;
    case 0x49:
        return Keys::I;
    case 0x4A:
        return Keys::J;
    case 0x4B:
        return Keys::K;
    case 0x4C:
        return Keys::L;
    case 0x4D:
        return Keys::M;
    case 0x4E:
        return Keys::N;
    case 0x4F:
        return Keys::O;
    case 0x50:
        return Keys::P;
    case 0x51:
        return Keys::Q;
    case 0x52:
        return Keys::R;
    case 0x53:
        return Keys::S;
    case 0x54:
        return Keys::T;
    case 0x55:
        return Keys::U;
    case 0x56:
        return Keys::V;
    case 0x57:
        return Keys::W;
    case 0x58:
        return Keys::X;
    case 0x59:
        return Keys::Y;
    case 0x5A:
        return Keys::Z;
    case VK_SPACE:
        return Keys::Space;

    case VK_F1:
        return Keys::F1;
    case VK_F2:
        return Keys::F2;
    case VK_F3:
        return Keys::F3;
    case VK_F4:
        return Keys::F4;
    case VK_F5:
        return Keys::F5;
    case VK_F6:
        return Keys::F6;
    case VK_F7:
        return Keys::F7;
    case VK_F8:
        return Keys::F8;
    case VK_F9:
        return Keys::F9;
    case VK_F10:
        return Keys::F10;
    case VK_F11:
        return Keys::F11;
    case VK_F12:
        return Keys::F12;

    case VK_ESCAPE:
        return Keys::Escape;
    case VK_OEM_3:
        return Keys::Tilde;

    case VK_OEM_MINUS:
        return Keys::MinusUnderscore;
    case VK_OEM_PLUS:
        return Keys::PlusEquals;
    case VK_BACK:
        return Keys::Backspace;
    case VK_OEM_1:
        return Keys::Semicolon;
    case VK_TAB:
        return Keys::Tab;
    case VK_OEM_4:
        return Keys::BracketSquaredLeft;
    case VK_OEM_6:
        return Keys::BracketSquaredRight;
    case VK_RETURN:
        return Keys::Enter;
    case VK_CAPITAL:
        return Keys::CapsLock;
    case VK_OEM_2:
        return Keys::Slash;
    case VK_OEM_5:
        return Keys::Backslash;
    case VK_SHIFT:
        return Keys::Shift;
    case VK_CONTROL:
        return Keys::Ctrl;
    case VK_MENU:
        return Keys::Alt;
    case VK_LWIN:
        return Keys::WinLeft;
    case VK_RWIN:
        return Keys::WinRight;
    case VK_LEFT:
        return Keys::Left;
    case VK_RIGHT:
        return Keys::Right;
    case VK_UP:
        return Keys::Up;
    case VK_DOWN:
        return Keys::Down;
    case VK_INSERT:
        return Keys::Insert;
    case VK_HOME:
        return Keys::Home;
    case VK_END:
        return Keys::End;
    case VK_DELETE:
        return Keys::Delete;
    case VK_PRIOR:
        return Keys::PageUp;
    case VK_NEXT:
        return Keys::PageDown;
    default:
        return Keys::END;
    }
}

std::uint32_t KeysToVKey(Keys key)
{
    switch (key)
    {
    case Keys::Num0:
        return 0x30;
    case Keys::Num1:
        return 0x31;
    case Keys::Num2:
        return 0x32;
    case Keys::Num3:
        return 0x33;
    case Keys::Num4:
        return 0x34;
    case Keys::Num5:
        return 0x35;
    case Keys::Num6:
        return 0x36;
    case Keys::Num7:
        return 0x37;
    case Keys::Num8:
        return 0x38;
    case Keys::Num9:
        return 0x39;

    case Keys::NumPad0:
        return VK_NUMPAD0;
    case Keys::NumPad1:
        return VK_NUMPAD1;
    case Keys::NumPad2:
        return VK_NUMPAD2;
    case Keys::NumPad3:
        return VK_NUMPAD3;
    case Keys::NumPad4:
        return VK_NUMPAD4;
    case Keys::NumPad5:
        return VK_NUMPAD5;
    case Keys::NumPad6:
        return VK_NUMPAD6;
    case Keys::NumPad7:
        return VK_NUMPAD7;
    case Keys::NumPad8:
        return VK_NUMPAD8;
    case Keys::NumPad9:
        return VK_NUMPAD9;


    case Keys::A:
        return 0x41;
    case Keys::B:
        return 0x42;
    case Keys::C:
        return 0x43;
    case Keys::D:
        return 0x44;
    case Keys::E:
        return 0x45;
    case Keys::F:
        return 0x46;
    case Keys::G:
        return 0x47;
    case Keys::H:
        return 0x48;
    case Keys::I:
        return 0x49;
    case Keys::J:
        return 0x4A;
    case Keys::K:
        return 0x4B;
    case Keys::L:
        return 0x4C;
    case Keys::M:
        return 0x4D;
    case Keys::N:
        return 0x4E;
    case Keys::O:
        return 0x4F;
    case Keys::P:
        return 0x50;
    case Keys::Q:
        return 0x51;
    case Keys::R:
        return 0x52;
    case Keys::S:
        return 0x53;
    case Keys::T:
        return 0x54;
    case Keys::U:
        return 0x55;
    case Keys::V:
        return 0x56;
    case Keys::W:
        return 0x57;
    case Keys::X:
        return 0x58;
    case Keys::Y:
        return 0x59;
    case Keys::Z:
        return 0x5A;
    case Keys::Space:
        return VK_SPACE;

    case Keys::F1:
        return VK_F1;
    case Keys::F2:
        return VK_F2;
    case Keys::F3:
        return VK_F3;
    case Keys::F4:
        return VK_F4;
    case Keys::F5:
        return VK_F5;
    case Keys::F6:
        return VK_F6;
    case Keys::F7:
        return VK_F7;
    case Keys::F8:
        return VK_F8;
    case Keys::F9:
        return VK_F9;
    case Keys::F10:
        return VK_F10;
    case Keys::F11:
        return VK_F11;
    case Keys::F12:
        return VK_F12;

    case Keys::Escape:
        return VK_ESCAPE;
    case Keys::Tilde:
        return VK_OEM_3;

    case Keys::MinusUnderscore:
        return VK_OEM_MINUS;
    case Keys::PlusEquals:
        return VK_OEM_PLUS;
    case Keys::Backspace:
        return VK_BACK;
    case Keys::Semicolon:
        return VK_OEM_1;
    case Keys::Tab:
        return VK_TAB;
    case Keys::BracketSquaredLeft:
        return VK_OEM_4;
    case Keys::BracketSquaredRight:
        return VK_OEM_6;
    case Keys::Enter:
        return VK_RETURN;
    case Keys::CapsLock:
        return VK_CAPITAL;
    case Keys::Slash:
        return VK_OEM_2;
    case Keys::Backslash:
        return VK_OEM_5;
    case Keys::Shift:
        return VK_SHIFT;
    case Keys::Ctrl:
        return VK_CONTROL;
    case Keys::Alt:
        return VK_MENU;
    case Keys::WinLeft:
        return VK_LWIN;
    case Keys::WinRight:
        return VK_RWIN;
    case Keys::Left:
        return VK_LEFT;
    case Keys::Right:
        return VK_RIGHT;
    case Keys::Up:
        return VK_UP;
    case Keys::Down:
        return VK_DOWN;
    case Keys::Insert:
        return VK_INSERT;
    case Keys::Home:
        return VK_HOME;
    case Keys::End:
        return VK_END;
    case Keys::Delete:
        return VK_DELETE;
    case Keys::PageUp:
        return VK_PRIOR;
    case Keys::PageDown:
        return VK_NEXT;
    default:
        return 0;
    }
}
