#pragma once

namespace webgpu
{
    enum class GLDataType
    {
        SBYTE = 5120,
        UBYTE = 5121,
        SSHORT = 5122,
        USHORT = 5123,
        UINT = 5125,
        FLOAT = 5126
    };

    inline int GLDataTypeSize(GLDataType type)
    {
        switch (type)
        {
            case GLDataType::SBYTE:
            case GLDataType::UBYTE:
                return 1;

            case GLDataType::SSHORT:
            case GLDataType::USHORT:
                return 2;

            case GLDataType::UINT:
            case GLDataType::FLOAT:
                return 4;
        }

        return -1;
    }

    enum class GLPrimitiveMode
    {
        POINTS = 0,
        LINES = 1,
        LINE_LOOP = 2,
        LINE_STRIP = 3,
        TRIANGLES = 4,
        TRIANGLE_STRIP = 5,
        TRIANGLE_FAN = 6,
    };

    enum class GLBufferType
    {
        ARRAY_BUFFER = 34962,
        ELEMENT_ARRAY_BUFFER = 34963
    };

    enum class GLMagFilter
    {
        NEAREST = 9728,
        LINEAR = 9729
    };

    enum class GLMinFilter
    {
        NEAREST = 9728,
        LINEAR = 9729,
        NEAREST_MIPMAP_NEAREST = 9984,
        LINEAR_MIPMAP_NEAREST = 9985,
        NEAREST_MIPMAP_LINEAR = 9986,
        LINEAR_MIPMAP_LINEAR = 9987
    };

    enum class GLWrapMode
    {
        CLAMP_TO_EDGE = 33071,
        MIRRORED_REPEAT = 33648,
        REPEAT = 10497
    };

    enum class GLAccessorType
    {
        SCALAR,
        VEC2,
        VEC3,
        VEC4,
        MAT2,
        MAT3,
        MAT4,
        string
    };

    inline int GLAccessorTypeSize(GLAccessorType accessorType)
    {
        switch (accessorType)
        {
            case GLAccessorType::SCALAR:
                return 1;

            case GLAccessorType::VEC2:
                return 2;

            case GLAccessorType::VEC3:
                return 3;

            case GLAccessorType::VEC4:
            case GLAccessorType::MAT2:
                return 4;

            case GLAccessorType::MAT3:
                return 9;

            case GLAccessorType::MAT4:
                return 16;

            default:
                break;
        }

        return -1;
    }

    inline int GLElementSize(GLDataType dataType, GLAccessorType accessorType)
    {
        int dataTypeSize = GLDataTypeSize(dataType);
        int accessorTypeSize = GLAccessorTypeSize(accessorType);
        return dataTypeSize * accessorTypeSize;
    }
}
