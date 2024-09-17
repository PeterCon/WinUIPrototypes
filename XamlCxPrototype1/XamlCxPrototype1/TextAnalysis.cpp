#include "pch.h"
#include "TextAnalysis.h"





TextAnalysis::TextAnalysis()
{
	// Create a DirectWrite factory.
	THROW_IF_FAILED(DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory2),
		reinterpret_cast<IUnknown**>(&m_dwriteFactory)
	));

	// Create a text analyzer.
	wil::com_ptr<IDWriteTextAnalyzer> textAnalyzer;
	THROW_IF_FAILED(m_dwriteFactory->CreateTextAnalyzer(&textAnalyzer));
	THROW_IF_FAILED(textAnalyzer->QueryInterface<IDWriteTextAnalyzer1>(&m_textAnalyzer));

	// IDWriteTextAnalysisSource requires the implementation to provide a number substitution
	// for any position in the text. We'll always return None.
	THROW_IF_FAILED(m_dwriteFactory->CreateNumberSubstitution(
		DWRITE_NUMBER_SUBSTITUTION_METHOD_NONE,
		/* localeName */ L"en-us",
		/* ignoreUserOverride */ true,
		&m_numberSubstitution
	));
}

void TextAnalysis::SetText(Platform::String^ text)
{
	m_text = text->Data();
	m_textLength = static_cast<UINT32>(m_text.length());
}

void TextAnalysis::SetReadingDirection(DWRITE_READING_DIRECTION readingDirection)
{
	m_readingDirection = readingDirection;
}


HRESULT __stdcall TextAnalysis::GenerateAnalysis(OUT std::vector<Run>& scriptRuns, OUT std::vector<Run>& bidiLevelRuns, OUT std::vector<Run>& mergedRuns)
{
	// Analyze the text and generate runs for script, bidi level, and merged script+bidi properties.

	/* 
		Analysis of script and bidi will segment the text into runs. AnalyzeScript will 
		result in separate callbacks for each script run. Similarly, AnalyzeBidi will 
		result in separate callbacks for each bidi level run. Script and bidi runs 
		can be combined into a single vector of runs; for example, when laying out text,
		each call to GetGlyphs should be for a run with the same script and same bidi 
		level.

		The calls to the sink will usually progress forward through the text. A vector 
		of runs is compiled, with each segmentation adding a new run to the end of the 
		vector. One run in the vector spans to the end of the text; usually that is the 
		last run in the vector, and segmentation usually results in splitting that run 
		into two. However, the calls will not always result in a run being split and a 
		new run being added. For example if script analysis is processed and then bidi
		analysis is subsequently processed, the bidi analysis could only need to set
		the bidi levels on the script runs without splitting any runs.
 
		Also, a run could need to be split that is not the last run in the vector. For 
		example, suppose the text begins with a combination of Arabic letters and 
		Arabic-Indic digits followed by Latin text. Script analysis would segment
		the Arabic letters and Arabic-Indic digits into a single script run. If bidi 
		analysis is subsequently processed over the script runs, that initial script
		run would be split into two (or more) bidi-level runs. Note that new runs
		are always addeded to the end of the vector. Thus, the resulting runs might
		not be in the logical order of the text.
 
		When a run is split, the current Set* call will be setting data for the front
		half of the post-split pair of runs, and the data for the back half will be set 
		in a subsequent Set* call.
 
		In addition, a Set* call might need to set data for multiple runs. For example,
		if script analysis has segmented the text into multiple script runs for LTR 
		scripts, and then AnalyzeBidi is called, a single SetBidiLevel call will be
		made to set the same bidi level on all of the script runs.
 
		Thus, the SetScriptAnalysis and SetBidiLevel calls will need to:

			 1. Determine the run that contains the target text position.
			 2. Split that run if necessary.
			 3. Determine the target run: the current run, the first half of a run
				that has just been split, or the next run.
			 4. Set the relevant data for the target run.

		In the current implementation, SetScriptAnalysis and SetBidiLevel call
		SplitCurrentRun right after calling SetCurrentRun, but that might not 
		be the point at which splits actually take place. Rather, there's a subsequent 
		call to FetchNextRun: that will check whether there needs to be a split and, 
		if so, it will call SplitCurrentRun. 
		
		Also, the FetchNextRun call (to get the target run) is in a loop. That allows
		for the case that a single Set* call needs to set data for multiple runs.
	*/


	// Seed initial runs for each analysis type with the entire text.
	{
		m_scriptRuns.resize(1);
		LinkedRun& initialRun = m_scriptRuns[0];
		initialRun.textStart = 0;
		initialRun.textLength = m_textLength;
		initialRun.script = DWRITE_SCRIPT_ANALYSIS{};
		initialRun.scriptProperties = DWRITE_SCRIPT_PROPERTIES{};
		initialRun.bidiLevel = 0;
		initialRun.nextRunIndex = 0;
		m_scriptCurrentRunIndex = 0;
	}
	{
		m_bidiRuns.resize(1);
		LinkedRun& initialRun = m_bidiRuns[0];
		initialRun.textStart = 0;
		initialRun.textLength = m_textLength;
		initialRun.script = DWRITE_SCRIPT_ANALYSIS{};
		initialRun.scriptProperties = DWRITE_SCRIPT_PROPERTIES{};
		initialRun.bidiLevel = (m_readingDirection == DWRITE_READING_DIRECTION_RIGHT_TO_LEFT) ? 1 : 0;
		initialRun.nextRunIndex = 0;
		m_bidiCurrentRunIndex = 0;
	}
	{
		m_mergedRuns.resize(1);
		LinkedRun& initialRun = m_mergedRuns[0];
		initialRun.textStart = 0;
		initialRun.textLength = m_textLength;
		initialRun.script = DWRITE_SCRIPT_ANALYSIS{};
		initialRun.scriptProperties = DWRITE_SCRIPT_PROPERTIES{};
		initialRun.bidiLevel = (m_readingDirection == DWRITE_READING_DIRECTION_RIGHT_TO_LEFT) ? 1 : 0;
		initialRun.nextRunIndex = 0;
		m_mergedCurrentRunIndex = 0;
	}

	// We'll call IDWriteTextAnalyzer methods AnalyzeScript and AnalyzeBidi once each. Each will
	// call the corresponding IDWriteTextAnalysisSink Set* methods one or more times. In the script
	// callbacks, the script and merge run vectors will be updated. Similarly, in the bidi 
	// callbacks, the bidi and merge run vectors will be updated. Thus, the merged run vector will
	// get updated twice, once for each analysis type.

	THROW_IF_FAILED(m_textAnalyzer->AnalyzeScript(this, 0, m_textLength, this));
	THROW_IF_FAILED(m_textAnalyzer->AnalyzeBidi(this, 0, m_textLength, this));


	// Set the script properties on the script and merged runs.
	THROW_IF_FAILED(SetScriptProperties(m_scriptRuns));
	THROW_IF_FAILED(SetScriptProperties(m_mergedRuns));

	// Reorder the runs so that they are in the logical order of the text.
	ReorderRuns(m_scriptRuns, scriptRuns);
	ReorderRuns(m_bidiRuns, bidiLevelRuns);
	ReorderRuns(m_mergedRuns, mergedRuns);


	return S_OK;
}


Platform::String^ TextAnalysis::GetTextRun(UINT32 textPosition, UINT32 textLength) const
{
	// Return the substring of the text that corresponds to the given position and length.
	std::wstring wText(m_text);
	std::wstring wRunText = wText.substr(textPosition, textLength);
	return ref new Platform::String(wRunText.c_str());
}




////////////////////////////////////////////////////////////////////////////////
// Run methods


Platform::String^ TextAnalysis::Run::TagToString(UINT32 tag)
{
	// The tag contains up to 4 ASCII characters with the first
	// in the low-order byte. Shift and mask to get each character,
	// cast to WCHAR, and append a null terminator.
	WCHAR tagChars[] = {
		static_cast<WCHAR>(tag & 0xFF),
		static_cast<WCHAR>((tag & 0xFF00) >> 8),
		static_cast<WCHAR>((tag & 0xFF0000) >> 16),
		static_cast<WCHAR>((tag & 0xFF000000) >> 24),
		L'\0'
	};
	return ref new Platform::String(tagChars);
}

Platform::String^ TextAnalysis::Run::ScriptTag()
{
	return TagToString(this->scriptProperties.isoScriptCode);
}






////////////////////////////////////////////////////////////////////////////////
// IDWriteTextAnalysisSource implementation

HRESULT STDMETHODCALLTYPE TextAnalysis::GetTextAtPosition(
	UINT32 textPosition,
	_Outptr_result_buffer_(*textLength) WCHAR const** textString,
	_Out_ UINT32* textLength
)
{
	if (textPosition >= m_text.length())
	{
		*textString = L"";
		*textLength = 0;
	}
	*textString = m_text.c_str() + textPosition;
	*textLength = static_cast<UINT32>(m_text.length() - textPosition);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE TextAnalysis::GetTextBeforePosition(
	UINT32 textPosition,
	_Outptr_result_buffer_(*textLength) WCHAR const** textString,
	_Out_ UINT32* textLength
)
{
	*textString = m_text.substr(0, textPosition).c_str();
	*textLength = textPosition;
	return S_OK;
}

DWRITE_READING_DIRECTION STDMETHODCALLTYPE TextAnalysis::GetParagraphReadingDirection()
{
	return m_readingDirection;
}

HRESULT STDMETHODCALLTYPE TextAnalysis::GetLocaleName(
	UINT32 textPosition,
	_Out_ UINT32* textLength,
	_Outptr_result_z_ WCHAR const** localeName
)
{
	// IDWriteTextAnalyzer can ask for a locale name at any text position, but for
	// itemization purposes this has no effect. (It can matter for shaping.) 
	// OK to just return "en-us" for the remainder of the text.
	*textLength = m_text.length() - textPosition;
	*localeName = L"en-us";
	return S_OK;
}

HRESULT STDMETHODCALLTYPE TextAnalysis::GetNumberSubstitution(
	UINT32 textPosition,
	_Out_ UINT32* textLength,
	_COM_Outptr_ IDWriteNumberSubstitution** numberSubstitution
)
{
	*textLength = m_text.length() - textPosition;
	*numberSubstitution = m_numberSubstitution.get();
	(*numberSubstitution)->AddRef();
	return S_OK;
}



////////////////////////////////////////////////////////////////////////////////
// IDWriteTextAnalysisSink implementation

HRESULT STDMETHODCALLTYPE TextAnalysis::SetScriptAnalysis(UINT32 textPosition, UINT32 textLength, DWRITE_SCRIPT_ANALYSIS const* scriptAnalysis) throw()
{
	// Will operate on the script and merged runs.

	try // First the script runs.
	{
		// Set the current run and split if necessary.
		SetCurrentRun(textPosition, RunType::Script);
		SplitCurrentRun(textPosition, RunType::Script);
		UINT32 textLength1 = textLength;
		while (textLength1 > 0)
		{
			LinkedRun& run = FetchNextRun(&textLength1, &m_scriptCurrentRunIndex, &m_scriptRuns);
			run.script = *scriptAnalysis;
		}
	}
	catch (...)
	{
		return E_FAIL;
	}

	try // Now the merged runs.
	{
		// Set the current run and split if necessary.
		SetCurrentRun(textPosition, RunType::Merged);
		SplitCurrentRun(textPosition, RunType::Merged);
		while (textLength > 0)
		{
			LinkedRun& run = FetchNextRun(&textLength, &m_mergedCurrentRunIndex, &m_mergedRuns);
			run.script = *scriptAnalysis;
		}
	}
	catch (...)
	{
		return E_FAIL;
	}

	return S_OK;
}


HRESULT STDMETHODCALLTYPE TextAnalysis::SetBidiLevel(UINT32 textPosition, UINT32 textLength, UINT8 explicitLevel, UINT8 resolvedLevel) throw()
{
	// Will operate on the bidi and merged runs.

	try // First the bidi runs.
	{
		// Set the current run and split if necessary.
		SetCurrentRun(textPosition, RunType::BidiLevel);
		SplitCurrentRun(textPosition, RunType::BidiLevel);
		while (textLength > 0)
		{
			LinkedRun& run = FetchNextRun(&textLength, &m_bidiCurrentRunIndex, &m_bidiRuns);
			run.bidiLevel = resolvedLevel;
		}
	}
	catch (...)
	{
		return E_FAIL;
	}

	try // Now the merged runs.
	{
		// Set the current run and split if necessary.
		SetCurrentRun(textPosition, RunType::Merged);
		SplitCurrentRun(textPosition, RunType::Merged);
		while (textLength > 0)
		{
			LinkedRun& run = FetchNextRun(&textLength, &m_mergedCurrentRunIndex, &m_mergedRuns);
			run.bidiLevel = resolvedLevel;
		}
	}
	catch (...)
	{
		return E_FAIL;
	}

	return S_OK;
}


HRESULT STDMETHODCALLTYPE TextAnalysis::SetLineBreakpoints(UINT32 textPosition, UINT32 textLength, DWRITE_LINE_BREAKPOINT const* lineBreakpoints) throw()
{
	// We won't be calling AnalyzeLineBreakpoints, so this won't be called.
	return S_OK;
}


HRESULT STDMETHODCALLTYPE TextAnalysis::SetNumberSubstitution(UINT32 textPosition, UINT32 textLength, IDWriteNumberSubstitution* numberSubstitution) throw()
{
	// Not taking any action on number substitution: the mode is NONE, so this likely won't be called.
	return S_OK;
}




////////////////////////////////////////////////////////////////////////////////
// IUnknown implementation

HRESULT STDMETHODCALLTYPE TextAnalysis::QueryInterface(
	REFIID riid,
	_COM_Outptr_ void** obj
)
{
	if (riid == __uuidof(IDWriteTextAnalysisSource))
	{
		*obj = static_cast<IDWriteTextAnalysisSource*>(this);
		AddRef();
		return S_OK;
	}
	else if (riid == __uuidof(IDWriteTextAnalysisSink))
	{
		*obj = static_cast<IDWriteTextAnalysisSink*>(this);
		AddRef();
		return S_OK;
	}
	else if (riid == __uuidof(IUnknown))
	{
		*obj = static_cast<IUnknown*>( static_cast<IDWriteTextAnalysisSink*>(this));
		AddRef();
		return S_OK;
	}
	else
	{
		*obj = nullptr;
		return E_NOINTERFACE;
	}
}

ULONG STDMETHODCALLTYPE TextAnalysis::AddRef()
{
	return ++m_refCount;
}

ULONG STDMETHODCALLTYPE TextAnalysis::Release()
{
	ULONG newCount = --m_refCount;
	if (newCount == 0)
	{
		delete this;
	}
	return newCount;
}



////////////////////////////////////////////////////////////////////////////////
// Internal helper methods:

void TextAnalysis::SetCurrentRun(uint32_t textPosition, RunType runType)
{
	// For the given run type(s), move to the run for the given position.

	if (runType == RunType::Script /*|| runType == RunType::Merged*/)
	{
		SetCurrentRun_Internal(textPosition, &m_scriptCurrentRunIndex, &m_scriptRuns);
	}

	if (runType == RunType::BidiLevel /*|| runType == RunType::Merged*/)
	{
		SetCurrentRun_Internal(textPosition, &m_bidiCurrentRunIndex, &m_bidiRuns);
	}
	
	if (runType == RunType::Merged)
	{
		SetCurrentRun_Internal(textPosition, &m_mergedCurrentRunIndex, &m_mergedRuns);
	}
} // SetCurrentRun


void TextAnalysis::SetCurrentRun_Internal(uint32_t textPosition, _Inout_ UINT32* currentRunIndex, std::vector<LinkedRun>* runs)
{
	// Move to the run for the given position. Since the analyzers generally work 
	// in a forward-only manner, this will usually return early.

	if (*currentRunIndex < runs->size()
		&& (*runs)[*currentRunIndex].ContainsTextPosition(textPosition))
	{
		// Current run index remains the same.
		return;
	}

	*currentRunIndex = static_cast<uint32_t>(
		std::find(runs->begin(), runs->end(), textPosition) - runs->begin()
		);
} // SetCurrentRun_Internal



void TextAnalysis::SplitCurrentRun(uint32_t splitPosition, RunType runType)
{
	// For the given run type(s), split the current run at the given position
	// if the split position is within the current run and not at the start.

	if (runType == RunType::Script /*|| runType == RunType::Merged*/)
	{
		SplitCurrentRun_Internal(splitPosition, &m_scriptCurrentRunIndex, &m_scriptRuns);
	}

	if (runType == RunType::BidiLevel /*|| runType == RunType::Merged*/)
	{
		SplitCurrentRun_Internal(splitPosition, &m_bidiCurrentRunIndex, &m_bidiRuns);
	}

	if (runType == RunType::Merged)
	{
		SplitCurrentRun_Internal(splitPosition, &m_mergedCurrentRunIndex, &m_mergedRuns);
	}
} // SplitCurrentRun


void TextAnalysis::SplitCurrentRun_Internal(uint32_t splitPosition, _Inout_ UINT32* currentRunIndex, std::vector<LinkedRun>* runs)
{
	const uint32_t runTextStart = (*runs)[*currentRunIndex].textStart;

	if (splitPosition <= runTextStart)
	{
		// Split position is at or before the start of the run.
		return;
	}

	// Will be splitting; add a new run to the vector
	const size_t totalRuns = runs->size();
	try
	{
		runs->resize(totalRuns + 1);
	}
	catch (const std::exception&)
	{
		return; // Can't increase size (out of memory?) Return without splitting.
	}

	// Copy the old run to the end.
	LinkedRun& frontHalf = (*runs)[*currentRunIndex];
	LinkedRun& backHalf = (*runs).back();
	backHalf = frontHalf;

	// Adjust runs' text positions and lengths.
	uint32_t splitPoint = splitPosition - runTextStart;
	backHalf.textStart += splitPoint;
	backHalf.textLength -= splitPoint;
	frontHalf.textLength = splitPoint;
	frontHalf.nextRunIndex = static_cast<uint32_t>(totalRuns);
	*currentRunIndex = static_cast<uint32_t>(totalRuns);

} // SplitCurrentRun_Internal


TextAnalysis::LinkedRun& TextAnalysis::FetchNextRun(_Inout_ UINT32* textLength, _Inout_ UINT32* currentRunIndex, std::vector<LinkedRun>* runs)
{
	// Used by the sink setters to get the next run to set properties on.
	// If the properties are to be set on only a starting portion of the run,
	// this will call SplitCurrentRun_Internal() and return the front half.
	// 
	// The length is updated to reflect the remaining length of the run.
	// The caller will loop until the length is zero.


	// Get the current run index and text length before any splitting.
	UINT32 runIndex = *currentRunIndex;
	UINT32 runTextLength = (*runs)[runIndex].textLength;

	// Split the tail if needed.
	if (*textLength < runTextLength)
	{
		runTextLength = *textLength; // Limit to what's actually left.
		UINT32 runTextStart = (*runs)[runIndex].textStart;
		SplitCurrentRun_Internal(runTextStart + runTextLength, currentRunIndex, runs);
	}
	else
	{
		// This can only happen if the Set* call is setting the same data on a
		// range that spans multiple existing runs. Just advance the current run.
		*currentRunIndex = (*runs)[*currentRunIndex].nextRunIndex;
	}
	*textLength -= runTextLength;

	// Return the front half of the split, or the current run if no split occurred.
	return (*runs)[runIndex];

} // FetchNextRun


HRESULT __stdcall TextAnalysis::SetScriptProperties(_Inout_ std::vector<LinkedRun>& runs)
{
	for (size_t i = 0; i < runs.size(); i++)
	{
		DWRITE_SCRIPT_PROPERTIES scriptProperties;
		THROW_IF_FAILED(m_textAnalyzer->GetScriptProperties(runs[i].script, &scriptProperties));
		runs[i].scriptProperties = scriptProperties;
	}
	return S_OK;
}



void TextAnalysis::ReorderRuns(_In_ std::vector<LinkedRun>& analyzedRuns, _Out_ std::vector<Run>& reorderedRuns)
{
	size_t totalRuns = analyzedRuns.size();
	reorderedRuns.resize(totalRuns);

	UINT32 nextRunIndex = 0;
	for (size_t i = 0; i < totalRuns; i++)
	{
		reorderedRuns[i] = analyzedRuns[nextRunIndex];
		nextRunIndex = analyzedRuns[nextRunIndex].nextRunIndex;
	}
}

