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


// Calculate and present results of text analysis:

void XamlCxPrototype1::MainPage::AnalyzeButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	m_textAnalysis.SetText(this->TextToAnalyze->Text->ToString());

	std::vector<TextAnalysis::Run> scriptRuns;
	std::vector<TextAnalysis::Run> bidiLevelRuns;
	std::vector<TextAnalysis::Run> mergedRuns;

	THROW_IF_FAILED( m_textAnalysis.GenerateAnalysis(scriptRuns, bidiLevelRuns, mergedRuns));


	// Present results on a canvas

	// Get the run tiles (these will already have been measured)
	Platform::Collections::Vector<TextAnalysisRun^>^ scriptRunTiles = CreateTextAnalysisRunTiles(scriptRuns, CONTENT_TYPE::SCRIPT_RUN);
	Platform::Collections::Vector<TextAnalysisRun^>^ bidiRunTiles = CreateTextAnalysisRunTiles(bidiLevelRuns, CONTENT_TYPE::BIDI_LEVEL_RUN);
	Platform::Collections::Vector<TextAnalysisRun^>^ mergedRunTiles = CreateTextAnalysisRunTiles(mergedRuns, CONTENT_TYPE::MERGED_RUN);

	// Get the default run tile positions
	std::vector<RunTileHorizontalMetrics> scriptRunTilePositions = GetRunTilePositions(scriptRunTiles, TILE_HORIZONTAL_GAP);
	std::vector<RunTileHorizontalMetrics> bidiRunTilePositions = GetRunTilePositions(bidiRunTiles, TILE_HORIZONTAL_GAP);
	std::vector<RunTileHorizontalMetrics> mergedRunTilePositions = GetRunTilePositions(mergedRunTiles, TILE_HORIZONTAL_GAP);

	// Arrange the run tile horizontal metrics
	ResultsCanvas->Width = ArrangeRunTileHorizontalMetrics(scriptRunTilePositions, bidiRunTilePositions, mergedRunTilePositions);


	// Get vertical positions for the run tiles
	double scriptTileVerticalPosition = 0;
	double bidiTileVerticalPosition = scriptRunTiles->GetAt(0)->DesiredSize.Height + TILE_VERTICAL_GAP;
	double mergedTileVerticalPosition = bidiTileVerticalPosition + bidiRunTiles->GetAt(0)->DesiredSize.Height + TILE_VERTICAL_GAP;
	ResultsCanvas->Height = mergedTileVerticalPosition + mergedRunTiles->GetAt(0)->DesiredSize.Height + 6;


	// Place the run tiles on the canvas
	ResultsCanvas->Children->Clear();
	PlaceRunTilesOnCanvas(scriptRunTiles, scriptRunTilePositions, scriptTileVerticalPosition);
	PlaceRunTilesOnCanvas(bidiRunTiles, bidiRunTilePositions, bidiTileVerticalPosition);
	PlaceRunTilesOnCanvas(mergedRunTiles, mergedRunTilePositions, mergedTileVerticalPosition);

	// for earlier prototype using stackpanels for displaying results
	//UpdateRunResults(this->ScriptResultsStackPanel, scriptRuns, CONTENT_TYPE::SCRIPT_RUN);
	//UpdateRunResults(this->BidiResultsStackPanel, bidiLevelRuns, CONTENT_TYPE::BIDI_LEVEL_RUN);
	//UpdateRunResults(this->MergedResultsStackPanel, mergedRuns, CONTENT_TYPE::MERGED_RUN);



	this->ResultsScrollViewer->Focus(Windows::UI::Xaml::FocusState::Programmatic);

} // AnalyzeButton_Click




//////////////////////////////////////////////////////////////////////////////////
///////// Helper functions for laying out run tiles on a canvas  ////////////////


// for earlier prototype using stackpanels for displaying results
//void XamlCxPrototype1::MainPage::UpdateRunResults(Platform::Object^ panel, std::vector<TextAnalysis::Run>& runs, CONTENT_TYPE type)
//{
//	StackPanel^ runResultsPanel = safe_cast<StackPanel^>(panel);
//	runResultsPanel->Children->Clear();
//	for (auto run : runs)
//	{
//		TextAnalysisRun^ analysisRun = ref new TextAnalysisRun();
//		analysisRun->ContentType = type;
//		analysisRun->Text = m_textAnalysis.GetTextRun(run.textStart, run.textLength);
//		analysisRun->StartPosition = run.textStart;
//		analysisRun->Length = run.textLength;
//
//		if (type == CONTENT_TYPE::SCRIPT_RUN || type == CONTENT_TYPE::MERGED_RUN)
//		{
//			analysisRun->Script = run.ScriptTag();
//		}
//		
//		if (type == CONTENT_TYPE::BIDI_LEVEL_RUN || type == CONTENT_TYPE::MERGED_RUN)
//		{
//			analysisRun->BidiLevel = run.bidiLevel;
//		}
//
//		runResultsPanel->Children->Append(analysisRun);
//	}
//} // UpdateRunResults



Platform::Collections::Vector<TextAnalysisRun^>^ XamlCxPrototype1::MainPage::CreateTextAnalysisRunTiles(std::vector<TextAnalysis::Run> runsData, CONTENT_TYPE type)
{
	Platform::Collections::Vector<TextAnalysisRun^>^ runTiles = ref new Platform::Collections::Vector<TextAnalysisRun^>();

	for (auto run : runsData)
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
		analysisRun->Measure(Size(INFINITE, INFINITE));

		runTiles->Append(analysisRun);

	}
	return runTiles;
} // CreateTextAnalysisRunTiles


std::vector<RunTileHorizontalMetrics> XamlCxPrototype1::MainPage::GetRunTilePositions(Platform::Collections::Vector<TextAnalysisRun^>^ runTiles, double gap)
{
	std::vector<RunTileHorizontalMetrics> runTilePositions;

	for (int i = 0; i < runTiles->Size; i++)
	{
		RunTileHorizontalMetrics runTilePosition;
		runTilePosition.TextStartPosition = runTiles->GetAt(i)->StartPosition;
//		runTilePosition.VerticalPosition = 0;
		runTilePosition.Width = runTiles->GetAt(i)->DesiredSize.Width;

		if (i == 0)
		{
			runTilePosition.HorizontalPosition = 0;
		}
		else
		{
			double previousTilePosition = runTilePositions[i - 1].HorizontalPosition;
			double previousTileWidth = runTiles->GetAt(i - 1)->DesiredSize.Width;

			runTilePosition.HorizontalPosition = previousTilePosition + previousTileWidth + gap;
		}
		runTilePositions.push_back(runTilePosition);
	}

	return runTilePositions;
} // GetRunTilePositions


double XamlCxPrototype1::MainPage::ArrangeRunTileHorizontalMetrics(std::vector<RunTileHorizontalMetrics>& scriptRunTilePositions, std::vector<RunTileHorizontalMetrics>& bidiRunTilePositions, std::vector<RunTileHorizontalMetrics>& mergedRunTilePositions)
{
	// Arrange the run tile horizontal metrics so that the horizontal positions of tiles
	// align for the same text start position. Returns the right edge position of the last tile.
	
	// For each run in the script vector, there will be a run in the merged vector 
	// with the same text start position. Similarly, for each run in the bidi vector, 
	// there will be a run in the merged vector with the same text start position.
	// 
	// For each script run tile, we can compare with the merged run tile for the 
	// same text start position and update the horizontal positions to the greater
	// of the two positions.


	// Compare the script run tile positions with the merged run tile positions, then
	// compare the bidi run tile positions with the merged run tile positions. If any
	// adjustments are made, repeat the process until no further adjustments are made.

	bool adjustmentsMade = true;
	while (adjustmentsMade)
	{
		adjustmentsMade = AdjustHorizontalPositionsForTwoRuns(scriptRunTilePositions, mergedRunTilePositions);
		adjustmentsMade = AdjustHorizontalPositionsForTwoRuns(bidiRunTilePositions, mergedRunTilePositions) || adjustmentsMade;
	}


	// Got final positions. Now update the widths of the run tiles to fill the gaps between them.
	double rightEdge = max(scriptRunTilePositions.back().HorizontalPosition + scriptRunTilePositions.back().Width,
			max(bidiRunTilePositions.back().HorizontalPosition + bidiRunTilePositions.back().Width,
			mergedRunTilePositions.back().HorizontalPosition + mergedRunTilePositions.back().Width));

	UpdateRunTileWidths(scriptRunTilePositions, TILE_HORIZONTAL_GAP, rightEdge);
	UpdateRunTileWidths(bidiRunTilePositions, TILE_HORIZONTAL_GAP, rightEdge);
	UpdateRunTileWidths(mergedRunTilePositions, TILE_HORIZONTAL_GAP, rightEdge);

	return rightEdge;
}

bool XamlCxPrototype1::MainPage::AdjustHorizontalPositionsForTwoRuns(std::vector<RunTileHorizontalMetrics>& runTilePositions1, std::vector<RunTileHorizontalMetrics>& runTilePositions2)
{
	// Returns true if adjustments were made to positions in either vector.
	bool adjustmentsMade = false;

	// For each run tile in the first vector, look for a corresponding run tile in the second vector.
	for (int i = 0; i < runTilePositions1.size(); i++)
	{
		int run2Index = GetRunIndex(runTilePositions2, runTilePositions1[i].TextStartPosition);
		if (run2Index != -1)
		{
			double horizontalPosition1 = runTilePositions1[i].HorizontalPosition;
			double horizontalPosition2 = runTilePositions2[run2Index].HorizontalPosition;

			if (horizontalPosition1 != horizontalPosition2)
			{
				double newHorizontalPosition = max(horizontalPosition1, horizontalPosition2);
				UpdateRunTileHorizontalPositionAtIndex(runTilePositions1, i, newHorizontalPosition);
				UpdateRunTileHorizontalPositionAtIndex(runTilePositions2, run2Index, newHorizontalPosition);
				adjustmentsMade = true;
			}
		}
	}

	return adjustmentsMade;
}



int XamlCxPrototype1::MainPage::GetRunIndex(std::vector<RunTileHorizontalMetrics>& runTilePositions, int textStartPosition)
{
	for (int i = 0; i < runTilePositions.size(); i++)
	{
		if (runTilePositions[i].TextStartPosition == textStartPosition)
		{
			return i;
		}
		else if (runTilePositions[i].TextStartPosition > textStartPosition)
		{
			return -1;
		}
	}
	return -1;
}


void XamlCxPrototype1::MainPage::UpdateRunTileHorizontalPositionAtIndex(std::vector<RunTileHorizontalMetrics>& runTilePositions, int index, double newHorizontalPosition)
{
	// Update the horizontal position of the run tile at the specified index. All of the following
	// run tiles will need to be adjusted by the same amount.
	if (index < 0 || index >= runTilePositions.size())
	{
		throw ref new Platform::InvalidArgumentException("Index out of range");
	}

	if (runTilePositions[index].HorizontalPosition != newHorizontalPosition)
	{
		double horizontalAdjustment = newHorizontalPosition - runTilePositions[index].HorizontalPosition;
		for (int i = index; i < runTilePositions.size(); i++)
		{
			runTilePositions[i].HorizontalPosition += horizontalAdjustment;
		}
	}
}


void XamlCxPrototype1::MainPage::UpdateRunTileWidths(std::vector<RunTileHorizontalMetrics>& runTileHorizontalMetrics, double gap, double rightEdgePosition)
{
	// Update the width of each run tile in the vector to fill the gap to the next run tile.
	// The last run tile will be adjusted to fill the remaining space on the canvas.

	for (int i = 0; i < runTileHorizontalMetrics.size(); i++)
	{
		if (i == runTileHorizontalMetrics.size() - 1)
		{
			runTileHorizontalMetrics[i].Width = rightEdgePosition - runTileHorizontalMetrics[i].HorizontalPosition;
		}
		else
		{
			runTileHorizontalMetrics[i].Width = runTileHorizontalMetrics[i + 1].HorizontalPosition - runTileHorizontalMetrics[i].HorizontalPosition - gap;
		}
	}

}


void XamlCxPrototype1::MainPage::PlaceRunTilesOnCanvas(Platform::Collections::Vector<TextAnalysisRun^>^ runTiles, std::vector<RunTileHorizontalMetrics>& runTileHorizontalMetrics, double verticalPosition)
{
	// Place the run tiles on the canvas at the specified vertical position.

	// The two vectors should have the same number of elements.
	if (runTiles->Size != runTileHorizontalMetrics.size())
	{
		throw ref new Platform::InvalidArgumentException("Mismatched number of run tiles and horizontal metrics");
	}

	for (int i = 0; i < runTiles->Size; i++)
	{
		runTiles->GetAt(i)->Width = runTileHorizontalMetrics[i].Width;
		runTiles->GetAt(i)->SetValue(Canvas::LeftProperty, runTileHorizontalMetrics[i].HorizontalPosition);
		runTiles->GetAt(i)->SetValue(Canvas::TopProperty, verticalPosition);
		ResultsCanvas->Children->Append(runTiles->GetAt(i));
	}
}


// End of helper functions for laying out run tiles on a canvas
