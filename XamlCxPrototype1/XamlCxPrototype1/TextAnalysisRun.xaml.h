//
// TextAnalysisRun.xaml.h
// Declaration of the TextAnalysisRun class
//

#pragma once

#include "TextAnalysisRun.g.h"

namespace XamlCxPrototype1
{

	public enum class CONTENT_TYPE : int32
	{
		SCRIPT_RUN,
			BIDI_LEVEL_RUN,
			MERGED_RUN
	};



	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class TextAnalysisRun sealed
	{
	public:
		TextAnalysisRun();


		property CONTENT_TYPE ContentType
		{
			CONTENT_TYPE get();
			void set(CONTENT_TYPE value);
		}


	private:
		CONTENT_TYPE m_contentType;
	};



}
