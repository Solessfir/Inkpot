#pragma once

#include "CoreMinimal.h"
#include "InkpotLine.generated.h"

class UInkpotStory;

UCLASS(BlueprintType)
class INKPOT_API UInkpotLine : public UObject
{
	GENERATED_BODY()

public:
	UInkpotLine();
	void Initialise(const FString &String);

	UFUNCTION(BlueprintPure, Category="Inkpot|Line")
	const FString& GetString() const;

	UFUNCTION(BlueprintPure, Category="Inkpot|Line")
	const FText& GetText() const;

	UFUNCTION(BlueprintPure, Category="Inkpot|Line")
	const TArray<FString> &GetTags() const;

	UFUNCTION(BlueprintPure, Category="Inkpot|Line")
	UInkpotStory* GetStory() const;

	UFUNCTION(BlueprintPure, Category = "Inkpot|Line")
	bool IsDirty();

	UFUNCTION(BlueprintCallable, Category = "Inkpot|Line")
	void SetDirty(bool bIsDirty );

private:
	virtual const TArray<FString> &GetTagsInternal() const;

private:
	UPROPERTY(VisibleAnywhere, Category="Inkpot|Line")
	FString String;

	UPROPERTY(VisibleAnywhere, Category="Inkpot|Line")
	FText Text;

	UPROPERTY(VisibleAnywhere, Category = "Inkpot|Line")
	bool bIsDirty;
};
