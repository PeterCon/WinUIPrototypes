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
	int32_t	TextAnalysisRun::ContentType()
	{
		return m_contentType;
	}
	void TextAnalysisRun::ContentType(int32_t contentType)
	{
		switch (contentType)
		{
		case SCRIPT_RUN:
			this->scriptRow().Height(GridLength{ 1.0, GridUnitType::Auto });
			this->bidiLevelRow().Height(GridLength{ 0.0, GridUnitType::Pixel });
			break;
		case BIDI_LEVEL_RUN:
			this->scriptRow().Height(GridLength{ 0.0, GridUnitType::Pixel });
			this->bidiLevelRow().Height(GridLength{ 1.0, GridUnitType::Auto });
			break;
		case MERGED_RUN:
			this->scriptRow().Height(GridLength{ 1.0, GridUnitType::Auto });
			this->bidiLevelRow().Height(GridLength{ 1.0, GridUnitType::Auto });
			break;
		}
	m_contentType = contentType;
	}

	int32_t TextAnalysisRun::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void TextAnalysisRun::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }
}
