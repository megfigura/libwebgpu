#pragma once

enum class GLDataType
{
    SBYTE = 5120,
    UBYTE = 5121,
    SSHORT = 5122,
    USHORT = 5123,
    UINT = 5125,
    FLOAT = 5126
};

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