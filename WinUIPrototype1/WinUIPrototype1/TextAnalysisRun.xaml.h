#pragma once

#include "TextAnalysisRun.g.h"

namespace winrt::WinUIPrototype1::implementation
{
    struct TextAnalysisRun : TextAnalysisRunT<TextAnalysisRun>
    {
        TextAnalysisRun()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void myButton_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::WinUIPrototype1::factory_implementation
{
    struct TextAnalysisRun : TextAnalysisRunT<TextAnalysisRun, implementation::TextAnalysisRun>
    {
    };
}
