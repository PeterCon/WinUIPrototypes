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

    public: enum CONTENT_TYPE
        {
            SCRIPT_RUN,
            BIDI_LEVEL_RUN,
			MERGED_RUN
        };

		int32_t ContentType();
		void ContentType(int32_t contentType);

        int32_t MyProperty();
        void MyProperty(int32_t value);

	private:
		//CONTENT_TYPE m_contentType = CONTENT_TYPE::MERGED_RUN;
        int32_t m_contentType = 0;
    };
}

namespace winrt::WinUIPrototype1::factory_implementation
{
    struct TextAnalysisRun : TextAnalysisRunT<TextAnalysisRun, implementation::TextAnalysisRun>
    {
    };
}
