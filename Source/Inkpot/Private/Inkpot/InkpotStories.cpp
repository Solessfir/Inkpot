#include "Inkpot/InkpotStories.h"
#include "Inkpot/InkpotStoryInternal.h"
#include "Inkpot/InkpotStoryFactory.h"
#include "Settings/InkpotSettings.h"
#include "Asset/InkpotStoryAsset.h"
#include "Ink/Choice.h"
#include "Utility/InkpotLog.h"

UInkpotStories::UInkpotStories()
: Super()
{
}

void UInkpotStories::Initialise()
{
	InitialiseStoryFactory();
	Reset();
}

void UInkpotStories::InitialiseStoryFactory()
{
	const UInkpotSettings *settings = GetDefault<UInkpotSettings>();
	FSoftClassPath storyFactoryClassName = settings->StoryFactoryClass;
	UClass* storyFactoryClass = (storyFactoryClassName.IsValid() ? LoadObject<UClass>(NULL, *storyFactoryClassName.ToString() ) : UInkpotStoryFactory::StaticClass() );
	if (!storyFactoryClass)
	{
		INKPOT_ERROR("Inkpot story factory class is not set");
		return;
	}
	StoryFactory = NewObject<UInkpotStoryFactoryBase>(this, storyFactoryClass);
	StoryFactory->Initialise();
}


void UInkpotStories::Reset()
{
	StoryFactory->Reset();
	NextStoryHandle = 0;
	Stories.Empty();
	StoryAssets.Empty();
	StoryHistories.Empty();
}

UInkpotStory* UInkpotStories::BeginStory( UInkpotStoryAsset* InInkpotStoryAsset )
{
	if(!IsValid(InInkpotStoryAsset))
	{
		INKPOT_ERROR("Invalid Story Asset, could not begin story!");
		return StoryFactory->BadStory();
	}

	const int32 handle = NextStoryHandle++;

	UInkpotStory* storyNew = StoryFactory->CreateStory(InInkpotStoryAsset, handle, this);

	if (storyNew->GetID() != FInkpotStoryInternal::BadStoryHandle)
	{
		Stories.Emplace(handle, storyNew);

		StoryAssets.Emplace(handle, InInkpotStoryAsset);

		UInkpotStoryHistory* history = NewObject<UInkpotStoryHistory>(storyNew);
		history->Initialise(storyNew);
		StoryHistories.Emplace(handle, history);
	}

	return storyNew;
}

void UInkpotStories::EndStory(UInkpotStory *InStory)
{
	if(!IsValid(InStory))
		return;
	const TObjectPtr<UInkpotStory>* RegisteredStory = Stories.Find(InStory->GetID());
	if (!RegisteredStory || RegisteredStory->Get() != InStory)
		return;
	EndStory( InStory->GetID() );
}

void UInkpotStories::EndStory(int32 InStoryHandle)
{
	if(!Stories.Contains(InStoryHandle))
		return;
	Stories.Remove( InStoryHandle );
	StoryAssets.Remove( InStoryHandle );
	StoryHistories.Remove( InStoryHandle );
}

UInkpotStory* UInkpotStories::GetStory( int32 InStoryHandle ) const
{
	if(!Stories.Contains(InStoryHandle))
	{
		INKPOT_ERROR("Story ID does not exist.");
		return StoryFactory->BadStory();
	}
	return Stories[InStoryHandle];
}

int32 UInkpotStories::FindStoryHandle(TSoftObjectPtr<UInkpotStoryAsset> InStoryAssetPath) const
{
	const FString AssetPath = InStoryAssetPath.ToString();
	int32 StoryHandle = FInkpotStoryInternal::BadStoryHandle;
	// Asset-based APIs select the most recently started instance.
	for (const auto& Pair : StoryAssets)
	{
		if (Pair.Key > StoryHandle && AssetPath.Equals(Pair.Value->GetPathName()))
		{
			StoryHandle = Pair.Key;
		}
	}
	return StoryHandle;
}

UInkpotStory* UInkpotStories::GetStory( TSoftObjectPtr<UInkpotStoryAsset>  InStoryAssetPath )
{
	const int32 StoryHandle = FindStoryHandle(InStoryAssetPath);
	if (StoryHandle != FInkpotStoryInternal::BadStoryHandle)
		return GetStory(StoryHandle);
	INKPOT_ERROR("Story '%s' is not running.", *InStoryAssetPath.ToString());
	return StoryFactory->BadStory();
}

UInkpotStoryAsset* UInkpotStories::GetStoryAsset(UInkpotStory* InkpotStory)
{
	if (!IsValid(InkpotStory))
		return nullptr;
	const TObjectPtr<UInkpotStory>* RegisteredStory = Stories.Find(InkpotStory->GetID());
	if (!RegisteredStory || RegisteredStory->Get() != InkpotStory)
		return nullptr;
	int handle = InkpotStory->GetID();

	TObjectPtr<UInkpotStoryAsset>* storyAssetPtr = StoryAssets.Find(handle);
	if (!storyAssetPtr)
		return nullptr;

	return *storyAssetPtr;
}

UInkpotStory* UInkpotStories::Reload( UInkpotStoryAsset* InInkpotStoryAsset )
{
	const int32 handle = FindStoryHandle(InInkpotStoryAsset);
	if(handle == FInkpotStoryInternal::BadStoryHandle)
		return nullptr;

	TObjectPtr<UInkpotStory>* storyInProgressPtr = Stories.Find( handle );
	if(!storyInProgressPtr)
		return nullptr;

	TSharedPtr<FInkpotStoryInternal> storyInternal = StoryFactory->CreateInternalStory( InInkpotStoryAsset, handle );
	if(!storyInternal.IsValid() || !storyInternal->IsValidStory())
		return nullptr;

	UInkpotStory *story = storyInProgressPtr->Get();
	story->ResetContent( storyInternal );
	return story;
}

void UInkpotStories::Replay( UInkpotStory* InStory, bool bInResetState )
{
	if(!IsValid(InStory))
		return;
	const TObjectPtr<UInkpotStory>* RegisteredStory = Stories.Find(InStory->GetID());
	if (!RegisteredStory || RegisteredStory->Get() != InStory)
		return;

	int index = InStory->GetID();
	TObjectPtr<UInkpotStoryHistory>* history = StoryHistories.Find( index );
	if(!history)
		return;

	history->Get()->Replay( bInResetState );
}

TArray<UInkpotStory*> UInkpotStories::GetStories()
{
	TArray<UInkpotStory*> stories;
	stories.Reserve(Stories.Num());
	for (auto pair : Stories)
		stories.Add( pair.Value.Get() );
	return stories;
}
