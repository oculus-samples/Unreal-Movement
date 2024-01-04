/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXRFacialExpressionToAnimationCurveEditor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OculusXRFacialExpressionToAnimationCurveEditor)

#define LOCTEXT_NAMESPACE "OculusXRFacialExpressionToAnimationCurveEditor"
const FName UOculusXRFacialExpressionToAnimationCurveEditor::AnimModeName(TEXT("FaceRetarget.FaceRetargetEditor.FaceRetargetEditMode"));

FText UOculusXRFacialExpressionToAnimationCurveEditor::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("OculusXRFacialExpressionToAnimationCurveEditor_Title", "OculusXR Facial Expression To Animation Curve");
}

FEditorModeID UOculusXRFacialExpressionToAnimationCurveEditor::GetEditorMode() const
{
	return AnimModeName;
}

#undef LOCTEXT_NAMESPACE
