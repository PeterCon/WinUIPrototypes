#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#include "TextAnalysisRun.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::WinUIPrototype1::implementation
{
    int32_t MainWindow::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void MainWindow::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void MainWindow::textBaseDirection_Changed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& args)
    {

    }

    void MainWindow::analyzeButton_Click(IInspectable const&, RoutedEventArgs const&)  
    {  
        // Try adding a TextAnalysisRun to the resultsStackPanel. Create the object, then set its ContentType property to SCRIPT_RUN.

        auto analysisRun = winrt::WinUIPrototype1::TextAnalysisRun();  

		// This doesn't work: the ContentType property is not found!!
		analysisRun.ContentType(CONTENT_TYPE::SCRIPT_RUN);

        // making sure I can add a stock control first; replace with the custom control 
        auto textBlock = Microsoft::UI::Xaml::Controls::TextBlock();  
        textBlock.Text(L"Analysis complete.");  
        this->resultsStackPanel().Children().Append(textBlock);  
    }
}
