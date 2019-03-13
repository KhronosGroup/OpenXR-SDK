/*
================================================================================================

Description	:	Vector, matrix and quaternion math.
Author		:	J.M.P. van Waveren
Date		:	12/10/2016
Language	:	C99
Format		:	Real tabs with the tab size equal to 4 spaces.
Copyright	:	Copyright (c) 2016 Oculus VR, LLC. All Rights reserved.


LICENSE
=======

Copyright (c) 2016 Oculus VR, LLC.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

DESCRIPTION
===========

All matrices are column-major.

Set one of the following defines to 1 before including this header file to
construct an appropriate projection matrix for a particular graphics API.

#define GRAPHICS_API_OPENGL		0
#define GRAPHICS_API_OPENGL_ES	0
#define GRAPHICS_API_VULKAN		0
#define GRAPHICS_API_D3D		0
#define GRAPHICS_API_METAL		0


INTERFACE
=========

ksVector2i
ksVector3i
ksVector4i
ksVector2f
ksVector3f
ksVector4f
ksQuatf
ksMatrix2x2f
ksMatrix2x3f
ksMatrix2x4f
ksMatrix3x2f
ksMatrix3x3f
ksMatrix3x4f
ksMatrix4x2f
ksMatrix4x3f
ksMatrix4x4f

static void ksVector3f_Set( ksVector3f * v, const float value );
static void ksVector3f_Add( ksVector3f * result, const ksVector3f * a, const ksVector3f * b );
static void ksVector3f_Sub( ksVector3f * result, const ksVector3f * a, const ksVector3f * b );
static void ksVector3f_Min( ksVector3f * result, const ksVector3f * a, const ksVector3f * b );
static void ksVector3f_Max( ksVector3f * result, const ksVector3f * a, const ksVector3f * b );
static void ksVector3f_Decay( ksVector3f * result, const ksVector3f * a, const float value );
static void ksVector3f_Lerp( ksVector3f * result, const ksVector3f * a, const ksVector3f * b, const float fraction );
static void ksVector3f_Normalize( ksVector3f * v );
static float ksVector3f_Length( const ksVector3f * v );

static void ksQuatf_Lerp( ksQuatf * result, const ksQuatf * a, const ksQuatf * b, const float fraction );

static void ksMatrix3x3f_CreateTransposeFromMatrix4x4f( ksMatrix3x3f * result, const ksMatrix4x4f * src );
static void ksMatrix3x4f_CreateFromMatrix4x4f( ksMatrix3x4f * result, const ksMatrix4x4f * src );

static void ksMatrix4x4f_CreateIdentity( ksMatrix4x4f * result );
static void ksMatrix4x4f_CreateTranslation( ksMatrix4x4f * result, const float x, const float y, const float z );
static void ksMatrix4x4f_CreateRotation( ksMatrix4x4f * result, const float degreesX, const float degreesY, const float degreesZ );
static void ksMatrix4x4f_CreateScale( ksMatrix4x4f * result, const float x, const float y, const float z );
static void ksMatrix4x4f_CreateTranslationRotationScale( ksMatrix4x4f * result, const ksVector3f * translation, const ksQuatf * rotation, const ksVector3f * scale );
static void ksMatrix4x4f_CreateProjection( ksMatrix4x4f * result, const float tanAngleLeft, const float tanAngleRight,
											const float tanAngleUp, float const tanAngleDown, const float nearZ, const float farZ );
static void ksMatrix4x4f_CreateProjectionFov( ksMatrix4x4f * result, const float fovDegreesLeft, const float fovDegreesRight,
											const float fovDegreeUp, const float fovDegreesDown, const float nearZ, const float farZ );
static void ksMatrix4x4f_CreateFromQuaternion( ksMatrix3x4f * result, const ksQuatf * src );
static void ksMatrix4x4f_CreateOffsetScaleForBounds( ksMatrix4x4f * result, const ksMatrix4x4f * matrix, const ksVector3f * mins, const ksVector3f * maxs );

static bool ksMatrix4x4f_IsAffine( const ksMatrix4x4f * matrix, const float epsilon );
static bool ksMatrix4x4f_IsOrthogonal( const ksMatrix4x4f * matrix, const float epsilon );
static bool ksMatrix4x4f_IsOrthonormal( const ksMatrix4x4f * matrix, const float epsilon );
static bool ksMatrix4x4f_IsHomogeneous( const ksMatrix4x4f * matrix, const float epsilon );

static void ksMatrix4x4f_GetTranslation( ksVector3f * result, const ksMatrix4x4f * src );
static void ksMatrix4x4f_GetRotation( ksQuatf * result, const ksMatrix4x4f * src );
static void ksMatrix4x4f_GetScale( ksVector3f * result, const ksMatrix4x4f * src );

static void ksMatrix4x4f_Multiply( ksMatrix4x4f * result, const ksMatrix4x4f * a, const ksMatrix4x4f * b );
static void ksMatrix4x4f_Transpose( ksMatrix4x4f * result, const ksMatrix4x4f * src );
static void ksMatrix4x4f_Invert( ksMatrix4x4f * result, const ksMatrix4x4f * src );
static void ksMatrix4x4f_InvertHomogeneous( ksMatrix4x4f * result, const ksMatrix4x4f * src );

static void ksMatrix4x4f_TransformVector3f( ksVector3f * result, const ksMatrix4x4f * m, const ksVector3f * v );
static void ksMatrix4x4f_TransformVector4f( ksVector4f * result, const ksMatrix4x4f * m, const ksVector4f * v );

static void ksMatrix4x4f_TransformBounds( ksVector3f * resultMins, ksVector3f * resultMaxs, const ksMatrix4x4f * matrix, const ksVector3f * mins, const ksVector3f * maxs );
static bool ksMatrix4x4f_CullBounds( const ksMatrix4x4f * mvp, const ksVector3f * mins, const ksVector3f * maxs );

================================================================================================
*/

#if !defined( KSALGEBRA_H )
#define KSALGEBRA_H

#include <math.h>
#include <stdbool.h>

#define MATH_PI				3.14159265358979323846f

#define DEFAULT_NEAR_Z		0.015625f		// exact floating point representation
#define INFINITE_FAR_Z		0.0f

// 2D integer vector
typedef struct
{
	int x;
	int y;
} ksVector2i;

// 3D integer vector
typedef struct
{
	int x;
	int y;
	int z;
} ksVector3i;

// 4D integer vector
typedef struct
{
	int x;
	int y;
	int z;
	int w;
} ksVector4i;

// 2D float vector
typedef struct
{
	float x;
	float y;
} ksVector2f;

// 3D float vector
typedef struct
{
	float x;
	float y;
	float z;
} ksVector3f;

// 4D float vector
typedef struct
{
	float x;
	float y;
	float z;
	float w;
} ksVector4f;

// Quaternion
typedef struct
{
	float x;
	float y;
	float z;
	float w;
} ksQuatf;

// Column-major 2x2 matrix
typedef struct
{
	float m[2][2];
} ksMatrix2x2f;

// Column-major 2x3 matrix
typedef struct
{
	float m[2][3];
} ksMatrix2x3f;

// Column-major 2x4 matrix
typedef struct
{
	float m[2][4];
} ksMatrix2x4f;

// Column-major 3x2 matrix
typedef struct
{
	float m[3][2];
} ksMatrix3x2f;

// Column-major 3x3 matrix
typedef struct
{
	float m[3][3];
} ksMatrix3x3f;

// Column-major 3x4 matrix
typedef struct
{
	float m[3][4];
} ksMatrix3x4f;

// Column-major 4x2 matrix
typedef struct
{
	float m[4][2];
} ksMatrix4x2f;

// Column-major 4x3 matrix
typedef struct
{
	float m[4][3];
} ksMatrix4x3f;

// Column-major 4x4 matrix
typedef struct
{
	float m[4][4];
} ksMatrix4x4f;

static const ksVector4f ksColorRed			= { 1.0f, 0.0f, 0.0f, 1.0f };
static const ksVector4f ksColorGreen		= { 0.0f, 1.0f, 0.0f, 1.0f };
static const ksVector4f ksColorBlue			= { 0.0f, 0.0f, 1.0f, 1.0f };
static const ksVector4f ksColorYellow		= { 1.0f, 1.0f, 0.0f, 1.0f };
static const ksVector4f ksColorPurple		= { 1.0f, 0.0f, 1.0f, 1.0f };
static const ksVector4f ksColorCyan			= { 0.0f, 1.0f, 1.0f, 1.0f };
static const ksVector4f ksColorLightGrey	= { 0.7f, 0.7f, 0.7f, 1.0f };
static const ksVector4f ksColorDarkGrey		= { 0.3f, 0.3f, 0.3f, 1.0f };

static float ksRcpSqrt( const float x )
{
	const float SMALLEST_NON_DENORMAL = 1.1754943508222875e-038f;	// ( 1U << 23 )
	const float rcp = ( x >= SMALLEST_NON_DENORMAL ) ? 1.0f / sqrtf( x ) : 1.0f;
	return rcp;
}

static void ksVector3f_Set( ksVector3f * v, const float value )
{
	v->x = value;
	v->y = value;
	v->z = value;
}

static void ksVector3f_Add( ksVector3f * result, const ksVector3f * a, const ksVector3f * b )
{
	result->x = a->x + b->x;
	result->y = a->y + b->y;
	result->z = a->z + b->z;
}

static void ksVector3f_Sub( ksVector3f * result, const ksVector3f * a, const ksVector3f * b )
{
	result->x = a->x - b->x;
	result->y = a->y - b->y;
	result->z = a->z - b->z;
}

static void ksVector3f_Min( ksVector3f * result, const ksVector3f * a, const ksVector3f * b )
{
	result->x = ( a->x < b->x ) ? a->x : b->x;
	result->y = ( a->y < b->y ) ? a->y : b->y;
	result->z = ( a->z < b->z ) ? a->z : b->z;
}

static void ksVector3f_Max( ksVector3f * result, const ksVector3f * a, const ksVector3f * b )
{
	result->x = ( a->x > b->x ) ? a->x : b->x;
	result->y = ( a->y > b->y ) ? a->y : b->y;
	result->z = ( a->z > b->z ) ? a->z : b->z;
}

static void ksVector3f_Decay( ksVector3f * result, const ksVector3f * a, const float value )
{
	result->x = ( fabsf( a->x ) > value ) ? ( ( a->x > 0.0f ) ? ( a->x - value ) : ( a->x + value ) ) : 0.0f;
	result->y = ( fabsf( a->y ) > value ) ? ( ( a->y > 0.0f ) ? ( a->y - value ) : ( a->y + value ) ) : 0.0f;
	result->z = ( fabsf( a->z ) > value ) ? ( ( a->z > 0.0f ) ? ( a->z - value ) : ( a->z + value ) ) : 0.0f;
}

static void ksVector3f_Lerp( ksVector3f * result, const ksVector3f * a, const ksVector3f * b, const float fraction )
{
	result->x = a->x + fraction * ( b->x - a->x );
	result->y = a->y + fraction * ( b->y - a->y );
	result->z = a->z + fraction * ( b->z - a->z );
}

static void ksVector3f_Normalize( ksVector3f * v )
{
	const float lengthRcp = ksRcpSqrt( v->x * v->x + v->y * v->y + v->z * v->z );
	v->x *= lengthRcp;
	v->y *= lengthRcp;
	v->z *= lengthRcp;
}

static float ksVector3f_Length( const ksVector3f * v )
{
	return sqrtf( v->x * v->x + v->y * v->y + v->z * v->z );
}

static void ksQuatf_Lerp( ksQuatf * result, const ksQuatf * a, const ksQuatf * b, const float fraction )
{
	const float s = a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
	const float fa = 1.0f - fraction;
	const float fb = ( s < 0.0f ) ? -fraction : fraction;
	const float x = a->x * fa + b->x * fb;
	const float y = a->y * fa + b->y * fb;
	const float z = a->z * fa + b->z * fb;
	const float w = a->w * fa + b->w * fb;
	const float lengthRcp = ksRcpSqrt( x * x + y * y + z * z + w * w );
	result->x = x * lengthRcp;
	result->y = y * lengthRcp;
	result->z = z * lengthRcp;
	result->w = w * lengthRcp;
}

static void ksMatrix3x3f_CreateTransposeFromMatrix4x4f( ksMatrix3x3f * result, const ksMatrix4x4f * src )
{
	result->m[0][0] = src->m[0][0];
	result->m[0][1] = src->m[1][0];
	result->m[0][2] = src->m[2][0];

	result->m[1][0] = src->m[0][1];
	result->m[1][1] = src->m[1][1];
	result->m[1][2] = src->m[2][1];

	result->m[2][0] = src->m[0][2];
	result->m[2][1] = src->m[1][2];
	result->m[2][2] = src->m[2][2];
}

static void ksMatrix3x4f_CreateFromMatrix4x4f( ksMatrix3x4f * result, const ksMatrix4x4f * src )
{
	result->m[0][0] = src->m[0][0];
	result->m[0][1] = src->m[1][0];
	result->m[0][2] = src->m[2][0];
	result->m[0][3] = src->m[3][0];
	result->m[1][0] = src->m[0][1];
	result->m[1][1] = src->m[1][1];
	result->m[1][2] = src->m[2][1];
	result->m[1][3] = src->m[3][1];
	result->m[2][0] = src->m[0][2];
	result->m[2][1] = src->m[1][2];
	result->m[2][2] = src->m[2][2];
	result->m[2][3] = src->m[3][2];
}

// Use left-multiplication to accumulate transformations.
static void ksMatrix4x4f_Multiply( ksMatrix4x4f * result, const ksMatrix4x4f * a, const ksMatrix4x4f * b )
{
	result->m[0][0] = a->m[0][0] * b->m[0][0] + a->m[1][0] * b->m[0][1] + a->m[2][0] * b->m[0][2] + a->m[3][0] * b->m[0][3];
	result->m[0][1] = a->m[0][1] * b->m[0][0] + a->m[1][1] * b->m[0][1] + a->m[2][1] * b->m[0][2] + a->m[3][1] * b->m[0][3];
	result->m[0][2] = a->m[0][2] * b->m[0][0] + a->m[1][2] * b->m[0][1] + a->m[2][2] * b->m[0][2] + a->m[3][2] * b->m[0][3];
	result->m[0][3] = a->m[0][3] * b->m[0][0] + a->m[1][3] * b->m[0][1] + a->m[2][3] * b->m[0][2] + a->m[3][3] * b->m[0][3];

	result->m[1][0] = a->m[0][0] * b->m[1][0] + a->m[1][0] * b->m[1][1] + a->m[2][0] * b->m[1][2] + a->m[3][0] * b->m[1][3];
	result->m[1][1] = a->m[0][1] * b->m[1][0] + a->m[1][1] * b->m[1][1] + a->m[2][1] * b->m[1][2] + a->m[3][1] * b->m[1][3];
	result->m[1][2] = a->m[0][2] * b->m[1][0] + a->m[1][2] * b->m[1][1] + a->m[2][2] * b->m[1][2] + a->m[3][2] * b->m[1][3];
	result->m[1][3] = a->m[0][3] * b->m[1][0] + a->m[1][3] * b->m[1][1] + a->m[2][3] * b->m[1][2] + a->m[3][3] * b->m[1][3];

	result->m[2][0] = a->m[0][0] * b->m[2][0] + a->m[1][0] * b->m[2][1] + a->m[2][0] * b->m[2][2] + a->m[3][0] * b->m[2][3];
	result->m[2][1] = a->m[0][1] * b->m[2][0] + a->m[1][1] * b->m[2][1] + a->m[2][1] * b->m[2][2] + a->m[3][1] * b->m[2][3];
	result->m[2][2] = a->m[0][2] * b->m[2][0] + a->m[1][2] * b->m[2][1] + a->m[2][2] * b->m[2][2] + a->m[3][2] * b->m[2][3];
	result->m[2][3] = a->m[0][3] * b->m[2][0] + a->m[1][3] * b->m[2][1] + a->m[2][3] * b->m[2][2] + a->m[3][3] * b->m[2][3];

	result->m[3][0] = a->m[0][0] * b->m[3][0] + a->m[1][0] * b->m[3][1] + a->m[2][0] * b->m[3][2] + a->m[3][0] * b->m[3][3];
	result->m[3][1] = a->m[0][1] * b->m[3][0] + a->m[1][1] * b->m[3][1] + a->m[2][1] * b->m[3][2] + a->m[3][1] * b->m[3][3];
	result->m[3][2] = a->m[0][2] * b->m[3][0] + a->m[1][2] * b->m[3][1] + a->m[2][2] * b->m[3][2] + a->m[3][2] * b->m[3][3];
	result->m[3][3] = a->m[0][3] * b->m[3][0] + a->m[1][3] * b->m[3][1] + a->m[2][3] * b->m[3][2] + a->m[3][3] * b->m[3][3];
}

// Creates the transpose of the given matrix.
static void ksMatrix4x4f_Transpose( ksMatrix4x4f * result, const ksMatrix4x4f * src )
{
	result->m[0][0] = src->m[0][0];
	result->m[0][1] = src->m[1][0];
	result->m[0][2] = src->m[2][0];
	result->m[0][3] = src->m[3][0];

	result->m[1][0] = src->m[0][1];
	result->m[1][1] = src->m[1][1];
	result->m[1][2] = src->m[2][1];
	result->m[1][3] = src->m[3][1];

	result->m[2][0] = src->m[0][2];
	result->m[2][1] = src->m[1][2];
	result->m[2][2] = src->m[2][2];
	result->m[2][3] = src->m[3][2];

	result->m[3][0] = src->m[0][3];
	result->m[3][1] = src->m[1][3];
	result->m[3][2] = src->m[2][3];
	result->m[3][3] = src->m[3][3];
}

// Returns a 3x3 minor of a 4x4 matrix.
static float ksMatrix4x4f_Minor( const ksMatrix4x4f * matrix, int r0, int r1, int r2, int c0, int c1, int c2 )
{
	return	matrix->m[r0][c0] * ( matrix->m[r1][c1] * matrix->m[r2][c2] - matrix->m[r2][c1] * matrix->m[r1][c2] ) -
			matrix->m[r0][c1] * ( matrix->m[r1][c0] * matrix->m[r2][c2] - matrix->m[r2][c0] * matrix->m[r1][c2] ) +
			matrix->m[r0][c2] * ( matrix->m[r1][c0] * matrix->m[r2][c1] - matrix->m[r2][c0] * matrix->m[r1][c1] );
}
 
// Calculates the inverse of a 4x4 matrix.
static void ksMatrix4x4f_Invert( ksMatrix4x4f * result, const ksMatrix4x4f * src )
{
	const float rcpDet = 1.0f / (	src->m[0][0] * ksMatrix4x4f_Minor( src, 1, 2, 3, 1, 2, 3 ) -
									src->m[0][1] * ksMatrix4x4f_Minor( src, 1, 2, 3, 0, 2, 3 ) +
									src->m[0][2] * ksMatrix4x4f_Minor( src, 1, 2, 3, 0, 1, 3 ) -
									src->m[0][3] * ksMatrix4x4f_Minor( src, 1, 2, 3, 0, 1, 2 ) );

	result->m[0][0] =  ksMatrix4x4f_Minor( src, 1, 2, 3, 1, 2, 3 ) * rcpDet;
	result->m[0][1] = -ksMatrix4x4f_Minor( src, 0, 2, 3, 1, 2, 3 ) * rcpDet;
	result->m[0][2] =  ksMatrix4x4f_Minor( src, 0, 1, 3, 1, 2, 3 ) * rcpDet;
	result->m[0][3] = -ksMatrix4x4f_Minor( src, 0, 1, 2, 1, 2, 3 ) * rcpDet;
	result->m[1][0] = -ksMatrix4x4f_Minor( src, 1, 2, 3, 0, 2, 3 ) * rcpDet;
	result->m[1][1] =  ksMatrix4x4f_Minor( src, 0, 2, 3, 0, 2, 3 ) * rcpDet;
	result->m[1][2] = -ksMatrix4x4f_Minor( src, 0, 1, 3, 0, 2, 3 ) * rcpDet;
	result->m[1][3] =  ksMatrix4x4f_Minor( src, 0, 1, 2, 0, 2, 3 ) * rcpDet;
	result->m[2][0] =  ksMatrix4x4f_Minor( src, 1, 2, 3, 0, 1, 3 ) * rcpDet;
	result->m[2][1] = -ksMatrix4x4f_Minor( src, 0, 2, 3, 0, 1, 3 ) * rcpDet;
	result->m[2][2] =  ksMatrix4x4f_Minor( src, 0, 1, 3, 0, 1, 3 ) * rcpDet;
	result->m[2][3] = -ksMatrix4x4f_Minor( src, 0, 1, 2, 0, 1, 3 ) * rcpDet;
	result->m[3][0] = -ksMatrix4x4f_Minor( src, 1, 2, 3, 0, 1, 2 ) * rcpDet;
	result->m[3][1] =  ksMatrix4x4f_Minor( src, 0, 2, 3, 0, 1, 2 ) * rcpDet;
	result->m[3][2] = -ksMatrix4x4f_Minor( src, 0, 1, 3, 0, 1, 2 ) * rcpDet;
	result->m[3][3] =  ksMatrix4x4f_Minor( src, 0, 1, 2, 0, 1, 2 ) * rcpDet;
}

// Calculates the inverse of a 4x4 homogeneous matrix.
static void ksMatrix4x4f_InvertHomogeneous( ksMatrix4x4f * result, const ksMatrix4x4f * src )
{
	result->m[0][0] = src->m[0][0];
	result->m[0][1] = src->m[1][0];
	result->m[0][2] = src->m[2][0];
	result->m[0][3] = 0.0f;
	result->m[1][0] = src->m[0][1];
	result->m[1][1] = src->m[1][1];
	result->m[1][2] = src->m[2][1];
	result->m[1][3] = 0.0f;
	result->m[2][0] = src->m[0][2];
	result->m[2][1] = src->m[1][2];
	result->m[2][2] = src->m[2][2];
	result->m[2][3] = 0.0f;
	result->m[3][0] = -( src->m[0][0] * src->m[3][0] + src->m[0][1] * src->m[3][1] + src->m[0][2] * src->m[3][2] );
	result->m[3][1] = -( src->m[1][0] * src->m[3][0] + src->m[1][1] * src->m[3][1] + src->m[1][2] * src->m[3][2] );
	result->m[3][2] = -( src->m[2][0] * src->m[3][0] + src->m[2][1] * src->m[3][1] + src->m[2][2] * src->m[3][2] );
	result->m[3][3] = 1.0f;
}

// Creates an identity matrix.
static void ksMatrix4x4f_CreateIdentity( ksMatrix4x4f * result )
{
	result->m[0][0] = 1.0f; result->m[0][1] = 0.0f; result->m[0][2] = 0.0f; result->m[0][3] = 0.0f;
	result->m[1][0] = 0.0f; result->m[1][1] = 1.0f; result->m[1][2] = 0.0f; result->m[1][3] = 0.0f;
	result->m[2][0] = 0.0f; result->m[2][1] = 0.0f; result->m[2][2] = 1.0f; result->m[2][3] = 0.0f;
	result->m[3][0] = 0.0f; result->m[3][1] = 0.0f; result->m[3][2] = 0.0f; result->m[3][3] = 1.0f;
}

// Creates a translation matrix.
static void ksMatrix4x4f_CreateTranslation( ksMatrix4x4f * result, const float x, const float y, const float z )
{
	result->m[0][0] = 1.0f; result->m[0][1] = 0.0f; result->m[0][2] = 0.0f; result->m[0][3] = 0.0f;
	result->m[1][0] = 0.0f; result->m[1][1] = 1.0f; result->m[1][2] = 0.0f; result->m[1][3] = 0.0f;
	result->m[2][0] = 0.0f; result->m[2][1] = 0.0f; result->m[2][2] = 1.0f; result->m[2][3] = 0.0f;
	result->m[3][0] =    x; result->m[3][1] =    y; result->m[3][2] =    z; result->m[3][3] = 1.0f;
}

// Creates a rotation matrix.
// If -Z=forward, +Y=up, +X=right, then degreesX=pitch, degreesY=yaw, degreesZ=roll.
static void ksMatrix4x4f_CreateRotation( ksMatrix4x4f * result, const float degreesX, const float degreesY, const float degreesZ )
{
	const float sinX = sinf( degreesX * ( MATH_PI / 180.0f ) );
	const float cosX = cosf( degreesX * ( MATH_PI / 180.0f ) );
	const ksMatrix4x4f rotationX =
	{ {
		{ 1,     0,    0, 0 },
		{ 0,  cosX, sinX, 0 },
		{ 0, -sinX, cosX, 0 },
		{ 0,     0,    0, 1 }
	} };
	const float sinY = sinf( degreesY * ( MATH_PI / 180.0f ) );
	const float cosY = cosf( degreesY * ( MATH_PI / 180.0f ) );
	const ksMatrix4x4f rotationY =
	{ {
		{ cosY, 0, -sinY, 0 },
		{    0, 1,     0, 0 },
		{ sinY, 0,  cosY, 0 },
		{    0, 0,     0, 1 }
	} };
	const float sinZ = sinf( degreesZ * ( MATH_PI / 180.0f ) );
	const float cosZ = cosf( degreesZ * ( MATH_PI / 180.0f ) );
	const ksMatrix4x4f rotationZ =
	{ {
		{  cosZ, sinZ, 0, 0 },
		{ -sinZ, cosZ, 0, 0 },
		{     0,    0, 1, 0 },
		{     0,    0, 0, 1 }
	} };
	ksMatrix4x4f rotationXY;
	ksMatrix4x4f_Multiply( &rotationXY, &rotationY, &rotationX );
	ksMatrix4x4f_Multiply( result, &rotationZ, &rotationXY );
}

// Creates a scale matrix.
static void ksMatrix4x4f_CreateScale( ksMatrix4x4f * result, const float x, const float y, const float z )
{
	result->m[0][0] =    x; result->m[0][1] = 0.0f; result->m[0][2] = 0.0f; result->m[0][3] = 0.0f;
	result->m[1][0] = 0.0f; result->m[1][1] =    y; result->m[1][2] = 0.0f; result->m[1][3] = 0.0f;
	result->m[2][0] = 0.0f; result->m[2][1] = 0.0f; result->m[2][2] =    z; result->m[2][3] = 0.0f;
	result->m[3][0] = 0.0f; result->m[3][1] = 0.0f; result->m[3][2] = 0.0f; result->m[3][3] = 1.0f;
}

// Creates a matrix from a quaternion.
static void ksMatrix4x4f_CreateFromQuaternion( ksMatrix4x4f * result, const ksQuatf * quat )
{
	const float x2 = quat->x + quat->x;
	const float y2 = quat->y + quat->y;
	const float z2 = quat->z + quat->z;

	const float xx2 = quat->x * x2;
	const float yy2 = quat->y * y2;
	const float zz2 = quat->z * z2;

	const float yz2 = quat->y * z2;
	const float wx2 = quat->w * x2;
	const float xy2 = quat->x * y2;
	const float wz2 = quat->w * z2;
	const float xz2 = quat->x * z2;
	const float wy2 = quat->w * y2;

	result->m[0][0] = 1.0f - yy2 - zz2;
	result->m[0][1] = xy2 + wz2;
	result->m[0][2] = xz2 - wy2;
	result->m[0][3] = 0.0f;

	result->m[1][0] = xy2 - wz2;
	result->m[1][1] = 1.0f - xx2 - zz2;
	result->m[1][2] = yz2 + wx2;
	result->m[1][3] = 0.0f;

	result->m[2][0] = xz2 + wy2;
	result->m[2][1] = yz2 - wx2;
	result->m[2][2] = 1.0f - xx2 - yy2;
	result->m[2][3] = 0.0f;

	result->m[3][0] = 0.0f;
	result->m[3][1] = 0.0f;
	result->m[3][2] = 0.0f;
	result->m[3][3] = 1.0f;
}

// Creates a combined translation(rotation(scale(object))) matrix.
static void ksMatrix4x4f_CreateTranslationRotationScale( ksMatrix4x4f * result, const ksVector3f * translation, const ksQuatf * rotation, const ksVector3f * scale )
{
	ksMatrix4x4f scaleMatrix;
	ksMatrix4x4f_CreateScale( &scaleMatrix, scale->x, scale->y, scale->z );

	ksMatrix4x4f rotationMatrix;
	ksMatrix4x4f_CreateFromQuaternion( &rotationMatrix, rotation );

	ksMatrix4x4f translationMatrix;
	ksMatrix4x4f_CreateTranslation( &translationMatrix, translation->x, translation->y, translation->z );

	ksMatrix4x4f combinedMatrix;
	ksMatrix4x4f_Multiply( &combinedMatrix, &rotationMatrix, &scaleMatrix );
	ksMatrix4x4f_Multiply( result, &translationMatrix, &combinedMatrix );
}

// Creates a projection matrix based on the specified dimensions.
// The projection matrix transforms -Z=forward, +Y=up, +X=right to the appropriate clip space for the graphics API.
// The far plane is placed at infinity if farZ <= nearZ.
// An infinite projection matrix is preferred for rasterization because, except for
// things *right* up against the near plane, it always provides better precision:
//		"Tightening the Precision of Perspective Rendering"
//		Paul Upchurch, Mathieu Desbrun
//		Journal of Graphics Tools, Volume 16, Issue 1, 2012
static void ksMatrix4x4f_CreateProjection( ksMatrix4x4f * result, const float tanAngleLeft, const float tanAngleRight,
											const float tanAngleUp, float const tanAngleDown, const float nearZ, const float farZ )
{
	const float tanAngleWidth = tanAngleRight - tanAngleLeft;

#if GRAPHICS_API_VULKAN == 1
	// Set to tanAngleDown - tanAngleUp for a clip space with positive Y down (Vulkan).
	const float tanAngleHeight = tanAngleDown - tanAngleUp;
#else
	// Set to tanAngleUp - tanAngleDown for a clip space with positive Y up (OpenGL / D3D / Metal).
	const float tanAngleHeight = tanAngleUp - tanAngleDown;
#endif

#if GRAPHICS_API_OPENGL == 1 || GRAPHICS_API_OPENGL_ES == 1
	// Set to nearZ for a [-1,1] Z clip space (OpenGL / OpenGL ES).
	const float offsetZ = nearZ;
#else
	// Set to zero for a [0,1] Z clip space (Vulkan / D3D / Metal).
	const float offsetZ = 0;
#endif

	if ( farZ <= nearZ )
	{
		// place the far plane at infinity
		result->m[0][0] = 2 / tanAngleWidth;
		result->m[1][0] = 0;
		result->m[2][0] = ( tanAngleRight + tanAngleLeft ) / tanAngleWidth;
		result->m[3][0] = 0;

		result->m[0][1] = 0;
		result->m[1][1] = 2 / tanAngleHeight;
		result->m[2][1] = ( tanAngleUp + tanAngleDown ) / tanAngleHeight;
		result->m[3][1] = 0;

		result->m[0][2] = 0;
		result->m[1][2] = 0;
		result->m[2][2] = -1;
		result->m[3][2] = -( nearZ + offsetZ );

		result->m[0][3] = 0;
		result->m[1][3] = 0;
		result->m[2][3] = -1;
		result->m[3][3] = 0;
	}
	else
	{
		// normal projection
		result->m[0][0] = 2 / tanAngleWidth;
		result->m[1][0] = 0;
		result->m[2][0] = ( tanAngleRight + tanAngleLeft ) / tanAngleWidth;
		result->m[3][0] = 0;

		result->m[0][1] = 0;
		result->m[1][1] = 2 / tanAngleHeight;
		result->m[2][1] = ( tanAngleUp + tanAngleDown ) / tanAngleHeight;
		result->m[3][1] = 0;

		result->m[0][2] = 0;
		result->m[1][2] = 0;
		result->m[2][2] = -( farZ + offsetZ ) / ( farZ - nearZ );
		result->m[3][2] = -( farZ * ( nearZ + offsetZ ) ) / ( farZ - nearZ );

		result->m[0][3] = 0;
		result->m[1][3] = 0;
		result->m[2][3] = -1;
		result->m[3][3] = 0;
	}
}

// Creates a projection matrix based on the specified FOV.
static void ksMatrix4x4f_CreateProjectionFov( ksMatrix4x4f * result, const float fovDegreesLeft, const float fovDegreesRight,
												const float fovDegreesUp, const float fovDegreesDown, const float nearZ, const float farZ )
{
	const float tanLeft = - tanf( fovDegreesLeft * ( MATH_PI / 180.0f ) );
	const float tanRight = tanf( fovDegreesRight * ( MATH_PI / 180.0f ) );

	const float tanDown = - tanf( fovDegreesDown * ( MATH_PI / 180.0f ) );
	const float tanUp = tanf( fovDegreesUp * ( MATH_PI / 180.0f ) );

	ksMatrix4x4f_CreateProjection( result, tanLeft, tanRight, tanUp, tanDown, nearZ, farZ );
}

// Creates a matrix that transforms the -1 to 1 cube to cover the given 'mins' and 'maxs' transformed with the given 'matrix'.
static void ksMatrix4x4f_CreateOffsetScaleForBounds( ksMatrix4x4f * result, const ksMatrix4x4f * matrix, const ksVector3f * mins, const ksVector3f * maxs )
{
	const ksVector3f offset = { ( maxs->x + mins->x ) * 0.5f, ( maxs->y + mins->y ) * 0.5f, ( maxs->z + mins->z ) * 0.5f };
	const ksVector3f scale = { ( maxs->x - mins->x ) * 0.5f, ( maxs->y - mins->y ) * 0.5f, ( maxs->z - mins->z ) * 0.5f };

	result->m[0][0] = matrix->m[0][0] * scale.x;
	result->m[0][1] = matrix->m[0][1] * scale.x;
	result->m[0][2] = matrix->m[0][2] * scale.x;
	result->m[0][3] = matrix->m[0][3] * scale.x;

	result->m[1][0] = matrix->m[1][0] * scale.y;
	result->m[1][1] = matrix->m[1][1] * scale.y;
	result->m[1][2] = matrix->m[1][2] * scale.y;
	result->m[1][3] = matrix->m[1][3] * scale.y;

	result->m[2][0] = matrix->m[2][0] * scale.z;
	result->m[2][1] = matrix->m[2][1] * scale.z;
	result->m[2][2] = matrix->m[2][2] * scale.z;
	result->m[2][3] = matrix->m[2][3] * scale.z;

	result->m[3][0] = matrix->m[3][0] + matrix->m[0][0] * offset.x + matrix->m[1][0] * offset.y + matrix->m[2][0] * offset.z;
	result->m[3][1] = matrix->m[3][1] + matrix->m[0][1] * offset.x + matrix->m[1][1] * offset.y + matrix->m[2][1] * offset.z;
	result->m[3][2] = matrix->m[3][2] + matrix->m[0][2] * offset.x + matrix->m[1][2] * offset.y + matrix->m[2][2] * offset.z;
	result->m[3][3] = matrix->m[3][3] + matrix->m[0][3] * offset.x + matrix->m[1][3] * offset.y + matrix->m[2][3] * offset.z;
}

// Returns true if the given matrix is affine.
static bool ksMatrix4x4f_IsAffine( const ksMatrix4x4f * matrix, const float epsilon )
{
	return	fabsf( matrix->m[0][3] ) <= epsilon &&
			fabsf( matrix->m[1][3] ) <= epsilon &&
			fabsf( matrix->m[2][3] ) <= epsilon &&
			fabsf( matrix->m[3][3] - 1.0f ) <= epsilon;
}

// Returns true if the given matrix is orthogonal.
static bool ksMatrix4x4f_IsOrthogonal( const ksMatrix4x4f * matrix, const float epsilon )
{
	for ( int i = 0; i < 3; i++ )
	{
		for ( int j = 0; j < 3; j++ )
		{
			if ( i != j )
			{
				if ( fabsf( matrix->m[i][0] * matrix->m[j][0] + matrix->m[i][1] * matrix->m[j][1] + matrix->m[i][2] * matrix->m[j][2] ) > epsilon )
				{
					return false;
				}
				if ( fabsf( matrix->m[0][i] * matrix->m[0][j] + matrix->m[1][i] * matrix->m[1][j] + matrix->m[2][i] * matrix->m[2][j] ) > epsilon )
				{
					return false;
				}
			}
		}
	}
	return true;
}

// Returns true if the given matrix is orthonormal.
static bool ksMatrix4x4f_IsOrthonormal( const ksMatrix4x4f * matrix, const float epsilon )
{
	for ( int i = 0; i < 3; i++ )
	{
		for ( int j = 0; j < 3; j++ )
		{
			const float kd = ( i == j ) ? 1.0f : 0.0f;	// Kronecker delta
			if ( fabsf( kd - ( matrix->m[i][0] * matrix->m[j][0] + matrix->m[i][1] * matrix->m[j][1] + matrix->m[i][2] * matrix->m[j][2] ) ) > epsilon )
			{
				return false;
			}
			if ( fabsf( kd - ( matrix->m[0][i] * matrix->m[0][j] + matrix->m[1][i] * matrix->m[1][j] + matrix->m[2][i] * matrix->m[2][j] ) ) > epsilon )
			{
				return false;
			}
		}
	}
	return true;
}

// Returns true if the given matrix is homogeneous.
static bool ksMatrix4x4f_IsHomogeneous( const ksMatrix4x4f * matrix, const float epsilon )
{
	return ksMatrix4x4f_IsAffine( matrix, epsilon ) && ksMatrix4x4f_IsOrthonormal( matrix, epsilon );
}

// Get the translation from a combined translation(rotation(scale(object))) matrix.
static void ksMatrix4x4f_GetTranslation( ksVector3f * result, const ksMatrix4x4f * src )
{
	assert( ksMatrix4x4f_IsAffine( src, 1e-4f ) );
	assert( ksMatrix4x4f_IsOrthogonal( src, 1e-4f ) );

	result->x = src->m[3][0];
	result->y = src->m[3][1];
	result->z = src->m[3][2];
}

// Get the rotation from a combined translation(rotation(scale(object))) matrix.
static void ksMatrix4x4f_GetRotation( ksQuatf * result, const ksMatrix4x4f * src )
{
	assert( ksMatrix4x4f_IsAffine( src, 1e-4f ) );
	assert( ksMatrix4x4f_IsOrthogonal( src, 1e-4f ) );

	const float rcpScaleX = ksRcpSqrt( src->m[0][0] * src->m[0][0] + src->m[0][1] * src->m[0][1] + src->m[0][2] * src->m[0][2] );
	const float rcpScaleY = ksRcpSqrt( src->m[1][0] * src->m[1][0] + src->m[1][1] * src->m[1][1] + src->m[1][2] * src->m[1][2] );
	const float rcpScaleZ = ksRcpSqrt( src->m[2][0] * src->m[2][0] + src->m[2][1] * src->m[2][1] + src->m[2][2] * src->m[2][2] );
	const float m[9] =
	{
		src->m[0][0] * rcpScaleX, src->m[0][1] * rcpScaleX, src->m[0][2] * rcpScaleX,
		src->m[1][0] * rcpScaleY, src->m[1][1] * rcpScaleY, src->m[1][2] * rcpScaleY,
		src->m[2][0] * rcpScaleZ, src->m[2][1] * rcpScaleZ, src->m[2][2] * rcpScaleZ
	};
	if ( m[0 * 3 + 0] + m[1 * 3 + 1] + m[2 * 3 + 2] > 0.0f )
	{
		float t = + m[0 * 3 + 0] + m[1 * 3 + 1] + m[2 * 3 + 2] + 1.0f;
		float s = ksRcpSqrt( t ) * 0.5f;
		result->w = s * t;
		result->z = ( m[0 * 3 + 1] - m[1 * 3 + 0] ) * s;
		result->y = ( m[2 * 3 + 0] - m[0 * 3 + 2] ) * s;
		result->x = ( m[1 * 3 + 2] - m[2 * 3 + 1] ) * s;
	}
	else if ( m[0 * 3 + 0] > m[1 * 3 + 1] && m[0 * 3 + 0] > m[2 * 3 + 2] )
	{
		float t = + m[0 * 3 + 0] - m[1 * 3 + 1] - m[2 * 3 + 2] + 1.0f;
		float s = ksRcpSqrt( t ) * 0.5f;
		result->x = s * t;
		result->y = ( m[0 * 3 + 1] + m[1 * 3 + 0] ) * s; 
		result->z = ( m[2 * 3 + 0] + m[0 * 3 + 2] ) * s;
		result->w = ( m[1 * 3 + 2] - m[2 * 3 + 1] ) * s;
	}
	else if ( m[1 * 3 + 1] > m[2 * 3 + 2] )
	{
		float t = - m[0 * 3 + 0] + m[1 * 3 + 1] - m[2 * 3 + 2] + 1.0f;
		float s = ksRcpSqrt( t ) * 0.5f;
		result->y = s * t;
		result->x = ( m[0 * 3 + 1] + m[1 * 3 + 0] ) * s;
		result->w = ( m[2 * 3 + 0] - m[0 * 3 + 2] ) * s;
		result->z = ( m[1 * 3 + 2] + m[2 * 3 + 1] ) * s;
	}
	else
	{
		float t = - m[0 * 3 + 0] - m[1 * 3 + 1] + m[2 * 3 + 2] + 1.0f;
		float s = ksRcpSqrt( t ) * 0.5f;
		result->z = s * t;
		result->w = ( m[0 * 3 + 1] - m[1 * 3 + 0] ) * s;
		result->x = ( m[2 * 3 + 0] + m[0 * 3 + 2] ) * s;
		result->y = ( m[1 * 3 + 2] + m[2 * 3 + 1] ) * s;
	}
}

// Get the scale from a combined translation(rotation(scale(object))) matrix.
static void ksMatrix4x4f_GetScale( ksVector3f * result, const ksMatrix4x4f * src )
{
	assert( ksMatrix4x4f_IsAffine( src, 1e-4f ) );
	assert( ksMatrix4x4f_IsOrthogonal( src, 1e-4f ) );

	result->x = sqrtf( src->m[0][0] * src->m[0][0] + src->m[0][1] * src->m[0][1] + src->m[0][2] * src->m[0][2] );
	result->y = sqrtf( src->m[1][0] * src->m[1][0] + src->m[1][1] * src->m[1][1] + src->m[1][2] * src->m[1][2] );
	result->z = sqrtf( src->m[2][0] * src->m[2][0] + src->m[2][1] * src->m[2][1] + src->m[2][2] * src->m[2][2] );
}

// Transforms a 3D vector.
static void ksMatrix4x4f_TransformVector3f( ksVector3f * result, const ksMatrix4x4f * m, const ksVector3f * v )
{
	const float w = m->m[0][3] * v->x + m->m[1][3] * v->y + m->m[2][3] * v->z + m->m[3][3];
	const float rcpW = 1.0f / w;
	result->x = ( m->m[0][0] * v->x + m->m[1][0] * v->y + m->m[2][0] * v->z + m->m[3][0] ) * rcpW;
	result->y = ( m->m[0][1] * v->x + m->m[1][1] * v->y + m->m[2][1] * v->z + m->m[3][1] ) * rcpW;
	result->z = ( m->m[0][2] * v->x + m->m[1][2] * v->y + m->m[2][2] * v->z + m->m[3][2] ) * rcpW;
}

// Transforms a 4D vector.
static void ksMatrix4x4f_TransformVector4f( ksVector4f * result, const ksMatrix4x4f * m, const ksVector4f * v )
{
	result->x = m->m[0][0] * v->x + m->m[1][0] * v->y + m->m[2][0] * v->z + m->m[3][0];
	result->y = m->m[0][1] * v->x + m->m[1][1] * v->y + m->m[2][1] * v->z + m->m[3][1];
	result->z = m->m[0][2] * v->x + m->m[1][2] * v->y + m->m[2][2] * v->z + m->m[3][2];
	result->w = m->m[0][3] * v->x + m->m[1][3] * v->y + m->m[2][3] * v->z + m->m[3][3];
}

// Transforms the 'mins' and 'maxs' bounds with the given 'matrix'.
static void ksMatrix4x4f_TransformBounds( ksVector3f * resultMins, ksVector3f * resultMaxs, const ksMatrix4x4f * matrix, const ksVector3f * mins, const ksVector3f * maxs )
{
	assert( ksMatrix4x4f_IsAffine( matrix, 1e-4f ) );

	const ksVector3f center = { ( mins->x + maxs->x ) * 0.5f, ( mins->y + maxs->y ) * 0.5f, ( mins->z + maxs->z ) * 0.5f };
	const ksVector3f extents = { maxs->x - center.x, maxs->y - center.y, maxs->z - center.z };
	const ksVector3f newCenter =
	{
		matrix->m[0][0] * center.x + matrix->m[1][0] * center.y + matrix->m[2][0] * center.z + matrix->m[3][0],
		matrix->m[0][1] * center.x + matrix->m[1][1] * center.y + matrix->m[2][1] * center.z + matrix->m[3][1],
		matrix->m[0][2] * center.x + matrix->m[1][2] * center.y + matrix->m[2][2] * center.z + matrix->m[3][2]
	};
	const ksVector3f newExtents =
	{
		fabsf( extents.x * matrix->m[0][0] ) + fabsf( extents.y * matrix->m[1][0] ) + fabsf( extents.z * matrix->m[2][0] ),
		fabsf( extents.x * matrix->m[0][1] ) + fabsf( extents.y * matrix->m[1][1] ) + fabsf( extents.z * matrix->m[2][1] ),
		fabsf( extents.x * matrix->m[0][2] ) + fabsf( extents.y * matrix->m[1][2] ) + fabsf( extents.z * matrix->m[2][2] )
	};
	ksVector3f_Sub( resultMins, &newCenter, &newExtents );
	ksVector3f_Add( resultMaxs, &newCenter, &newExtents );
}

// Returns true if the 'mins' and 'maxs' bounds is completely off to one side of the projection matrix.
static bool ksMatrix4x4f_CullBounds( const ksMatrix4x4f * mvp, const ksVector3f * mins, const ksVector3f * maxs )
{
	if ( maxs->x <= mins->x && maxs->y <= mins->y && maxs->z <= mins->z )
	{
		return false;
	}

	ksVector4f c[8];
	for ( int i = 0; i < 8; i++ )
	{
		const ksVector4f corner =
		{
			( i & 1 ) ? maxs->x : mins->x,
			( i & 2 ) ? maxs->y : mins->y,
			( i & 4 ) ? maxs->z : mins->z,
			1.0f
		};
		ksMatrix4x4f_TransformVector4f( &c[i], mvp, &corner );
	}

	int i;
	for ( i = 0; i < 8; i++ )
	{
		if ( c[i].x > -c[i].w )
		{
			break;
		}
	}
	if ( i == 8 )
	{
		return true;
	}
	for ( i = 0; i < 8; i++ )
	{
		if ( c[i].x < c[i].w )
		{
			break;
		}
	}
	if ( i == 8 )
	{
		return true;
	}

	for ( i = 0; i < 8; i++ )
	{
		if ( c[i].y > -c[i].w )
		{
			break;
		}
	}
	if ( i == 8 )
	{
		return true;
	}
	for ( i = 0; i < 8; i++ )
	{
		if ( c[i].y < c[i].w )
		{
			break;
		}
	}
	if ( i == 8 )
	{
		return true;
	}
	for ( i = 0; i < 8; i++ )
	{
		if ( c[i].z > -c[i].w )
		{
			break;
		}
	}
	if ( i == 8 )
	{
		return true;
	}
	for ( i = 0; i < 8; i++ )
	{
		if ( c[i].z < c[i].w )
		{
			break;
		}
	}
	if ( i == 8 )
	{
		return true;
	}

	return false;
}

#endif // !KSALGEBRA_H
