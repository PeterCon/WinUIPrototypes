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
	/// A structure to hold horizontal metrics for a run tile.
	/// Used in arrangement of run tiles on a canvas.
	/// </summary>
	struct RunTileHorizontalMetrics
	{
	public:
		int TextStartPosition;
		double HorizontalPosition;
		double Width;

		RunTileHorizontalMetrics() = default;
	};




	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
		const double TILE_HORIZONTAL_GAP = 5.0;
		const double TILE_VERTICAL_GAP = 10.0;

	public:
		MainPage();

	private:
		void HamburgerButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void BaseDirection_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void AnalyzeButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		// the following was used for earlier prototype using stackpanels for displaying results
		//void UpdateRunResults(Platform::Object^ panel, std::vector<TextAnalysis::Run>& runs, CONTENT_TYPE type);


		Platform::Collections::Vector<TextAnalysisRun^>^ CreateTextAnalysisRunTiles(std::vector<TextAnalysis::Run> runsData, CONTENT_TYPE type);

		std::vector<RunTileHorizontalMetrics> GetRunTilePositions(Platform::Collections::Vector<TextAnalysisRun^>^ runTiles, double gap);

		double ArrangeRunTileHorizontalMetrics(std::vector<RunTileHorizontalMetrics>& scriptRunTileHorizontalMetrics, std::vector<RunTileHorizontalMetrics>& bidiRunTileHorizontalMetrics, std::vector<RunTileHorizontalMetrics>& mergedRunTileHorizontalMetrics);
		bool AdjustHorizontalPositionsForTwoRuns(std::vector<RunTileHorizontalMetrics>& runTileHorizontalMetrics1, std::vector<RunTileHorizontalMetrics>& runTileHorizontalMetrics2);
		int GetRunIndex(std::vector<RunTileHorizontalMetrics>& runTileHorizontalMetrics, int textStartPosition);
		void UpdateRunTileHorizontalPositionAtIndex(std::vector<RunTileHorizontalMetrics>& runTileHorizontalMetrics, int index, double newHorizontalPosition);
		void UpdateRunTileWidths(std::vector<RunTileHorizontalMetrics>& runTileHorizontalMetrics, double gap, double rightEdgePosition);

		void PlaceRunTilesOnCanvas(Platform::Collections::Vector<TextAnalysisRun^>^ runTiles, std::vector<RunTileHorizontalMetrics>& runTileHorizontalMetrics, double verticalPosition);


		TextAnalysis m_textAnalysis;
	};

}
