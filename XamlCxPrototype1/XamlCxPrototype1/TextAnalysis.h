#pragma once
#include "pch.h"

class TextAnalysis :
	public IDWriteTextAnalysisSource, public IDWriteTextAnalysisSink
{
public:

    ////////////////////////////////////////////////////////////////////////////////
	// structs used for storing analysis results:

    // A single contiguous run of text with the same script analysis.
    struct Run
    {
        Run() noexcept
        :   textStart(0),
            textLength(0),
            script(),
            scriptProperties(),
            bidiLevel(0)
        { }

        UINT32 textStart;							// starting text position of the run (in UTF-16 code units)
        UINT32 textLength;							// number of UTF-16 code units
        DWRITE_SCRIPT_ANALYSIS script;				// script analysis for the run
        DWRITE_SCRIPT_PROPERTIES scriptProperties;	// script properties for the run
        UINT8 bidiLevel;							// resolved level of bidirectional embedding (0 if no override)
        bool isNumberSubstituted;					// true if the run is number substituted

        Platform::String^ ScriptTag();

        inline bool ContainsTextPosition(UINT32 desiredTextPosition) const throw()
        {
            return desiredTextPosition >= textStart
                && desiredTextPosition < textStart + textLength;
        }

        inline bool operator==(uint32_t desiredTextPosition) const throw()
        {
            // Allows search by text position using std::find
            return ContainsTextPosition(desiredTextPosition);
        }

    }; // struct Run


    // Single text analysis run, which points to the next run.
    struct LinkedRun : Run
    {
        LinkedRun() throw()
            : nextRunIndex(0)
        { }

        uint32_t nextRunIndex;  // index of next run
    };


    
    ////////////////////////////////////////////////////////////////////////////////
	// Class-specific methods:

    TextAnalysis();

	void SetText(Platform::String^ text);

	void SetReadingDirection(DWRITE_READING_DIRECTION readingDirection);

    HRESULT STDMETHODCALLTYPE GenerateAnalysis(
		OUT std::vector<Run>& scriptRuns,
		OUT std::vector<Run>& bidiLevelRuns,
		OUT std::vector<Run>& mergedRuns
    );

	Platform::String^ GetTextRun(UINT32 textPosition, UINT32 textLength) const;



    ////////////////////////////////////////////////////////////////////////////////
    // IDWriteTextAnalysisSource methods:

    HRESULT STDMETHODCALLTYPE GetTextAtPosition(
        UINT32 textPosition,
        _Outptr_result_buffer_(*textLength) WCHAR const** textString,
        _Out_ UINT32* textLength
    );

    HRESULT STDMETHODCALLTYPE GetTextBeforePosition(
        UINT32 textPosition,
        _Outptr_result_buffer_(*textLength) WCHAR const** textString,
        _Out_ UINT32* textLength
    );

    DWRITE_READING_DIRECTION STDMETHODCALLTYPE GetParagraphReadingDirection();

    HRESULT STDMETHODCALLTYPE GetLocaleName(
        UINT32 textPosition,
        _Out_ UINT32* textLength,
        _Outptr_result_z_ WCHAR const** localeName
    ) override;

    HRESULT STDMETHODCALLTYPE GetNumberSubstitution(
        UINT32 textPosition,
        _Out_ UINT32* textLength,
        _COM_Outptr_ IDWriteNumberSubstitution** numberSubstitution
    );


    ////////////////////////////////////////////////////////////////////////////////
	// IDWriteTextAnalysisSink methods:

    HRESULT STDMETHODCALLTYPE SetScriptAnalysis(
        UINT32 textPosition,
        UINT32 textLength,
        DWRITE_SCRIPT_ANALYSIS const* scriptAnalysis
    ) throw();

    HRESULT STDMETHODCALLTYPE SetLineBreakpoints(
        UINT32 textPosition,
        UINT32 textLength,
        const DWRITE_LINE_BREAKPOINT* lineBreakpoints // [textLength]
    ) throw();

    HRESULT STDMETHODCALLTYPE SetBidiLevel(
        UINT32 textPosition,
        UINT32 textLength,
        UINT8 explicitLevel,
        UINT8 resolvedLevel
    ) throw();

    HRESULT STDMETHODCALLTYPE SetNumberSubstitution(
        UINT32 textPosition,
        UINT32 textLength,
        IDWriteNumberSubstitution* numberSubstitution
    ) throw();




    ////////////////////////////////////////////////////////////////////////////////
    // IDWriteTextAnalysisSource and IDWriteTextAnalysisSink both inherit from IUnknown:

    HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID riid,
        _COM_Outptr_ void** obj
    ) override;
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;



protected:

    ////////////////////////////////////////////////////////////////////////////////
	// Internal helper methods:

	enum class RunType
	{
		Script,
		BidiLevel,
		Merged
	};

    void SetCurrentRun(uint32_t textPosition, RunType runType);
	void SetCurrentRun_Internal(uint32_t textPosition, _Inout_ UINT32* currentRunIndex, std::vector<LinkedRun>* runs);

    void SplitCurrentRun(uint32_t splitPosition, RunType runType);
	void SplitCurrentRun_Internal(uint32_t splitPosition, _Inout_ UINT32* currentRunIndex, std::vector<LinkedRun>* runs);

    LinkedRun& FetchNextRun(_Inout_ UINT32* textLength, _Inout_ UINT32* currentRunIndex, std::vector<LinkedRun>* runs);
	HRESULT STDMETHODCALLTYPE SetScriptProperties(_Inout_ std::vector<LinkedRun>& runs);
	void ReorderRuns(_In_ std::vector<LinkedRun>& analysisRuns, _Out_ std::vector<Run>& reorderedRuns);



private:
    uint32_t                                    m_refCount = 0;
	wil::com_ptr<IDWriteFactory2>               m_dwriteFactory;
	wil::com_ptr<IDWriteTextAnalyzer1>          m_textAnalyzer;

	// for IDWriteTextAnalysisSource 
    std::wstring                                m_text = L"";
	UINT32									    m_textLength = 0;
	DWRITE_READING_DIRECTION			        m_readingDirection = DWRITE_READING_DIRECTION_LEFT_TO_RIGHT;
	wil::com_ptr<IDWriteNumberSubstitution>     m_numberSubstitution;

	// for IDWriteTextAnalysisSink and GenerateAnalysis()
    std::vector<LinkedRun> m_scriptRuns;
    std::vector<LinkedRun> m_bidiRuns;
    std::vector<LinkedRun> m_mergedRuns;

    // Current processing state
    UINT32 m_scriptCurrentRunIndex;
    UINT32 m_bidiCurrentRunIndex;
    UINT32 m_mergedCurrentRunIndex;
    //UINT32 m_scriptCurrentPosition;
    //UINT32 m_bidiCurrentPosition;
    //UINT32 m_mergedCurrentPosition;

};

