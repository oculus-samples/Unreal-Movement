/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXRFaceTrackingCorrectives.h"
#include "OculusXRFaceCorrectiveNamingScheme.h"
#include "MovementSample/MovementSample.h"

// Sets default values for this component's properties
UOculusXRFaceTrackingCorrectives::UOculusXRFaceTrackingCorrectives()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	bUseCorrectives = true;
	NamingSchemeVersion = V0;

	TargetMeshComponent = nullptr;
}

// Called when the game starts
void UOculusXRFaceTrackingCorrectives::BeginPlay()
{
	Super::BeginPlay();

	if (!InitializeCorrectives())
	{
		UE_LOG(LogOculusXRMovementSample, Warning, TEXT("Could not initialize correctives. (%s:%s)"), *GetOwner()->GetName(), *GetName());
		SetComponentTickEnabled(false);
		return;
	}
}

bool UOculusXRFaceTrackingCorrectives::InitializeCorrectives()
{
	TargetMeshComponent = Cast<USkinnedMeshComponent>(GetOwner()->GetComponentByClass(USkinnedMeshComponent::StaticClass()));

	if (!IsValid(TargetMeshComponent))
	{
		UE_LOG(LogOculusXRMovementSample, Warning, TEXT("Could not find skeletal mesh component on this actor: (%s)"), *GetOwner()->GetName());
		return false;
	}

	if (TargetMeshComponent == nullptr)
		return false;

	USkeletalMesh* TargetMesh = Cast<USkeletalMesh>(TargetMeshComponent->GetSkinnedAsset());
	if (TargetMesh == nullptr)
		return false;

	const auto MorphTargetsRaw = TargetMesh->GetMorphTargets();

	TArray<FName> MorphTargetNames;

	// Collect all the morph target names
	for (int i = 0; i < MorphTargetsRaw.Num(); i++)
	{
		auto MorphTargetName = FName(MorphTargetsRaw[i].GetName());
		// Store them into an array to avoid calling GetMorphTargets() every tick
		MorphTargetNames.Add(MorphTargetName);
	}

	// Parse the correctives
	NamingScheme* NamingScheme;
	switch (NamingSchemeVersion)
	{
		case V0:
			NamingScheme = NamingSchemes::V0();
			break;
		default:
			UE_LOG(LogOculusXRMovementSample, Warning, TEXT("Trying to parse correctives with unsupported naming scheme version (%s:%s)"), *GetOwner()->GetName(), *GetName());
			return false;
	}
	Correctives = NamingScheme->ParseCorrectives(MorphTargetNames);
	delete NamingScheme;

	return true;
}

// Called every frame
void UOculusXRFaceTrackingCorrectives::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// WARNING: This component needs to update after OculusXRFaceTrackingComponent as this is a post-processing step.
	// The order should be set in the parent actor blueprint.
	if (bUseCorrectives)
	{
		for (int i = 0; i < Correctives.Num(); i++)
		{
			Correctives[i]->Apply(MorphTargets);
		}
		MorphTargets.ApplyMorphTargets(TargetMeshComponent);
	}
}
