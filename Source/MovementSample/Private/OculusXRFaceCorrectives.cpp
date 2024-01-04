/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXRFaceCorrectives.h"

Combination::Combination(const FName DrivenTarget, const TArray<FName>& DriverTargets)
{
	this->DrivenTarget = DrivenTarget;
	this->DriverTargets = DriverTargets;
}

void Combination::Apply(FOculusXRMorphTargetsController& Controller)
{
	// Multiplies all driver values together to get the product
	float Product = 1.0f;
	for (int i = 0; i < this->DriverTargets.Num(); i++)
	{
		const auto Driver = this->DriverTargets[i];	  // Name of driver
		Product *= Controller.GetMorphTarget(Driver); // Multiply by driver value
	}

	Controller.SetMorphTarget(DrivenTarget, Product); // Set driven value to product
}

InBetween::InBetween(const FName DriverTarget, const FName DrivenTarget, const FRichCurve& InfluenceCurve)
{
	this->DriverTarget = DriverTarget;
	this->DrivenTarget = DrivenTarget;
	this->InfluenceCurve = InfluenceCurve;
}

void InBetween::Apply(FOculusXRMorphTargetsController& Controller)
{
	// Get the weight of the driver
	const float DriverWeight = Controller.GetMorphTarget(DriverTarget);
	// Sample the curve and set driven value
	Controller.SetMorphTarget(DrivenTarget, this->InfluenceCurve.Eval(DriverWeight));
}

TArray<TSharedPtr<InBetween>> InBetween::BuildFromPartials(TArray<Partial*> Partials)
{
	TMap<FName, TArray<Partial>> GroupedPartials;

	// Group Partials by driver index in order to easier find
	// which Driver target maps to multiple Driven targets
	for (int i = 0; i < Partials.Num(); i++)
	{
		Partial* Partial = Partials[i];
		GroupedPartials.FindOrAdd(Partial->DriverTarget).Add(*Partial);
	}

	TArray<TSharedPtr<InBetween>> InBetweens;

	// Construct a curve for each group of Partials
	// This curve will determine how the much the Driver influences the Driven target
	for (auto& KeyValuePair : GroupedPartials)
	{
		TArray<Partial>& Subshapes = KeyValuePair.Value;
		// Sort subshapes by peak value in order to create the influence curve
		Subshapes.Sort([](const Partial& A, const Partial& B) {
			return A.PeakValue < B.PeakValue;
		});

		for (int32 i = 0; i < Subshapes.Num(); ++i)
		{
			TArray<FRichCurveKey> Keys;
			Keys.Add(FRichCurveKey(0.0f, 0.0f));

			if (i != 0)
			{
				Keys.Add(FRichCurveKey(Subshapes[i - 1].PeakValue, 0.0f));
			}

			Keys.Add(FRichCurveKey(Subshapes[i].PeakValue, 1.0f));

			if (i < Subshapes.Num() - 1)
			{
				Keys.Add(FRichCurveKey(Subshapes[i + 1].PeakValue, 0.0f));
			}

			Keys.Add(FRichCurveKey(1.0f, 0.0f));

			FRichCurve Curve;
			for (const FRichCurveKey& Key : Keys)
			{
				Curve.AddKey(Key.Time, Key.Value);
			}

			TSharedPtr<InBetween> InBetweenPtr = MakeShareable(new InBetween(KeyValuePair.Key, Subshapes[i].DrivenTarget, MoveTemp(Curve)));
			InBetweens.Add(InBetweenPtr);
		}
	}

	return InBetweens;
}
