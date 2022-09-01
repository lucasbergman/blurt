#pragma once

// Every day we stray further from God's light.
//
// The <windows.h> file (and indeed, most of the legacy Win32, C-compatible
// header files) define function-like macros min() and max() in the global
// namespace (!), which plays havoc with use of the standard C++ library in
// obvious ways. This will presumably never be removed because backward
// compatibility, but you can suppress it by defining NOMINMAX ahead of time.
#define NOMINMAX
#include <windows.h>

#include <hstring.h>
#include <restrictederrorinfo.h>
#include <unknwn.h>
#include <winrt/Windows.ApplicationModel.Activation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Data.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Windows.UI.Xaml.Markup.h>
#include <winrt/Windows.UI.Xaml.Navigation.h>
#include <winrt/Windows.UI.Xaml.h>
