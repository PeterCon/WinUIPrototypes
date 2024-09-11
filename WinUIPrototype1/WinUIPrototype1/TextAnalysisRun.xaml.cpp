#include "pch.h"
#include "TextAnalysisRun.xaml.h"
#if __has_include("TextAnalysisRun.g.cpp")
#include "TextAnalysisRun.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::WinUIPrototype1::implementation
{
    int32_t TextAnalysisRun::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void TextAnalysisRun::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

}
