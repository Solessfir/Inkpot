#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Asset/InkpotStoryAsset.h"
#include "Inkpot/InkpotStories.h"

namespace
{
	constexpr const TCHAR* StoryJSON = TEXT("{\"inkVersion\":21,\"root\":[\"^First\",\"\\n\",\"^Second\",\"\\n\",\"end\",null],\"listDefs\":{}}");
	constexpr const TCHAR* UpdatedStoryJSON = TEXT("{\"inkVersion\":21,\"root\":[\"^Updated\",\"\\n\",\"end\",null],\"listDefs\":{}}");
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInkpotConcurrentStoriesTest, "Inkpot.Stories.ConcurrentInstances", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInkpotConcurrentStoriesTest::RunTest(const FString& Parameters)
{
	UInkpotStories* Registry = NewObject<UInkpotStories>();
	Registry->Initialise();
	UInkpotStoryAsset* Asset = NewObject<UInkpotStoryAsset>();
	Asset->SetCompiledJSON(StoryJSON);
	UInkpotStory* First = Registry->BeginStory(Asset);
	UInkpotStory* Second = Registry->BeginStory(Asset);
	if (!TestTrue(TEXT("Both stories are valid"), First->IsValid() && Second->IsValid()))
		return false;

	TestTrue(TEXT("Each story has its own object"), First != Second);
	TestTrue(TEXT("Each story has its own handle"), First->GetID() != Second->GetID());
	TestEqual(TEXT("Both stories remain registered"), Registry->GetStories().Num(), 2);
	TestTrue(TEXT("Asset lookup selects the latest instance"), Registry->GetStory(Asset) == Second);
	TestTrue(TEXT("First story retains its asset"), Registry->GetStoryAsset(First) == Asset);
	TestTrue(TEXT("Second story retains its asset"), Registry->GetStoryAsset(Second) == Asset);

	First->Continue();
	TestEqual(TEXT("First story advances"), First->GetCurrentText(), FString(TEXT("First")));
	TestTrue(TEXT("Second story is still at its beginning"), Second->GetCurrentText().IsEmpty());
	Registry->Replay(First);
	TestEqual(TEXT("First story replays its own history"), First->GetCurrentText(), FString(TEXT("First")));
	TestTrue(TEXT("Replay leaves the other story untouched"), Second->GetCurrentText().IsEmpty());

	Registry->EndStory(First);
	Registry->EndStory(First);
	TestEqual(TEXT("Ending first story leaves second registered"), Registry->GetStories().Num(), 1);
	TestTrue(TEXT("Ended story has no registered asset"), Registry->GetStoryAsset(First) == nullptr);
	TestTrue(TEXT("Second story remains accessible"), Registry->GetStory(Asset) == Second);
	Second->Continue();
	TestEqual(TEXT("Second story can continue independently"), Second->GetCurrentText(), FString(TEXT("First")));

	Registry->Reset();
	UInkpotStoryAsset* ReplacementAsset = NewObject<UInkpotStoryAsset>();
	ReplacementAsset->SetCompiledJSON(UpdatedStoryJSON);
	UInkpotStory* AfterReset = Registry->BeginStory(ReplacementAsset);
	TestEqual(TEXT("Reset reuses the old numeric handle"), AfterReset->GetID(), First->GetID());
	Registry->EndStory(First);
	TestTrue(TEXT("Stale story cannot end a new instance"), Registry->GetStory(ReplacementAsset) == AfterReset);
	TestTrue(TEXT("Stale story cannot retrieve a new instance's asset"), Registry->GetStoryAsset(First) == nullptr);
	TestTrue(TEXT("Replacement story retains its own asset"), Registry->GetStoryAsset(AfterReset) == ReplacementAsset);
	AfterReset->Continue();
	AfterReset->ResetState();
	TestTrue(TEXT("Replacement state is reset while its history remains"), AfterReset->GetCurrentText().IsEmpty());
	Registry->Replay(First, true);
	TestTrue(TEXT("Stale story cannot replay a new instance's history"), AfterReset->GetCurrentText().IsEmpty());
	Registry->Replay(AfterReset, true);
	TestEqual(TEXT("Replacement story can replay its own history"), AfterReset->GetCurrentText(), FString(TEXT("Updated")));
	Registry->EndStory(AfterReset);
	TestEqual(TEXT("All stories ended"), Registry->GetStories().Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInkpotReloadStoriesTest, "Inkpot.Stories.ReloadLatestInstance", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInkpotReloadStoriesTest::RunTest(const FString& Parameters)
{
	UInkpotStories* Registry = NewObject<UInkpotStories>();
	Registry->Initialise();
	UInkpotStoryAsset* Asset = NewObject<UInkpotStoryAsset>();
	Asset->SetCompiledJSON(StoryJSON);
	UInkpotStory* First = Registry->BeginStory(Asset);
	UInkpotStory* Second = Registry->BeginStory(Asset);
	if (!TestTrue(TEXT("Both stories are valid"), First->IsValid() && Second->IsValid()))
		return false;

	First->Continue();
	Second->Continue();
	const int32 SecondHandle = Second->GetID();
	Asset->SetCompiledJSON(UpdatedStoryJSON);
	TestTrue(TEXT("Reload selects the latest instance"), Registry->Reload(Asset) == Second);
	TestEqual(TEXT("Reload preserves the handle"), Second->GetID(), SecondHandle);
	TestEqual(TEXT("Reload keeps both instances registered"), Registry->GetStories().Num(), 2);
	TestEqual(TEXT("Reload leaves the older instance untouched"), First->GetCurrentText(), FString(TEXT("First")));
	Second->Continue();
	TestEqual(TEXT("Reload uses updated content"), Second->GetCurrentText(), FString(TEXT("Updated")));

	Registry->EndStory(Second);
	TestTrue(TEXT("Asset lookup falls back to the remaining instance"), Registry->GetStory(Asset) == First);
	TestTrue(TEXT("Reload falls back to the remaining instance"), Registry->Reload(Asset) == First);
	First->Continue();
	TestEqual(TEXT("Remaining instance reloads updated content"), First->GetCurrentText(), FString(TEXT("Updated")));
	Registry->EndStory(First);
	TestTrue(TEXT("Reload returns null when no instance is running"), Registry->Reload(Asset) == nullptr);
	return true;
}

#endif
