/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.
This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "CoreMinimal.h"
#include "OculusXRFaceCorrectiveNamingScheme.h"
#include "OculusXRFaceCorrectives.h"
#include "Components/ActorComponent.h"
#include "OculusXRFaceTrackingCorrectives.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UOculusXRFaceTrackingCorrectives : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UOculusXRFaceTrackingCorrectives();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	/**
	 * If true, the correctives driver will apply correctives.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OculusXR|Movement")
	bool bUseCorrectives;

	/*
	 * Determines which naming scheme to use for the correctives.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OculusXR|Movement")
	TEnumAsByte<ENamingSchemeVersion> NamingSchemeVersion;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	bool InitializeCorrectives();

	// The mesh component targeted for expressions
	UPROPERTY()
	USkinnedMeshComponent* TargetMeshComponent;

	// Morph targets controller
	FOculusXRMorphTargetsController MorphTargets;

	/**
	 * Collection of correctives to be used on the morph targets.
	 * This turns some MorphTargets into drivers for other MorphTargets.
	 */
	TArray<TSharedPtr<ICorrectiveShape>> Correctives;
};
