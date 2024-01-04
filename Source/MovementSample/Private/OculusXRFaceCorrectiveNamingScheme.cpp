/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.
This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXRFaceCorrectiveNamingScheme.h"

TArray<TSharedPtr<ICorrectiveShape>> NamingScheme::ParseCorrectives(TArray<FName> MorphTargetNames)
{
	TArray<TSharedPtr<ICorrectiveShape>> CorrectiveShapes;

	auto CorrectiveFound = false;
	auto PartialInBetweens = new TArray<Partial*>();

	for (int shapeIndex = 0; shapeIndex < MorphTargetNames.Num(); shapeIndex++)
	{
		if (TSharedPtr<Combination> Combination = MakeShareable(new struct Combination); TryParseCombination(MorphTargetNames, shapeIndex, Combination))
		{
			CorrectiveShapes.Add(Combination);
			CorrectiveFound = true;
		}
		else if (Partial* InBetween = new struct Partial(); TryParseInBetween(MorphTargetNames, shapeIndex, InBetween))
		{
			PartialInBetweens->Add(InBetween);
			CorrectiveFound = true;
		}
		else if (CorrectiveFound)
		{
			UE_LOG(LogTemp, Error, TEXT("Found unparseable shape name %s and assumed driver shape after finding one or more valid corrective shapes."), *MorphTargetNames[shapeIndex].ToString())
		}
	}

	auto InBetweens = InBetween::BuildFromPartials(*PartialInBetweens);
	for (auto i = 0; i < InBetweens.Num(); i++)
	{
		auto inBetween = InBetweens[i];
		if (inBetween->DrivenTarget.IsValid() && inBetween->DriverTarget.IsValid() && !inBetween->InfluenceCurve.IsEmpty())
		{
			CorrectiveShapes.Add(inBetween);
		}
		else
			UE_LOG(LogTemp, Error, TEXT("Invalid InBetween Corrective"))
	}

	return CorrectiveShapes;
}

bool NamingSchemes::NamingSchemeV0::TryParseCombination(TArray<FName>& MorphTargetNames, int Index, TSharedPtr<Combination> Combination)
{
	const auto MorphTargetName = MorphTargetNames[Index];
	FRegexMatcher Matcher(FRegexPattern(TEXT("^([a-zA-Z0-9]{4,25}(_[a-zA-Z0-9]{4,25})+)(_([A-Z]{1,2}))?$")), MorphTargetName.ToString());

	// Return if no matches
	if (!Matcher.FindNext())
	{
		return false;
	}

	// TODO: Check if groups number is higher than 5

	FString ElementsWithUnderscore = Matcher.GetCaptureGroup(1);
	TArray<FString> Elements;

	// Split the string on the underscore character
	ElementsWithUnderscore.ParseIntoArray(Elements, TEXT("_"));

	FString Suffix = "";
	if (Matcher.GetCaptureGroup(4).Len() > 0)
	{
		Suffix = Matcher.GetCaptureGroup(4);
	}

	TArray<FName> DriverTargets;
	for (auto i = 0; i < Elements.Num(); i++)
	{
		auto Element = Elements[i];
		FName DriverName = FName(*Element);
		if (FName Result; FindDriver(MorphTargetNames, DriverName, Suffix, Result))
		{
			DriverTargets.Add(Result);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Could not find shape %s"), *Element)
			return false;
		}
	}

	Combination->DrivenTarget = MorphTargetName;
	Combination->DriverTargets = TArray<FName>(DriverTargets);

	return true;
}

bool NamingSchemes::NamingSchemeV0::FindDriver(TArray<FName>& MorphTargetNames, FName& DriverName, FString Suffix, FName& Result)
{
	int DriverIndex = MorphTargetNames.IndexOfByKey(DriverName);
	if (DriverIndex != INDEX_NONE)
	{
		Result = MorphTargetNames[DriverIndex];
		return true;
	}

	if (Suffix.Len() > 2)
	{
		UE_LOG(LogTemp, Error, TEXT("Expected suffix of length 0-2, got %s"), *Suffix);
	}

	if (Suffix.Len() > 0)
	{
		FString DriverNameString = FString::Printf(TEXT("%s_%c"), *DriverName.ToString(), Suffix[0]);
		FName DriverNameFName(*DriverNameString);

		DriverIndex = MorphTargetNames.IndexOfByKey(DriverNameFName);
		if (DriverIndex != INDEX_NONE)
		{
			Result = MorphTargetNames[DriverIndex];
			return true;
		}
	}

	if (Suffix.Len() > 1)
	{
		FString DriverNameString = FString::Printf(TEXT("%s_%c"), *DriverName.ToString(), Suffix[1]);
		FName DriverNameFName(*DriverNameString);
		DriverIndex = MorphTargetNames.IndexOfByKey(DriverNameFName);

		if (DriverIndex != INDEX_NONE)
		{
			Result = MorphTargetNames[DriverIndex];
			return true;
		}

		FString DriverNameString2 = FString::Printf(TEXT("%s_%c"), *DriverName.ToString(), *Suffix);
		FName DriverNameFName2(*DriverNameString2);
		DriverIndex = MorphTargetNames.IndexOfByKey(DriverNameFName2);

		if (DriverIndex != INDEX_NONE)
		{
			Result = MorphTargetNames[DriverIndex];
			return true;
		}
	}

	return false;
}

bool NamingSchemes::NamingSchemeV0::TryParseInBetween(TArray<FName>& MorphTargetNames, int Index, Partial* InBetween)
{
	const auto MorphTargetName = MorphTargetNames[Index];

	FRegexMatcher Matcher(FRegexPattern(TEXT("^([a-zA-Z]+)([0-9]{2})(_([A-Z]{1,2}))?$")), MorphTargetName.ToString());

	if (!Matcher.FindNext())
	{
		return false;
	}

	// TODO: Check that we have at least 5 capture groups or throw an error

	FString DriverNameString = Matcher.GetCaptureGroup(1);
	FName DriverName(*DriverNameString);

	// The highest number from the naming scheme (25, 50, 75, 100)
	int32 PeakValue = FCString::Atoi(*Matcher.GetCaptureGroup(2));

	if (PeakValue <= 0 || PeakValue >= 100)
	{
		UE_LOG(LogTemp, Warning, TEXT("Parsed value in %s is outside of (0, 100): %d"), *MorphTargetName.ToString(), PeakValue);
		return false;
	}

	FString Suffix = "";
	if (Matcher.GetCaptureGroup(4).Len() > 0)
	{
		Suffix = Matcher.GetCaptureGroup(4);
	}
	FName Result;
	if (!FindDriver(MorphTargetNames, DriverName, Suffix, Result))
	{
		// combination = ; ????????
		UE_LOG(LogTemp, Error, TEXT("Could not find shape %s"), *DriverName.ToString())
		return false;
	}

	InBetween->DrivenTarget = MorphTargetName;
	InBetween->DriverTarget = Result;
	InBetween->PeakValue = static_cast<float>(PeakValue) * 0.01f;

	return true;
}

FString NamingSchemes::NamingSchemeV0::VersionName()
{
	return TEXT("V0");
}
