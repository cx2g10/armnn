﻿//
// Copyright © 2017 Arm Ltd. All rights reserved.
// See LICENSE file in the project root for full license information.
//
#include "ArmComputeTensorUtils.hpp"
#include "ArmComputeUtils.hpp"

#include <armnn/Descriptors.hpp>

namespace armnn
{
namespace armcomputetensorutils
{

arm_compute::DataType GetArmComputeDataType(armnn::DataType dataType)
{
    switch(dataType)
    {
        case armnn::DataType::Float32:
        {
            return arm_compute::DataType::F32;
        }
        case armnn::DataType::QuantisedAsymm8:
        {
            return arm_compute::DataType::QASYMM8;
        }
        case armnn::DataType::Signed32:
        {
            return arm_compute::DataType::S32;
        }
        default:
        {
            BOOST_ASSERT_MSG(false, "Unknown data type");
            return arm_compute::DataType::UNKNOWN;
        }
    }
}

arm_compute::TensorShape BuildArmComputeTensorShape(const armnn::TensorShape& tensorShape)
{
    arm_compute::TensorShape shape;

    // armnn tensors are (batch, channels, height, width)
    // arm_compute tensors are (width, height, channels, batch)
    for (unsigned int i = 0; i < tensorShape.GetNumDimensions(); i++)
    {
        // note that our dimensions are stored in the opposite order to ACL's
        shape.set(tensorShape.GetNumDimensions() - i - 1, tensorShape[i]);

        // TensorShape::set() flattens leading ones, so that batch size 1 cannot happen.
        // arm_compute tensors expect this
    }

    // prevent arm_compute issue where tensor is flattened to nothing
    if (shape.num_dimensions() == 0)
    {
        shape.set_num_dimensions(1);
    }

    return shape;
}

// Utility function used to build a TensorInfo object, that can be used to initialise
// ARM Compute Tensor and CLTensor allocators.
arm_compute::TensorInfo BuildArmComputeTensorInfo(const armnn::TensorInfo& tensorInfo)
{
    const arm_compute::TensorShape aclTensorShape = BuildArmComputeTensorShape(tensorInfo.GetShape());
    const arm_compute::DataType aclDataType = GetArmComputeDataType(tensorInfo.GetDataType());
    const arm_compute::QuantizationInfo aclQuantizationInfo(tensorInfo.GetQuantizationScale(),
                                                            tensorInfo.GetQuantizationOffset());

    return arm_compute::TensorInfo(aclTensorShape, 1, aclDataType, aclQuantizationInfo);
}

arm_compute::PoolingLayerInfo BuildArmComputePoolingLayerInfo(const Pooling2dDescriptor& descriptor)
{
    using arm_compute::PoolingType;
    using arm_compute::DimensionRoundingType;
    using arm_compute::PadStrideInfo;
    using arm_compute::PoolingLayerInfo;

    // Resolve ARM Compute layer parameters
    const PoolingType poolingType = ConvertPoolingAlgorithmToAclPoolingType(descriptor.m_PoolType);
    const DimensionRoundingType rounding = ConvertOutputShapeRoundingToAclDimensionRoundingType(
                                                                                    descriptor.m_OutputShapeRounding);

    const PadStrideInfo padStrideInfo(descriptor.m_StrideX,
                                      descriptor.m_StrideY,
                                      descriptor.m_PadLeft,
                                      descriptor.m_PadRight,
                                      descriptor.m_PadTop,
                                      descriptor.m_PadBottom,
                                      rounding);

    const bool excludePadding = (descriptor.m_PaddingMethod == PaddingMethod::Exclude);

    return arm_compute::PoolingLayerInfo(poolingType, descriptor.m_PoolWidth, padStrideInfo, excludePadding);
}

arm_compute::NormalizationLayerInfo BuildArmComputeNormalizationLayerInfo(const NormalizationDescriptor& descriptor)
{
    const arm_compute::NormType normType =
        ConvertNormalizationAlgorithmChannelToAclNormType(descriptor.m_NormChannelType);
    return arm_compute::NormalizationLayerInfo(normType,
                                               descriptor.m_NormSize,
                                               descriptor.m_Alpha,
                                               descriptor.m_Beta,
                                               descriptor.m_K,
                                               false);
}

arm_compute::PermutationVector BuildArmComputePermutationVector(const armnn::PermutationVector& perm)
{
    arm_compute::PermutationVector aclPerm;

    unsigned int start = 0;
    while ((start == perm[start]) && (start < perm.GetSize()))
    {
        ++start;
    }

    for (unsigned int i = start; i < perm.GetSize(); ++i)
    {
        aclPerm.set(i - start, perm[i] - start);
    }

    return aclPerm;
}

} // namespace armcomputetensorutils
} // namespace armnn
