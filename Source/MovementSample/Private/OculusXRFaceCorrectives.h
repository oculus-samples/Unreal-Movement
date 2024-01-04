/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.
This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "OculusXRMorphTargetsController.h"

/**
 * An interface for corrective shapes.
 */
class ICorrectiveShape
{
public:
	/**
	 * Calculate the weight of this corrective shape and apply it to the shape (or shapes)
	 * it drives.
	 */
	virtual void Apply(FOculusXRMorphTargetsController& Controller) = 0;
};

/**
 * Defines a combination target.
 */
struct Combination final : ICorrectiveShape
{
public:
	Combination() = default;
	Combination(const FName DrivenTarget, const TArray<FName>& DriverTargets);
	virtual ~Combination() = default;

	/** The MorphTarget name to be driven on the skinned mesh renderer. */
	FName DrivenTarget;

	/** The MorphTarget names used in calculating the final weight for the driven MorphTarget. */
	TArray<FName> DriverTargets;

	virtual void Apply(FOculusXRMorphTargetsController& Controller) override;
};

/**
 * Parsed data for a single inbetween (that may belong to a group of in-betweens for a
 * shared driver shape).
 *
 * This is used as an intermediate representation for building InBetweens.
 */
struct Partial
{
	FName DriverTarget;
	FName DrivenTarget;
	float PeakValue;
};

/**
 * Defines an in-between. More specifically, defines a single in-between (i.e. jawDrop50) that
 * has a single peak.
 */
struct InBetween final : ICorrectiveShape
{
public:
	virtual ~InBetween() = default;

	/** The target blendshape index used for calculating the blendshape weight. */
	FName DriverTarget;

	/** The blendshape index to be driven on the skinned mesh renderer. */
	FName DrivenTarget;

	/** A curve describing how this inbetween's weight should vary with the driver. */
	FRichCurve InfluenceCurve;

	virtual void Apply(FOculusXRMorphTargetsController& Controller) override;

	/**
	 * Constructs a list of independently usable InBetweens by inferring their influence
	 * curves' keyframes based on other Partials for the same driver index.
	 */
	static TArray<TSharedPtr<InBetween>> BuildFromPartials(TArray<Partial*> Partials);

private:
	/**
	 * InBetweens should be constructed via BuildFromPartials to ensure the correct curves are
	 * being set for groupings of InBetweens.
	 */
	InBetween(const FName DriverTarget, const FName DrivenTarget, const FRichCurve& InfluenceCurve);
};
