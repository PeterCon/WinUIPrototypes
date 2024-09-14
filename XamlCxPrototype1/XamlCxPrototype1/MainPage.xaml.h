//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include "TextAnalysis.h"
#include "TextAnalysisRun.xaml.h"

namespace XamlCxPrototype1
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

	private:
		void HamburgerButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void BaseDirection_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void AnalyzeButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void UpdateRunResults(Platform::Object^ panel, std::vector<TextAnalysis::Run>& runs, CONTENT_TYPE type);

		TextAnalysis m_textAnalysis;
	};

}
