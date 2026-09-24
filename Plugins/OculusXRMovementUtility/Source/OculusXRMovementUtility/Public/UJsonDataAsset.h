#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
// The generated header must be the last include before the class declaration
#include "UJsonDataAsset.generated.h"
UCLASS(BlueprintType)
class OCULUSXRMOVEMENTUTILITY_API UJsonDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UJsonDataAsset();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JSON")
	FString JsonText;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Instanced, Category = "Import Settings")
	class UAssetImportData* AssetImportData;
#endif

#if WITH_EDITOR
	UFUNCTION(
		CallInEditor,
		Category = "Import/Export",
		meta = (DisplayName = "Load JSON Data",
			ToolTip = "Loads Data from a JSON/jsondata file into the UAsset"))
	void LoadFromJson();

	UFUNCTION(CallInEditor,
		Category = "Import/Export",
		meta = (DisplayName = "Save to JsonData",
			ToolTip = "Saves data from the UAsset to a specified JsonData file"))
	void SaveToJson();

	UFUNCTION(CallInEditor,
		Category = "Import/Export",
		meta = (DisplayName = "Refresh JsonData",
			ToolTip = "Reloads the Source JsonData file into the UAsset (if the file exists)"))
	void RefreshFromSource();

	virtual void PostInitProperties() override;
	virtual void GetAssetRegistryTags(FAssetRegistryTagsContext Context) const override;
#endif
};
