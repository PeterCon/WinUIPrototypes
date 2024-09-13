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

		property Platform::String^ Text
		{
			Platform::String^ get();
			void set(Platform::String^ value);
		}

		property int32 StartPosition
		{
			int32 get();
			void set(int32 value);
		}

		property int32 Length
		{
			int32 get();
			void set(int32 value);
		}

		property Platform::String^ Script
		{
			Platform::String^ get();
			void set(Platform::String^ value);
		}

		property int32 BidiLevel
		{
			int32 get();
			void set(int32 value);
		}


	private:
		CONTENT_TYPE m_contentType;
		std::wstring m_text;
		int32 m_startPosition;
		int32 m_length;
		std::wstring m_script;
		int32 m_bidiLevel;

	};



}
