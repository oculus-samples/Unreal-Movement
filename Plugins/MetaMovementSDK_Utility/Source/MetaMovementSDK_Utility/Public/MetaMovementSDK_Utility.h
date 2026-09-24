// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

// C-Style format must be maintained for C compatibility
// @nolint

#ifndef MetaMovementSDK_Utility_h
#define MetaMovementSDK_Utility_h

#include "MetaMovementSDK_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************
 *
 *               Lifecycle Functions
 *
 **********************************************************/

// Integrations need to ensure this is called by Host Applications
// Default value is Y-Up, LH (will only work for Unity and engines
// that match coordinate spacing)
META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_initialize(
    const metaMovementSDK_CoordinateSpace* coordinateSpace);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_initializeLogging(metaMovementSDK_LogCallback logCallback);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_createOrUpdateHandle(
    const char* config,
    metaMovementSDK_Handle* in_out_handle);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_destroy(metaMovementSDK_Handle handle);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_destroyAllHandles();

// NOTE: optional_knownJointNamesById is either
// nullptr, or assumed to be of length metaMovementSDK_KnownJointType::KnownJointCount

// This function is used for a simple config definition of either a source only or
// target only configuration.
META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_createOrUpdateSimpleUtilityConfig(
    const char* configName,
    metaMovementSDK_SkeletonType skeletonType,
    const metaMovementSDK_SkeletonInitParams* initParams,
    metaMovementSDK_Handle* in_out_handle);

// NOTE: knownSourceJointNamesByID and knownTargetJointNamesByID are either
// nullptr, or assumed to be of length metaMovementSDK_KnownJointType::KnownJointCount
META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_createOrUpdateUtilityConfig(
    const char* configName,
    const metaMovementSDK_ConfigInitParams* initParams,
    metaMovementSDK_Handle* in_out_handle);
  
// NOTE: sourceHandle and in_out_handle can be the same. The result will
// be that the reversed mapping replaces the source mapping.
META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_createReverseMappingUtilityConfig(
    const char* reverseConfigName,
    metaMovementSDK_Handle sourceHandle,
    metaMovementSDK_Handle* in_out_handle);

/**********************************************************
 *
 *               Query Functions
 *
 **********************************************************/

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getCoordinateSpace(
    metaMovementSDK_CoordinateSpace* out_coordinateSpace);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getConfigName(
    metaMovementSDK_Handle handle,
    char* out_buffer,
    int* in_out_bufferSize);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getConfigRetargetingFlags(
    metaMovementSDK_Handle handle,
    metaMovementSDK_RetargetingBehaviorFlags* out_retargetingFlags);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getVersion(metaMovementSDK_Handle handle, double* version);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getSerializationVersion(double* version);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_isSerializationVersionMinSupported(double version);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getSkeletonInfo(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_SkeletonInfo* out_skeletonInfo);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getBlendShapeNames(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    char* out_buffer,
    int* in_out_bufferSize,
    const char** out_blendShapeNames,
    int* in_out_numBlendShapeNames);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getJointNames(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    char* out_buffer,
    int* in_out_bufferSize,
    const char** out_jointNames,
    int* in_out_numJointNames);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getJointNamesFromIndexList(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    const metaMovementSDK_JointIndex* in_jointIndexList,
    int in_jointIndexCount,
    char* out_buffer,
    int* in_out_bufferSize,
    const char** optional_out_jointNames);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getSkeletonTPose(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_SkeletonTPoseType tposeType,
    metaMovementSDK_JointRelativeSpaceType outJointSpaceType,
    metaMovementSDK_Transform* out_transformData,
    int* in_out_numJoints);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_calculateSkeletonTPose(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_JointRelativeSpaceType outJointSpaceType,
    float delta,
    metaMovementSDK_Transform* out_transformData,
    int* in_out_numJoints);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_calculateSkeletonTPoseAtHeight(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_JointRelativeSpaceType outJointSpaceType,
    float height,
    metaMovementSDK_Transform* out_transformData,
    int* in_out_numJoints);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getSkeletonMappings(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonTPoseType tposeType,
    metaMovementSDK_JointMapping* out_mappingData,
    int* in_out_numMappings);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getSkeletonMappingEntries(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonTPoseType tposeType,
    metaMovementSDK_JointMappingEntry* out_mappingEntryData,
    int* in_out_numMappingEntries);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getSkeletonMappingTargetJoints(
    metaMovementSDK_Handle handle,
    metaMovementSDK_JointIndex* out_jointIndexList,
    int* in_out_jointCount);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getAutoMappingAdditionalJointData(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    char* out_buffer,
    int* in_out_bufferSize,
    metaMovementSDK_AutoMappingJointData* out_jointData,
    int* in_out_numJoints);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getBlendShapeName(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_BlendShapeIndex blendShapeIndex,
    char* out_buffer,
    int* in_out_bufferSize);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getJointName(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_JointIndex jointIndex,
    char* out_buffer,
    int* in_out_bufferSize);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getJointIndex(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    const char* in_jointName,
    metaMovementSDK_JointIndex* out_jointIndex);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getJointIndexByKnownJointType(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_KnownJointType knownJointType,
    metaMovementSDK_JointIndex* out_jointIndex);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getKnownJointIndexes(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_KnownJointIndexData* out_knownJoints);

// Humanoid Limbs
META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getHumanoidLimbTypeFromJointName(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    const char* in_jointName,
    metaMovementSDK_HumanoidLimbType* out_humanoidLimbType);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getHumanoidLimbTypeFromJointIndex(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_JointIndex in_jointIndex,
    metaMovementSDK_HumanoidLimbType* out_humanoidLimbType);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getJointIndexesInHumanoidLimb(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_HumanoidLimbType humanoidLimbType,
    metaMovementSDK_JointIndex* out_jointIndexList,
    int* in_out_jointCount);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getParentJointIndex(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_JointIndex jointIndex,
    metaMovementSDK_JointIndex* out_parentJointIndex);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getParentJointIndexes(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_JointIndex* out_jointIndexArray,
    int* in_out_numJoints);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getChildJointIndexes(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_JointIndex parentJointIndex,
    metaMovementSDK_JointIndex* out_jointIndexArray,
    int* in_out_numJoints);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getManifestationNames(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    char* out_buffer,
    int* in_out_bufferSize,
    const char** out_manifestationNames,
    int* in_out_numManifestationNames);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getJointsInManifestation(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    const char* manifestationName,
    metaMovementSDK_JointIndex* out_jointIndexList,
    int* in_out_jointCount);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_convertPoseFromToManifestation(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    const char* sourceManifestationName, // nullptr or Empty string is no manifestation
    metaMovementSDK_JointRelativeSpaceType sourceJointSpaceType,
    const metaMovementSDK_Transform* sourceTransformData,
    int sourceManifestationJointCount,
    const char* outManifestationName, // nullptr or Empty string is no manifestation
    metaMovementSDK_JointRelativeSpaceType outJointSpaceType,
    metaMovementSDK_Transform* out_transformData,
    int* in_out_numOutputJoints);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getPoseInfo(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_SkeletonTPoseType poseType,
    metaMovementSDK_PoseInfo* out_poseInfo);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getLengthBetweenJoints(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_SkeletonTPoseType poseType,
    metaMovementSDK_JointIndex startJointIndex,
    metaMovementSDK_JointIndex endJointIndex,
    float* out_length);

/**********************************************************
 *
 *               Retargeting Functions
 *
 **********************************************************/

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_updateSourceReferenceTPose(
    metaMovementSDK_Handle handle,
    const metaMovementSDK_Transform* in_transformData,
    int in_numJoints,
    const char* sourceManifestation);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_retargetFromSourceFrameData(
    metaMovementSDK_Handle handle,
    metaMovementSDK_RetargetingBehaviorInfo in_retargetingBehaviorInfo,
    const metaMovementSDK_Transform* in_sourceTransformData,
    int in_numSourceJoints,
    metaMovementSDK_Transform* out_retargetedTargetTransformData,
    int* in_out_numTargetJoints,
    const char* sourceManifestation,
    const char* targetOutputManifestation);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getLastProcessedFramePose(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_JointRelativeSpaceType out_jointSpaceType,
    metaMovementSDK_Transform* out_transformData,
    int* in_out_numJoints,
    const char* manifestation);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_captureLastProcessedFramePoseToConfig(
    metaMovementSDK_Handle in_handle,
    metaMovementSDK_Handle* out_handle);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getLastRetargetedMappingData(
    metaMovementSDK_Handle handle,
    metaMovementSDK_JointIndex targetJointIndex,
    metaMovementSDK_SkeletonType* out_sourceSkeletonType,
    metaMovementSDK_Transform* out_tPoseBlendedTransform,
    metaMovementSDK_Transform* out_lastPoseBlendedTransform,
    metaMovementSDK_JointIndex* out_sourceJointIndexList,
    int* in_out_sourceJointCount);

// TODO: Separate Manifestion into purpose built conversions

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_matchCurrentTPose(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_MatchPoseBehavior matchBehavior,
    metaMovementSDK_JointRelativeSpaceType jointSpaceType,
    int jointCount,
    metaMovementSDK_Transform* in_out_transform);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_matchPose(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_MatchPoseBehavior matchBehavior,
    metaMovementSDK_JointRelativeSpaceType jointSpaceType,
    int jointCount,
    const metaMovementSDK_Transform* in_transformSourcePose,
    metaMovementSDK_Transform* in_out_transformData);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_scalePoseToHeight(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_JointRelativeSpaceType jointSpaceType,
    float height,
    int jointCount,
    metaMovementSDK_Transform* in_out_transformData);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_alignTargetToSource(
    const char* configName,
    metaMovementSDK_AlignmentFlags alignmentBehavior,
    metaMovementSDK_Handle sourceConfigHandle,
    metaMovementSDK_SkeletonType sourceConfigSkeletonType,
    metaMovementSDK_Handle targetConfigHandle,
    metaMovementSDK_Handle* in_out_handle);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_alignInputToSource(
    const char* configName,
    metaMovementSDK_AlignmentFlags alignmentBehavior,
    const metaMovementSDK_Transform* inputTargetSkeleton,
    int inputTargetSkeletonJointCount,
    metaMovementSDK_Handle sourceConfigHandle,
    metaMovementSDK_SkeletonType sourceConfigSkeletonType,
    metaMovementSDK_Handle targetConfigHandle,
    metaMovementSDK_Handle* in_out_handle);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_generateMappings(
    metaMovementSDK_Handle handle,
    metaMovementSDK_AutoMappingFlags autoMappingBehaviorFlags,
    const metaMovementSDK_AutoMappingJointData* additionalAutoMappingJointData,
    int additionalJointDataCount);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getTwistJoints(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_TwistJointDefinition* out_twistJointDefinitions,
    int* in_out_numTwistJoints);

/**********************************************************
 *
 *               Serialization Functions
 *
 **********************************************************/

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getSerializationSettings(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SerializationSettings* out_serializationSettings);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_setSerializationSettings(
    metaMovementSDK_Handle handle,
    const metaMovementSDK_SerializationSettings* in_serializationSettings);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getCurrentAck(metaMovementSDK_Handle handle, int* out_ack);

/**********************************************************
 *
 *               Snapshot Functions
 *
 **********************************************************/

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_buildSnapshot(
    metaMovementSDK_Handle handle,
    const metaMovementSDK_snapshotData* snapshotData);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_snapshotSkeleton(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    const metaMovementSDK_Transform* skeletonPose,
    const metaMovementSDK_JointIndex* skeletonIndices,
    int numOfSkeletonIndices);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_snapshotFace(
    metaMovementSDK_Handle handle,
    const float* facePose,
    const metaMovementSDK_BlendShapeIndex* faceIndices,
    int numOfFaceIndices);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_snapshotFrameData(
    metaMovementSDK_Handle handle,
    metaMovementSDK_frameData frameData,
    const metaMovementSDK_Transform* bindPose,
    int numBindPoseJoints);

/**********************************************************
 *
 *               Data Functions
 *
 **********************************************************/

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_serializeStartHeader(
    metaMovementSDK_startHeader startHeader,
    metaMovementSDK_startHeaderSerializedBytes* out_header_bytes);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_serializeEndHeader(
    metaMovementSDK_endHeader endHeader,
    metaMovementSDK_endHeaderBytes_* out_header_bytes);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_serializeSnapshot(
    metaMovementSDK_Handle handle,
    void* out_buffer,
    int* in_out_bufferSizeInBytes);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_deserializeStartHeader(
    metaMovementSDK_startHeader* startHeader,
    metaMovementSDK_startHeaderSerializedBytes in_header_bytes);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_deserializeEndHeader(
    metaMovementSDK_endHeader* endHeader,
    metaMovementSDK_endHeaderBytes_ in_header_bytes);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_deserializeSnapshotTimestamp(
    const void* snapshotInBytes,
    double* out_timestamp);

// TODO: We should pass in the transform count and blendshape count
// for the out_skeletonPose, out_facePose, and out_bodyTrackingPose to validate
// that the output buffers are large enough for the data we need to write.
META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_deserializeSnapshot(
    metaMovementSDK_Handle handle,
    const void* snapshotInBytes,
    double* out_timestamp,
    metaMovementSDK_CompressionType* out_compressionType,
    int* out_ack,
    metaMovementSDK_Transform* out_skeletonPose,
    float* out_facePose,
    metaMovementSDK_Transform* out_bodyTrackingPose,
    metaMovementSDK_frameData* out_framedata,
    metaMovementSDK_Transform* out_bindPose,
    int* out_numBindPoseJoints,
    metaMovementSDK_CoordinateSpace* coordinate_space_source,
    double dataVersion);

// TODO: eventually we should deprecate the function that requires multiple arguments.
META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_deserializeSnapshotData(
    metaMovementSDK_Handle handle,
    const void* snapshotInBytes,
    double dataVersion,
    metaMovementSDK_deserializedSnapshotData* out_deserializedSnapshotData);

/**********************************************************
 *
 *               Interpolator Functions
 *
 **********************************************************/

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getInterpolatedSkeletonPose(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    double time,
    metaMovementSDK_Transform* out_skeletonPose,
    int* in_out_jointCount);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getInterpolatedFacePose(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    double time,
    float* out_facePose,
    int* in_out_shapeCount);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getInterpolatedTrackerJointPose(
    metaMovementSDK_Handle handle,
    metaMovementSDK_TrackerJointType trackerJointType,
    double time,
    metaMovementSDK_Transform* out_pose);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_resetInterpolators(metaMovementSDK_Handle handle);

/**********************************************************
 *
 *               Tool and Data Functions
 *
 **********************************************************/

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_writeConfigDataToJSON(
    metaMovementSDK_Handle handle,
    const metaMovementSDK_CoordinateSpace* optional_coordinateSpace,
    const metaMovementSDK_JointRelativeSpaceType* optional_jointSpaceType,
    char* out_buffer,
    int* in_out_bufferSize);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_generateTargetToSourceJointPairsFromNames(
    metaMovementSDK_Handle sourceConfigHandle,
    metaMovementSDK_Handle targetConfigHandle,
    int numPairs,
    const metaMovementSDK_TargetToSourceNamedMappingPair* in_namedPairs,
    metaMovementSDK_TargetToSourceMappingPair* out_pairs);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_applyWorldSpaceCoordinateSpaceConversion(
    metaMovementSDK_CoordinateSpace in_coordinateSpace,
    metaMovementSDK_CoordinateSpace out_coordinateSpace,
    bool applyFBXandGLBModelFileJointRotationFixup,
    metaMovementSDK_Transform* in_out_worldSpaceTransformData,
    int jointCount);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_applyCoordinateSpaceConversion(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_JointRelativeSpaceType jointSpaceType,
    metaMovementSDK_CoordinateSpace in_coordinateSpace,
    metaMovementSDK_CoordinateSpace out_coordinateSpace,
    metaMovementSDK_Transform* in_out_transformData,
    int jointCount);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_convertJointPose(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_JointRelativeSpaceType in_jointSpaceType,
    metaMovementSDK_JointRelativeSpaceType out_jointSpaceType,
    metaMovementSDK_Transform* in_out_transformData,
    int jointCount);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_calculatePoseExtents(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_JointRelativeSpaceType in_jointSpaceType,
    const metaMovementSDK_Transform* in_transformData,
    int jointCount,
    metaMovementSDK_Extents* out_extents);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_identifyKnownJointIndexesFromRestPose(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    metaMovementSDK_KnownJointIndexData* out_knownJoints);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_getKnownJointNamesFromIndexData(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    const metaMovementSDK_KnownJointIndexData* knownJointIndexData,
    char* out_buffer,
    int* in_out_bufferSizeInBytes,
    metaMovementSDK_KnownJointNameData* out_knownJointNames);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_identifyJointsToIncludeInMappingUsingRestPose(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    char* out_buffer,
    int* in_out_bufferSizeInBytes,
    const char** optional_out_jointNames,
    int* optional_in_out_numJoints);

META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_identifyPossibleExcludeFromMappingUsingRestPose(
    metaMovementSDK_Handle handle,
    metaMovementSDK_SkeletonType skeletonType,
    char* out_buffer,
    int* in_out_bufferSizeInBytes,
    const char** optional_out_jointNames,
    int* optional_in_out_numJoints);

// Generated a Simple Mapping Config using a Target to Source Joints.
// Input:
// sourceConfigHandle: Handle for the loaded config containing the source T-Pose data
// targetJointCount: The number of joints in the target character
// targetJointNames: Target joint names in index order
// targetParentJointNames: Parent joint names in the same index order as above.
// sourceJointNamesMapping: Source joint names (from sourceConfig) in target index order.
// out_generatedConfigHandle: Handle variable to write the new generated config to.
//
// NOTE: Unparented and Unmapped joints can be communicated with either an empty string,
// or nullptr in the target joint index in the array.
META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_generateSimpleMappingConfig(
    metaMovementSDK_Handle sourceConfigHandle,
    int targetJointCount,
    const char** targetJointNames,
    const char** targetParentJointNames,
    const metaMovementSDK_Transform* targetRestPose,
    const char** sourceJointNamesMapping,
    const metaMovementSDK_Handle* out_generatedConfigHandle);

#ifdef OVR_INTERNAL_CODE
// Used to test OVR_INTERNAL_CODE. If working properly, this
// function will not be exposed if OVR_INTERNAL_CODE is disabled.
META_MOVEMENTSDK_EXPORT
metaMovementSDK_Result metaMovementSDK_hiddenFunc();
#endif

#ifdef __cplusplus
}
#endif

#endif // MetaMovementSDK_Utility_h
