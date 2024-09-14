//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include <TextAnalysisRun.xaml.h>

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

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

MainPage::MainPage()
{
	InitializeComponent();

	m_textAnalysis = TextAnalysis();
	m_textAnalysis.SetText(this->TextToAnalyze->Text->ToString());

	this->LTR->IsChecked = true;
}

void XamlCxPrototype1::MainPage::HamburgerButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	this->Splitter->IsPaneOpen = !this->Splitter->IsPaneOpen;
}

void XamlCxPrototype1::MainPage::BaseDirection_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	RadioButton^ radioButton = safe_cast<RadioButton^>(sender);
	this->baseDirectionChoice->Text = "You chose " + radioButton->Content->ToString();
	if (radioButton->Name == "LTR")
	{
		m_textAnalysis.SetReadingDirection(DWRITE_READING_DIRECTION_LEFT_TO_RIGHT);
	}
	else // radioButton->Name == "RTL"
	{
		m_textAnalysis.SetReadingDirection(DWRITE_READING_DIRECTION_RIGHT_TO_LEFT);
	}
}

void XamlCxPrototype1::MainPage::AnalyzeButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	m_textAnalysis.SetText(this->TextToAnalyze->Text->ToString());

	std::vector<TextAnalysis::Run> scriptRuns;
	std::vector<TextAnalysis::Run> bidiLevelRuns;
	std::vector<TextAnalysis::Run> mergedRuns;

	THROW_IF_FAILED( m_textAnalysis.GenerateAnalysis(scriptRuns, bidiLevelRuns, mergedRuns));

	UpdateRunResults(this->ScriptResultsStackPanel, scriptRuns, CONTENT_TYPE::SCRIPT_RUN);
	UpdateRunResults(this->BidiResultsStackPanel, bidiLevelRuns, CONTENT_TYPE::BIDI_LEVEL_RUN);
	UpdateRunResults(this->MergedResultsStackPanel, mergedRuns, CONTENT_TYPE::MERGED_RUN);


	this->ResultsScrollViewer->Focus(Windows::UI::Xaml::FocusState::Programmatic);
}

void XamlCxPrototype1::MainPage::UpdateRunResults(Platform::Object^ panel, std::vector<TextAnalysis::Run>& runs, CONTENT_TYPE type)
{
	StackPanel^ runResultsPanel = safe_cast<StackPanel^>(panel);
	runResultsPanel->Children->Clear();
	for (auto run : runs)
	{
		TextAnalysisRun^ analysisRun = ref new TextAnalysisRun();
		analysisRun->ContentType = type;
		analysisRun->Text = m_textAnalysis.GetTextRun(run.textStart, run.textLength);
		analysisRun->StartPosition = run.textStart;
		analysisRun->Length = run.textLength;

		if (type == CONTENT_TYPE::SCRIPT_RUN || type == CONTENT_TYPE::MERGED_RUN)
		{
			analysisRun->Script = run.ScriptTag();
		}
		
		if (type == CONTENT_TYPE::BIDI_LEVEL_RUN || type == CONTENT_TYPE::MERGED_RUN)
		{
			analysisRun->BidiLevel = run.bidiLevel;
		}

		runResultsPanel->Children->Append(analysisRun);
	}
}

