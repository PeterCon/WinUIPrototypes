//
// TextAnalysisRun.xaml.cpp
// Implementation of the TextAnalysisRun class
//

#include "pch.h"
#include "TextAnalysisRun.xaml.h"

using namespace XamlCxPrototype1;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

TextAnalysisRun::TextAnalysisRun()
{
	InitializeComponent();
}

CONTENT_TYPE XamlCxPrototype1::TextAnalysisRun::ContentType::get()
{
	return m_contentType;
}

void XamlCxPrototype1::TextAnalysisRun::ContentType::set(CONTENT_TYPE value)
{
	m_contentType = value;

	switch (value)
	{
	case CONTENT_TYPE::SCRIPT_RUN:
		this->scriptRow->Height = GridLength(1, GridUnitType::Auto);
		this->bidiLevelRow->Height = GridLength(0);
		break;
	case CONTENT_TYPE::BIDI_LEVEL_RUN:
		this->scriptRow->Height = GridLength(0);
		this->bidiLevelRow->Height = GridLength(1, GridUnitType::Auto);
		break;
	case CONTENT_TYPE::MERGED_RUN:
		this->scriptRow->Height = GridLength(1, GridUnitType::Auto);
		this->bidiLevelRow->Height = GridLength(1, GridUnitType::Auto);
		break;
	}
}

Platform::String^ XamlCxPrototype1::TextAnalysisRun::Text::get()
{
	String^ str = ref new String(m_text.c_str());
	return  str;
 }

void XamlCxPrototype1::TextAnalysisRun::Text::set(Platform::String^ value)
{
	m_text = value->Data();
	this->runText->Text = value;
}

int32 XamlCxPrototype1::TextAnalysisRun::StartPosition::get()
{
	return m_startPosition;
}

void XamlCxPrototype1::TextAnalysisRun::StartPosition::set(int32 value)
{
	m_startPosition = value;
	this->runStartPosition->Text = value.ToString();
}

int32 XamlCxPrototype1::TextAnalysisRun::Length::get()
{
	return m_length;
}

void XamlCxPrototype1::TextAnalysisRun::Length::set(int32 value)
{
	m_length = value;
	this->runLength->Text = value.ToString();
}

Platform::String^ XamlCxPrototype1::TextAnalysisRun::Script::get()
{
	String^ str = ref new String(m_script.c_str());
	return str;
}

void XamlCxPrototype1::TextAnalysisRun::Script::set(Platform::String^ value)
{
	m_script = value->Data();
	this->runScript->Text = value;
}

int32 XamlCxPrototype1::TextAnalysisRun::BidiLevel::get()
{
	return m_bidiLevel;
}

void XamlCxPrototype1::TextAnalysisRun::BidiLevel::set(int32 value)
{
	m_bidiLevel = value;
	this->runBidiLevel->Text = value.ToString();
}

