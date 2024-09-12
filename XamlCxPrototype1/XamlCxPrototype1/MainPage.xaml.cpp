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
	this->LTR->IsChecked = true;

}

void XamlCxPrototype1::MainPage::BaseDirection_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	RadioButton^ radioButton = safe_cast<RadioButton^>(sender);
	this->baseDirectionChoice->Text = "You chose " + radioButton->Content->ToString();
}

void XamlCxPrototype1::MainPage::AnalyzeButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//this->resultsStackPanel->Children->Clear();

	TextAnalysisRun^ analysisRun = ref new TextAnalysisRun();
	analysisRun->ContentType = CONTENT_TYPE::MERGED_RUN;

	this->resultsStackPanel->Children->Append(analysisRun);
}

