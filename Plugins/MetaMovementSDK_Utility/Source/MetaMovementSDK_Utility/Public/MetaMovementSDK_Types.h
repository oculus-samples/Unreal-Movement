// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

// C-Style format must be maintained for C compatibility
// @nolint

#ifndef MetaMovementSDK_Types_h
#define MetaMovementSDK_Types_h

#include <cstdint>

#ifndef META_MOVEMENTSDK_EXPORT
#if defined(_MSC_VER)
#define META_MOVEMENTSDK_EXPORT __declspec(dllexport)
#define META_MOVEMENTSDK_HIDDEN
#else
#define META_MOVEMENTSDK_EXPORT __attribute__((visibility("default")))
#define META_MOVEMENTSDK_HIDDEN __attribute__((visibility("hidden")))
#endif
#endif

#if defined ANDROID || defined __linux__
#define __cdecl
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ENUM_FORCE_32BIT(_prefix) _prefix##_Force32Bit = 0x7FFFFFFF

typedef enum {
  MetaMovementSDKLogLevel_DEBUG = 0,
  MetaMovementSDKLogLevel_INFO = 1,
  MetaMovementSDKLogLevel_WARN = 2,
  MetaMovementSDKLogLevel_ERROR = 3,

  MetaMovementSDKLogLevel_COUNT,

  ENUM_FORCE_32BIT(metaMovementSDK_LogLevel)
} metaMovementSDK_LogLevel;

// clang-format off: Use typedef for CAPI Exposure (C Compat)
typedef void (*metaMovementSDK_LogCallback)(metaMovementSDK_LogLevel logLevel, const char* message);

typedef unsigned long long metaMovementSDK_Handle;
typedef int metaMovementSDK_JointIndex;
typedef int metaMovementSDK_BlendShapeIndex;
// clang-format on

static const metaMovementSDK_Handle META_MOVEMENTSDK_INVALID_HANDLE = 0u;
static const metaMovementSDK_JointIndex META_MOVEMENTSDK_INVALID_JOINT_INDEX = -1;
static const metaMovementSDK_BlendShapeIndex META_MOVEMENTSDK_INVALID_BLENDSHAPE_INDEX = -1;

#define IS_VALID_META_MOVEMENTSDK_HANDLE(_handle) (_handle != META_MOVEMENTSDK_INVALID_HANDLE)
#define IS_VALID_META_MOVEMENTSDK_JOINT(_jointIdx) \
  ((_jointIdx) != META_MOVEMENTSDK_INVALID_JOINT_INDEX)
#define IS_VALID_META_MOVEMENTSDK_BLENDSHAPE(_blendShapeIdx) \
  ((_blendShapeIdx) != META_MOVEMENTSDK_INVALID_BLENDSHAPE_INDEX)

#define IS_ROOT_SPACE_TYPE(_spaceType)                                                \
  ((_spaceType) == metaMovementSDK_JointRelativeSpaceType::RootOriginRelativeSpace || \
   (_spaceType) == metaMovementSDK_JointRelativeSpaceType::RootOriginRelativeWithJointScale)

#define IS_LOCAL_SPACE_TYPE(_spaceType)                                  \
  ((_spaceType) == metaMovementSDK_JointRelativeSpaceType::LocalSpace || \
   (_spaceType) == metaMovementSDK_JointRelativeSpaceType::LocalSpaceScaled)

#define IS_SCALED_SPACE_TYPE(_spaceType)                                         \
    ((_spaceType) == metaMovementSDK_JointRelativeSpaceType::LocalSpaceScaled || \
    (_spaceType) == metaMovementSDK_JointRelativeSpaceType::RootOriginRelativeWithJointScale)

// Serialization Global Defines

// if data serialization is running at 12.5 fps,
// 800 frames is a reasonable cap for 60 seconds.
static const int META_MOVEMENTSDK_SERIALIZATION_MAX_POSSIBLE_SNAPSHOTS = 800;
static const int META_MOVEMENTSDK_SERIALIZATION_MIN_POSSIBLE_SNAPSHOTS = 254;

// clang-format off: Use typedef for CAPI Exposure (C Compat)
typedef struct metaMovementSDK_Vector3f_ {
  float x;
  float y;
  float z;
} metaMovementSDK_Vector3f;

typedef struct metaMovementSDK_Quatf_ {
  float x;
  float y;
  float z;
  float w;
} metaMovementSDK_Quatf;

typedef struct metaMovementSDK_Transform_ {
  metaMovementSDK_Quatf orientation;
  metaMovementSDK_Vector3f position;
  metaMovementSDK_Vector3f scale;
} metaMovementSDK_Transform;

typedef struct metaMovementSDK_CoordinateSpace_ {
  metaMovementSDK_Vector3f up;
  metaMovementSDK_Vector3f forward;
  metaMovementSDK_Vector3f right;
  float metersToUnitScale;
} metaMovementSDK_CoordinateSpace;

static const metaMovementSDK_Vector3f META_MOVEMENTSDK_ZERO_VECTOR = { 0.0f, 0.0f, 0.0f };
static const metaMovementSDK_Vector3f META_MOVEMENTSDK_ONE_VECTOR = { 1.0f, 1.0f, 1.0f };
static const metaMovementSDK_Quatf META_MOVEMENTSDK_IDENTITY_QUAT = { 0.0f, 0.0f, 0.0f, 1.0f };
static const metaMovementSDK_Transform META_MOVEMENTSDK_IDENTITY_TRANSFORM = {
    META_MOVEMENTSDK_IDENTITY_QUAT,
    META_MOVEMENTSDK_ZERO_VECTOR,
    META_MOVEMENTSDK_ONE_VECTOR };

// Default coordinate space set to Z-Forward, Y-Up, LH (Matches Unity space).
// Used as default - engine integration can override using Initialize function.
static const metaMovementSDK_CoordinateSpace META_MOVEMENTSDK_DEFAULT_COORDINATE_SPACE = {
    {0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f},
    {1.0f, 0.0f, 0.0f},
    1.0f
};

typedef enum {
  Failure = 0,

  // Specific failures.
  Failure_ConfigNull = -1000,
  Failure_ConfigCannotParse = -1001,
  Failure_ConfigInvalid = -1002,
  Failure_HandleInvalid = -1003,
  Failure_Initialization = -1004,
  Failure_InsufficientSize = -1005,
  Failure_WriteOutputNull = -1006,
  Failure_RequiredParameterNull = -1007,
  Failure_InvalidData = -1008,

  // Success.
  Success = 1,

  ENUM_FORCE_32BIT(metaMovementSDK_Result)
} metaMovementSDK_Result;

typedef enum {
  NoAlignment = 0u,
  ReorientToSourceFacing = 1u << 0u,
  LimbRotations = 1u << 1u,
  HandAndFingerRotations = 1u << 2u,
  ProportionalScalingToHeight = 1u << 3u,
  LimbDeformationMatchSourceProportion = 1u << 4u,
  MatchHandAndFingerPoseWithDeformation = 1u << 5u,

  ENUM_FORCE_32BIT(metaMovementSDK_AlignmentFlags)
} metaMovementSDK_AlignmentFlags;

static const metaMovementSDK_AlignmentFlags META_MOVEMENTSDK_ALIGN_TO_REST_POSE =
  static_cast<metaMovementSDK_AlignmentFlags>(
      metaMovementSDK_AlignmentFlags::ReorientToSourceFacing |
      metaMovementSDK_AlignmentFlags::LimbRotations |
      metaMovementSDK_AlignmentFlags::HandAndFingerRotations);

static const metaMovementSDK_AlignmentFlags META_MOVEMENTSDK_PROPORTIONAL_ALIGNMENT =
  static_cast<metaMovementSDK_AlignmentFlags>(
      META_MOVEMENTSDK_ALIGN_TO_REST_POSE |
      metaMovementSDK_AlignmentFlags::ProportionalScalingToHeight);

static const metaMovementSDK_AlignmentFlags META_MOVEMENTSDK_DEFORMATION_PRESERVE_HAND_PROPORTION =
  static_cast<metaMovementSDK_AlignmentFlags>(
      META_MOVEMENTSDK_PROPORTIONAL_ALIGNMENT |
      metaMovementSDK_AlignmentFlags::LimbDeformationMatchSourceProportion);

static const metaMovementSDK_AlignmentFlags META_MOVEMENTSDK_FULL_ALIGNMENT_WITH_DEFORMATION =
  static_cast<metaMovementSDK_AlignmentFlags>(
      META_MOVEMENTSDK_DEFORMATION_PRESERVE_HAND_PROPORTION |
      metaMovementSDK_AlignmentFlags::MatchHandAndFingerPoseWithDeformation);

typedef enum {
  AutoMapEmptyFlag = 0u,
  AutoMapSkipTwistJoints = 1u << 0u,

  ENUM_FORCE_32BIT(metaMovementSDK_AutoMappingFlags)
} metaMovementSDK_AutoMappingFlags;

typedef enum {
  AutoMapEmptyJointFlag = 0u,
  AutoMapJointFlagExclude = 1u << 0u,
  AutoMapJointFlagExcludeFromTwistMappings = 1u << 1u,

  ENUM_FORCE_32BIT(metaMovementSDK_AutoMappingJointFlags)
} metaMovementSDK_AutoMappingJointFlags;

// NOTE: DO NOT EVER CHANGE THE ORDER OR REMOVE JOINTS
// FROM THIS ENUM.
// If a new joint id is added, functions calling
// metaMovementSDK_createOrUpdateUtilityConfig could be affected
typedef enum {
  UnknownJoint = -1,
  Root = 0,
  Hips,
  RightUpperArm,
  LeftUpperArm,
  RightWrist,
  LeftWrist,
  Chest,
  Neck,
  RightUpperLeg,
  LeftUpperLeg,
  RightAnkle,
  LeftAnkle,

  KnownJointCount,

  ENUM_FORCE_32BIT(metaMovementSDK_KnownJointType)
} metaMovementSDK_KnownJointType;

// NOTE: Order MUST MATCH metaMovementSDK_KnownJointType enum
static const char* META_MOVEMENTSDK_KNOWN_JOINT_NAMES[metaMovementSDK_KnownJointType::KnownJointCount] = {
    "root",
    "hips",
    "rightUpperArm",
    "leftUpperArm",
    "rightWrist",
    "leftWrist",
    "chest",
    "neck",
    "rightUpperLeg",
    "leftUpperLeg",
    "rightAnkle",
    "leftAnkle",
};

// Humanoid Limbs are different from KnownJoints
// Limbs are used to encapsulate sets of joints based
// on the known joints defined by the skeleton.
typedef enum {
    UnknownHumanoidLimb = -1, // Catch All value
    RootToHipLimb = 0, // Root to Hip
    SpineAndTorsoLimb, // Hip to Chest
    ChestToNeckLimb, // Chest to Neck
    HeadAndFaceLimb, // Neck (Head) and all other child joints
    LeftArmToHandLimb, // Chest to Left Wrist
    RightArmToHandLimb, // Chest to Right Wrist
    LeftLegToFootLimb, // Hip to Left Leg and Ankle
    RightLegToFootLimb, // Hip to Right Leg and Ankle
    LeftHandLimb, // Wrist through all fingers
    RightHandLimb, // Wrist through all fingers
    LeftFootLimb, // Ankle and all child joints
    RightFootLimb, // Anke and all child joints

    HumanoidLimbCount,

    ENUM_FORCE_32BIT(metaMovementSDK_HumanoidLimbType)
} metaMovementSDK_HumanoidLimbType;

// Defines for left/right limbs
#define IS_MOVEMENTSDK_HUMANOID_LIMB_UNKNOWN(_limb) (_limb == metaMovementSDK_HumanoidLimbType::UnknownHumanoidLimb)
#define IS_MOVEMENTSDK_HUMANOID_LIMB_ARM(_limb) \
  (_limb == metaMovementSDK_HumanoidLimbType::RightArmToHandLimb || _limb == metaMovementSDK_HumanoidLimbType::LeftArmToHandLimb)
#define IS_MOVEMENTSDK_HUMANOID_LIMB_HAND(_limb) \
  (_limb == metaMovementSDK_HumanoidLimbType::RightHandLimb || _limb == metaMovementSDK_HumanoidLimbType::LeftHandLimb)
#define IS_MOVEMENTSDK_HUMANOID_LIMB_LEG(_limb) \
  (_limb == metaMovementSDK_HumanoidLimbType::RightLegToFootLimb || _limb == metaMovementSDK_HumanoidLimbType::LeftLegToFootLimb)
#define IS_MOVEMENTSDK_HUMANOID_LIMB_FOOT(_limb) \
  (_limb == metaMovementSDK_HumanoidLimbType::RightFootLimb || _limb == metaMovementSDK_HumanoidLimbType::LeftFootLimb)

static const metaMovementSDK_HumanoidLimbType META_MOVEMENTSDK_HUMANOID_LIMB_PARENT_HIERARCHY[metaMovementSDK_HumanoidLimbType::HumanoidLimbCount] = {
    metaMovementSDK_HumanoidLimbType::UnknownHumanoidLimb, // RootToHipLimb
    metaMovementSDK_HumanoidLimbType::RootToHipLimb,  // SpineAndTorsoLimb
    metaMovementSDK_HumanoidLimbType::SpineAndTorsoLimb, // ChestToNeckLimb
    metaMovementSDK_HumanoidLimbType::ChestToNeckLimb, // HeadAndFaceLimb
    metaMovementSDK_HumanoidLimbType::SpineAndTorsoLimb, // LeftArmToHandLimb
    metaMovementSDK_HumanoidLimbType::SpineAndTorsoLimb, // RightArmToHandLimb
    metaMovementSDK_HumanoidLimbType::RootToHipLimb, // LeftLegToFootLimb
    metaMovementSDK_HumanoidLimbType::RootToHipLimb, // RightLegToFootLimb
    metaMovementSDK_HumanoidLimbType::LeftArmToHandLimb, // LeftHandLimb
    metaMovementSDK_HumanoidLimbType::RightArmToHandLimb, // RightHandLimb
    metaMovementSDK_HumanoidLimbType::LeftLegToFootLimb, // LeftFootLimb
    metaMovementSDK_HumanoidLimbType::RightLegToFootLimb // RightFootLimb
};

typedef enum {
  SourceSkeleton = 0,
  TargetSkeleton,

  SkeletonTypeCount,

  ENUM_FORCE_32BIT(metaMovementSDK_SkeletonType)
} metaMovementSDK_SkeletonType;

// NOTE: Order MUST MATCH metaMovementSDK_SkeletonType enum
static const char* META_MOVEMENTSDK_SKELETON_TYPES[metaMovementSDK_SkeletonType::SkeletonTypeCount] = {
    "SourceSkeleton",
    "TargetSkeleton",
};


typedef enum {
  SkeletonFlag_None = 0u,  
  SkeletonFlag_NoRotationCorrectionOnCoordConversion = 1u << 0u,

  ENUM_FORCE_32BIT(metaMovementSDK_SkeletonFlags)
} metaMovementSDK_SkeletonFlags;

typedef enum {
  CenterEye = 0,
  LeftInput,

  RightInput,
  ENUM_FORCE_32BIT(metaMovementSDK_TrackerJointType)
} metaMovementSDK_TrackerJointType;

typedef enum
{
  Normal = 0,
  Twist,
  ChildAlignedTwist,
  IKDerived,

  JointMappingBehaviorTypeCount,
  ENUM_FORCE_32BIT(metaMovementSDK_JointMappingBehaviorType)
} metaMovementSDK_JointMappingBehaviorType;

#define IS_TWIST_MSDK_JOINT_MAPPING_BEHAVIOR(_type) \
  (_type == metaMovementSDK_JointMappingBehaviorType::Twist || \
  _type == metaMovementSDK_JointMappingBehaviorType::ChildAlignedTwist)

typedef enum {
  CurrentTPose = 0,
  MinTPose,
  MaxTPose,
  UnscaledTPose,
  ConfigCachedPose,

  SkeletonTPoseTypeCount,
  ENUM_FORCE_32BIT(metaMovementSDK_SkeletonTPoseType)
} metaMovementSDK_SkeletonTPoseType;

typedef enum {
  RootOriginRelativeSpace = 0, // Tracking Origin
  LocalSpace,
  RootOriginRelativeWithJointScale,
  LocalSpaceScaled,

  JointRelativeSpaceTypeCount,
  ENUM_FORCE_32BIT(metaMovementSDK_JointRelativeSpaceType)
} metaMovementSDK_JointRelativeSpaceType;

// NOTE: Order MUST MATCH metaMovementSDK_JointRelativeSpaceType enum
static const char* META_MOVEMENTSDK_JOINT_RELATIVE_SPACE_TYPES[metaMovementSDK_JointRelativeSpaceType::JointRelativeSpaceTypeCount] = {
    "RootOriginRelative",
    "LocalSpace",
    "RootOriginRelativeWithJointScale",
    "LocalSpaceScaled",
};

typedef enum {
  MatchScale = 0,
  MatchOrientation,

  MatchPoseBehaviorCount,
  ENUM_FORCE_32BIT(metaMovementSDK_MatchPoseBehavior)
} metaMovementSDK_MatchPoseBehavior;

typedef struct metaMovementSDK_TargetToSourceMappingPair_ {
  metaMovementSDK_JointIndex targetJointIndex;
  metaMovementSDK_JointIndex sourceJointIndex;
} metaMovementSDK_TargetToSourceMappingPair;

typedef struct metaMovementSDK_TargetToSourceNamedMappingPair_ {
  const char* targetJointName;
  const char* sourceJointName;
} metaMovementSDK_TargetToSourceNamedMappingPair;

typedef struct metaMovementSDK_TwistJointDefinition_ {
  metaMovementSDK_JointIndex twistJointIndex;
  metaMovementSDK_JointIndex influenceStartJointIndex;
  metaMovementSDK_JointIndex influenceEndJointIndex;
  float ratio;
} metaMovementSDK_TwistJointDefinition;

typedef struct metaMovementSDK_JointMappingEntry_ {
  metaMovementSDK_JointIndex jointIndex;
  float rotationWeight;
  float positionWeight;
} metaMovementSDK_JointMappingEntry;

typedef struct metaMovementSDK_JointMapping_ {
  metaMovementSDK_JointIndex jointIndex;
  metaMovementSDK_SkeletonType skeletonType;
  metaMovementSDK_JointMappingBehaviorType behavior;
  int numOfEntries;
} metaMovementSDK_JointMapping;

typedef struct metaMovementSDK_JointMappingDefinition_ {
  int mappingsCount;
  int mappingEntryCount;
  const metaMovementSDK_JointMapping* mappings;
  const metaMovementSDK_JointMappingEntry* mappingEntries;
} metaMovementSDK_JointMappingDefinition;

typedef struct metaMovementSDK_SkeletonInfo_ {
  metaMovementSDK_SkeletonType skeletonType;
  metaMovementSDK_SkeletonFlags skeletonFlags;
  int jointCount;
  int blendShapeCount;
} metaMovementSDK_SkeletonInfo;

// AutoMapping Exclusion joints in the init params
typedef struct metaMovementSDK_AutoMappingJointData_ {
  const char* jointName;
  metaMovementSDK_AutoMappingJointFlags flags;
} metaMovementSDK_AutoMappingJointData;

typedef struct metaMovementSDK_Extents_ {
    metaMovementSDK_Vector3f min;
    metaMovementSDK_Vector3f max;
    metaMovementSDK_Vector3f range;
} metaMovementSDK_Extents;

typedef struct metaMovementSDK_PoseInfo_ {
    metaMovementSDK_CoordinateSpace coordinateSpace;
    metaMovementSDK_Extents extents;
    metaMovementSDK_Vector3f knownJointPositions[metaMovementSDK_KnownJointType::KnownJointCount];
} metaMovementSDK_PoseInfo;

typedef struct metaMovementSDK_KnownJointIndexData_ {
    metaMovementSDK_JointIndex jointIndexByType[metaMovementSDK_KnownJointType::KnownJointCount];
} metaMovementSDK_KnownJointIndexData;

typedef struct metaMovementSDK_KnownJointNameData_ {
    const char* jointNameByType[metaMovementSDK_KnownJointType::KnownJointCount];
} metaMovementSDK_KnownJointNameData;

/**********************************************************
 *
 *               Retargeting Enums/Structures
 *
 **********************************************************/

typedef enum {
    RetargetingBehaviorFlagEmpty = 0,
    ApplyJointOrientationFixup = 1u << 0u,
    UseTPoseForJointScale = 1u << 1u,
    ApplyMSDKSourceHandBugFixup = 1u << 2u,

    ENUM_FORCE_32BIT(metaMovementSDK_RetargetingBehaviorFlags)
} metaMovementSDK_RetargetingBehaviorFlags;

typedef enum {
    RetargetingRuntimeFlagEmpty = 0,
    RetargetingRuntimeForceOverrideBehaviorFlags = 1u << 0u,
    RetargetingRuntimeUnsetConfigFlagsWithMask = 1u << 1u,
    RetargetingRuntimeSkipTargetToTargetMappings = 1u << 2u,

    ENUM_FORCE_32BIT(metaMovementSDK_RetargetingRuntimeFlags)
} metaMovementSDK_RetargetingRuntimeFlags;

typedef enum {
    RotationsAndPositions = 0,
    RotationsAndPositionsHandsRotationOnly,
    RotationOnlyUniformScale,
    RotationOnlyNoScaling,

    RetargetingBehaviorCount,

    ENUM_FORCE_32BIT(metaMovementSDK_RetargetingBehavior)
} metaMovementSDK_RetargetingBehavior;

#define IS_ROTATION_ONLY_RETARGETING_BEHAVIOR(_behavior) \
    (_behavior == metaMovementSDK_RetargetingBehavior::RotationOnlyUniformScale || \
    _behavior == metaMovementSDK_RetargetingBehavior::RotationOnlyNoScaling)

typedef enum {
    CombineHipRotationIntoRoot = 0,
    RootFlatTranslationFullHipRotation,
    ZeroOutAllRootTranslationAndHipYaw,

    RetargetingRootMotionBehaviorCount,
    ENUM_FORCE_32BIT(metaMovementSDK_RetargetingRootMotionBehavior)
} metaMovementSDK_RetargetingRootMotionBehavior;

// Struct to package behavior enums together for Frame processing
typedef struct metaMovementSDK_RetargetingBehaviorInfo_ {
    // Value determines how the joints are represented after
    // retargeted (Local vs Root Origin/Tracking releative)
    metaMovementSDK_JointRelativeSpaceType targetOutputJointSpaceType;

    // Retargeting Behavior to apply (Positions/Rotations, scaling, etc)
    metaMovementSDK_RetargetingBehavior retargetingBehavior;

    // Root Motion behavior (Where the linear/angular velocity
    // should be stored, and how)
    metaMovementSDK_RetargetingRootMotionBehavior rootMotionBehavior;

    // Behavior Flags
    metaMovementSDK_RetargetingBehaviorFlags behaviorFlags;

    // Runtime Flags
    metaMovementSDK_RetargetingRuntimeFlags runtimeFlags;
} metaMovementSDK_RetargetingBehaviorInfo;

static const metaMovementSDK_RetargetingBehaviorInfo META_MOVEMENTSDK_DEFAULT_RETARGETING_SETTINGS = {
  metaMovementSDK_JointRelativeSpaceType::RootOriginRelativeSpace,
  metaMovementSDK_RetargetingBehavior::RotationsAndPositions,
  metaMovementSDK_RetargetingRootMotionBehavior::CombineHipRotationIntoRoot,
  static_cast<metaMovementSDK_RetargetingBehaviorFlags>(
      metaMovementSDK_RetargetingBehaviorFlags::ApplyJointOrientationFixup),
  static_cast<metaMovementSDK_RetargetingRuntimeFlags>(
      metaMovementSDK_RetargetingRuntimeFlags::RetargetingRuntimeFlagEmpty)
};

/**********************************************************
*
*               Initialization Structures
*
**********************************************************/

typedef struct metaMovementSDK_SkeletonInitParams_ {
  int blendShapeCount;
  int jointCount;
  const char** blendShapeNames;
  const char** jointNames;
  const char** parentJointNames;
  const metaMovementSDK_Transform* minTPose;
  const metaMovementSDK_Transform* maxTPose;
  const metaMovementSDK_Transform* unscaledTPose;

  // Coordinate Space Data
  metaMovementSDK_JointRelativeSpaceType jointSpaceType;

  const char** optional_knownSourceJointNamesById;

  int optional_autoMapJointDataCount;
  const metaMovementSDK_AutoMappingJointData* optional_autoMapJointData;

  // Number of manifestations in the name and joint counts arrays
  int  optional_manifestationCount;
  const char** optional_manifestationNames;
  // Array of integers
  int* optional_manifestationJointCounts;
  // Buffer of all Manifestation joint names in order of manifestations
  const char** optional_manifestationJointNames;

  // Flags
  metaMovementSDK_SkeletonFlags optional_skeletonFlags;
} metaMovementSDK_SkeletonInitParams;

typedef struct metaMovementSDK_ConfigInitParams_ {
  // Skeleton Data
  metaMovementSDK_SkeletonInitParams sourceSkeleton;
  metaMovementSDK_SkeletonInitParams targetSkeleton;

  // Mapping Data
  metaMovementSDK_JointMappingDefinition minMappings;
  metaMovementSDK_JointMappingDefinition maxMappings;

  // Optional config Data
  metaMovementSDK_RetargetingBehaviorFlags optional_retargetingFlags;
} metaMovementSDK_ConfigInitParams;

static const metaMovementSDK_SkeletonInitParams META_MOVEMENTSDK_EMPTY_SKELETON_INIT_PARAMS = {
  0, // blendShapeCount
  0, // jointCount
  nullptr, // blendShapeNames
  nullptr, // jointNames
  nullptr, // parentJointNames
  nullptr, // minTPose
  nullptr, // maxTPose
  nullptr, // unscaledTPose
  metaMovementSDK_JointRelativeSpaceType::RootOriginRelativeSpace, // jointSpaceType
  nullptr, // optional_knownSourceJointNamesById
  0, // optional_autoMapExcludedJointCount
  nullptr, // optional_autoMapExcludedJointNames
  0, // optional_manifestationCount
  nullptr, // optional_manifestationNames
  nullptr, // optional_manifestationJointCounts
  nullptr, // optional_manifestationJointNames
  metaMovementSDK_SkeletonFlags::SkeletonFlag_None, // optional_skeletonFlags
};

static const metaMovementSDK_JointMappingDefinition META_MOVEMENTSDK_EMPTY_JOINT_MAPPING_DEFINITION = {
   0, // mappingsCount
   0, // mappingEntryCount
   nullptr, // mappings
   nullptr, // mappingEntries
};

static const metaMovementSDK_ConfigInitParams META_MOVEMENTSDK_EMPTY_CONFIG_INIT_PARAMS = {
  META_MOVEMENTSDK_EMPTY_SKELETON_INIT_PARAMS, // sourceSkeleton
  META_MOVEMENTSDK_EMPTY_SKELETON_INIT_PARAMS, // targetSkeleton
  META_MOVEMENTSDK_EMPTY_JOINT_MAPPING_DEFINITION, // minMappings
  META_MOVEMENTSDK_EMPTY_JOINT_MAPPING_DEFINITION, // maxMappings
  metaMovementSDK_RetargetingBehaviorFlags::RetargetingBehaviorFlagEmpty, // optional_retargetingFlags
};

/**********************************************************
 *
 *               Serialization Enums/Structures
 *
 **********************************************************/

/// <summary>
/// Specifies different compression types.
/// </summary>
typedef enum {
  High = 0, // Compressed with joint lengths.
  Medium = 1, // 2^0. Joint compression, positions use less space by using small vector types.
  Low = 2, // 2^1. Joint compression, positions use more space by using large vector types.
  CompressionTypeCount,

  ENUM_FORCE_32BIT(metaMovementSDK_CompressionType)
} metaMovementSDK_CompressionType;

// NOTE: Order MUST MATCH metaMovementSDK_CompressionType enum
static const char* META_MOVEMENTSDK_COMPRESSION_TYPES[metaMovementSDK_CompressionType::CompressionTypeCount] = {
    "High",
    "Medium",
    "Low",
};

#define COMPRESSION_USES_LEN_COMPRESSION(_compressionType) ((_compressionType) == metaMovementSDK_CompressionType::High)

typedef struct metaMovementSDK_SerializationSettings_ {
    metaMovementSDK_CompressionType compressionType;

    float positionThreshold;
    float rotationAngleThresholdDegrees;
    float shapeThreshold;

    int numberOfSnapshots;
} metaMovementSDK_SerializationSettings;

// frame data useful for data recording
typedef struct metaMovementSDK_frameData_ {
  uint8_t bodyTrackingFidelity;
  double timestamp;
  bool isValid;
  float confidence;
  uint8_t jointSet;
  uint8_t calibrationState;
  bool isUsingHandsLeft;
  bool isUsingHandsRight;
  uint32_t skeletonChangeCount;

  metaMovementSDK_Transform leftInput;
  metaMovementSDK_Transform rightInput;
  metaMovementSDK_Transform centerEye;
} metaMovementSDK_frameData;

typedef struct metaMovementSDK_snapshotData_ {
    int baselineAck;
    double timestamp;

    const metaMovementSDK_Transform* targetSkeletonPose;
    const int* targetSkeletonIndices;
    int numTargetSkeletonIndices;

    const metaMovementSDK_Transform* sourceSkeletonPose;
    const int* sourceSkeletonIndices;
    int numSourceSkeletonIndices;

    const float* facePose;
    const int* faceIndices;
    int numOfFaceIndices;

    bool serializeFrameData;
    metaMovementSDK_frameData frameData;
    const metaMovementSDK_Transform* bindPose;
    int numBindPoseJoints;

    metaMovementSDK_CoordinateSpace coordinateSpaceSource;
} metaMovementSDK_snapshotData;

typedef struct metaMovementSDK_deserializedSnapshotData_ {
    double timestamp;
    metaMovementSDK_CompressionType compressionType;
    int ack;
    metaMovementSDK_Transform* targetSkeletonPose;
    float* facePose;
    metaMovementSDK_Transform* sourceSkeletonPose;
    metaMovementSDK_frameData frameData;
    metaMovementSDK_Transform* bindPose;
    int numBindPoseJoints;
    metaMovementSDK_CoordinateSpace coordinateSpaceSource;
} metaMovementSDK_deserializedSnapshotData;

static const double META_MOVEMENT_SDK_SERIALIZATION_VERSION_CURRENT = 0.04;
static const double META_MOVEMENT_SDK_SERIALIZATION_VERSION_MIN_SUPPORTED = 0.02;

// 256 bits or 32 characters should suffice for most strings.
static const int META_MOVEMENTSDK_SERIALIZATION_START_HEADER_STRING_SIZE_BYTES = 32;
// in serialized form, we will store:
// 1. One double for version (64 bits) + 
// 1. Four 32-byte strings (1024 bits) +
// 2. 64-bits for UTC +
// 3. 16-bits int for num frames +
// 4. 24-bits int for num total snapshot bytes +
// 5. 24-bits for for compressed start network time +
// 6. 10-bits int for num buffered snapshots
// The minimum space necessary for this data would be 1226 bits. But we use a bit more space
// to allocate to word boundaries (1248).
// TODO: find out why we need to align to word boundary?
static const int META_MOVEMENTSDK_SERIALIZATION_START_HEADER_SIZE_BITS = 1248;
static const int META_MOVEMENTSDK_SERIALIZATION_START_HEADER_NUM_INT_BITS = 16;
static const int META_MOVEMENTSDK_SERIALIZATION_START_HEADER_NUM_BIG_INT_BITS = 24;
static const int META_MOVEMENTSDK_SERIALIZATION_START_HEADER_NETWORK_TIME_BITS = 24;
static const int META_MOVEMENTSDK_SERIALIZATION_START_HEADER_NUM_SMALL_INT_BITS = 10;

typedef struct metaMovementSDK_startHeader_ {
  double dataVersion;
  char osVersion[META_MOVEMENTSDK_SERIALIZATION_START_HEADER_STRING_SIZE_BYTES];
  char gameEngineVersion[META_MOVEMENTSDK_SERIALIZATION_START_HEADER_STRING_SIZE_BYTES];
  char bundleID[META_MOVEMENTSDK_SERIALIZATION_START_HEADER_STRING_SIZE_BYTES];
  char metaXRSDKVersion[META_MOVEMENTSDK_SERIALIZATION_START_HEADER_STRING_SIZE_BYTES];
  uint64_t utcTimestamp;
  uint32_t numSnapshots;
  uint32_t numTotalSnapshotBytes;
  double startNetworkTime;
  uint32_t numBufferedSnapshots;
} metaMovementSDK_startHeader;
static const int META_MOVEMENTSDK_SERIALIZATION_START_HEADER_SIZE_BYTES = 156;

typedef struct metaMovementSDK_startHeaderSerializedBytes_ {
  char serializedBytes[META_MOVEMENTSDK_SERIALIZATION_START_HEADER_SIZE_BYTES];
} metaMovementSDK_startHeaderSerializedBytes;

// one long long will be stored in serialized form
static const int META_MOVEMENTSDK_SERIALIZATION_END_HEADER_STRING_SIZE_BITS = 64;

typedef struct metaMovementSDK_endHeader_ {
  uint64_t utcTimestamp;
} metaMovementSDK_endHeader;

static const int META_MOVEMENTSDK_SERIALIZATION_END_HEADER_STRING_SIZE_BYTES = 8;

typedef struct metaMovementSDK_endHeaderBytes_ {
  char serializedBytes[META_MOVEMENTSDK_SERIALIZATION_END_HEADER_STRING_SIZE_BYTES];
} metaMovementSDK_endHeaderBytes;

// clang-format on

#ifdef __cplusplus
}
#endif

#endif // MetaMovementSDK_Types_h
