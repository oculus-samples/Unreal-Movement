/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "AnimGraphNode_Base.h"
#include "OculusXRRetargeterAnimNode.h"
#include "OculusXRRetargetIKRetargeterEditor.generated.h"

class FPrimitiveDrawInterface;
class USkeletalMeshComponent;

// Editor node for IKRig
UCLASS()
class OCULUSXRRETARGETINGEDITOR_API UOculusXRRetargetIKRetargeterEditor : public UAnimGraphNode_Base
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Settings)
	FOculusXRRetargeterAnimNode Node;

public:
	// UEdGraphNode interface
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog);
	// End of UEdGraphNode interface

	// UAnimGraphNode_Base interface
	virtual void PreloadRequiredAssets() override;
	virtual FEditorModeID GetEditorMode() const override;
	virtual void Draw(FPrimitiveDrawInterface* PDI, USkeletalMeshComponent* PreviewSkelMeshComp) const override;
	virtual void CustomizePinData(UEdGraphPin* Pin, FName SourcePropertyName, int32 ArrayIndex) const override;
	virtual bool UsingCopyPoseFromMesh() const override { return true; };
	// End of UAnimGraphNode_Base interface

	// UK2Node interface
	virtual UObject* GetJumpTargetForDoubleClick() const override;
	// End of UK2Node interface

	static const FName AnimModeName;
};
