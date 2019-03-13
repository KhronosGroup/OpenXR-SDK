/*
================================================================================================

Description	:	Metal format properties and conversion from OpenGL.
Author		:	J.M.P. van Waveren
Date		:	12/16/2016
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

This header implements several support routines to convert OpenGL formats/types
to Metal formats. These routines are particularly useful for loading file
formats that store OpenGL formats/types such as KTX and glTF.

The functions in this header file convert the format, internalFormat and type
that are used as parameters to the following OpenGL functions:

void glTexImage2D( GLenum target, GLint level, GLint internalFormat,
	GLsizei width, GLsizei height, GLint border,
	GLenum format, GLenum type, const GLvoid * data );
void glTexImage3D( GLenum target, GLint level, GLint internalFormat,
	GLsizei width, GLsizei height, GLsizei depth, GLint border,
	GLenum format, GLenum type, const GLvoid * data );
void glCompressedTexImage2D( GLenum target, GLint level, GLenum internalformat,
	GLsizei width, GLsizei height, GLint border,
	GLsizei imageSize, const GLvoid * data );
void glCompressedTexImage3D( GLenum target, GLint level, GLenum internalformat,
	GLsizei width, GLsizei height, GLsizei depth, GLint border,
	GLsizei imageSize, const GLvoid * data );
void glTexStorage2D( GLenum target, GLsizei levels, GLenum internalformat,
	GLsizei width, GLsizei height );
void glTexStorage3D( GLenum target, GLsizei levels, GLenum internalformat,
	GLsizei width, GLsizei height, GLsizei depth );
void glVertexAttribPointer( GLuint index, GLint size, GLenum type, GLboolean normalized,
	GLsizei stride, const GLvoid * pointer);


IMPLEMENTATION
==============

This file does not include OpenGL / OpenGL ES headers because:

  1. Including OpenGL / OpenGL ES headers is platform dependent and
     may require a separate installation of an OpenGL SDK.
  2. The OpenGL format/type constants are the same between extensions and core.
  3. The OpenGL format/type constants are the same between OpenGL and OpenGL ES.
  4. File formats like KTX and glTF may use OpenGL formats and types that
     are not supported by the OpenGL implementation on the platform but are
     supported by the Metal implementation.


ENTRY POINTS
============

static inline MTLPixelFormat mtlGetFormatFromOpenGLFormat( const GLenum format, const GLenum type );
static inline MTLPixelFormat mtlGetFormatFromOpenGLType( const GLenum type, const GLuint numComponents, const GLboolean normalized );
static inline MTLPixelFormat mtlGetFormatFromOpenGLInternalFormat( const GLenum internalFormat );
static inline void mtlGetFormatSize( const MTLPixelFormat format, MtlFormatSize * pFormatSize );

================================================================================================
*/

#if !defined( MTL_FORMAT_H )
#define MTL_FORMAT_H

#include "../GL/gl_format.h"

static inline MTLPixelFormat mtlGetFormatFromOpenGLFormat( const GLenum format, const GLenum type )
{
	switch ( type )
	{
		//
		// 8 bits per component
		//
		case GL_UNSIGNED_BYTE:
		{
			switch ( format )
			{
				case GL_RED:					return MTLPixelFormatR8Unorm;
				case GL_RG:						return MTLPixelFormatRG8Unorm;
				case GL_RGB:					return MTLPixelFormatInvalid;
				case GL_BGR:					return MTLPixelFormatInvalid;
				case GL_RGBA:					return MTLPixelFormatRGBA8Unorm;
				case GL_BGRA:					return MTLPixelFormatBGRA8Unorm;
				case GL_RED_INTEGER:			return MTLPixelFormatR8Uint;
				case GL_RG_INTEGER:				return MTLPixelFormatRGBA8Uint;
				case GL_RGB_INTEGER:			return MTLPixelFormatInvalid;
				case GL_BGR_INTEGER:			return MTLPixelFormatInvalid;
				case GL_RGBA_INTEGER:			return MTLPixelFormatRGBA8Uint;
				case GL_BGRA_INTEGER:			return MTLPixelFormatInvalid;
				case GL_STENCIL_INDEX:			return MTLPixelFormatStencil8;
				case GL_DEPTH_COMPONENT:		return MTLPixelFormatInvalid;
				case GL_DEPTH_STENCIL:			return MTLPixelFormatInvalid;
			}
			break;
		}
		case GL_BYTE:
		{
			switch ( format )
			{
				case GL_RED:					return MTLPixelFormatR8Snorm;
				case GL_RG:						return MTLPixelFormatRG8Snorm;
				case GL_RGB:					return MTLPixelFormatInvalid;
				case GL_BGR:					return MTLPixelFormatInvalid;
				case GL_RGBA:					return MTLPixelFormatRGBA8Snorm;
				case GL_BGRA:					return MTLPixelFormatInvalid;
				case GL_RED_INTEGER:			return MTLPixelFormatR8Sint;
				case GL_RG_INTEGER:				return MTLPixelFormatRG8Sint;
				case GL_RGB_INTEGER:			return MTLPixelFormatInvalid;
				case GL_BGR_INTEGER:			return MTLPixelFormatInvalid;
				case GL_RGBA_INTEGER:			return MTLPixelFormatRGBA8Sint;
				case GL_BGRA_INTEGER:			return MTLPixelFormatInvalid;
				case GL_STENCIL_INDEX:			return MTLPixelFormatInvalid;
				case GL_DEPTH_COMPONENT:		return MTLPixelFormatInvalid;
				case GL_DEPTH_STENCIL:			return MTLPixelFormatInvalid;
			}
			break;
		}

		//
		// 16 bits per component
		//
		case GL_UNSIGNED_SHORT:
		{
			switch ( format )
			{
				case GL_RED:					return MTLPixelFormatR16Unorm;
				case GL_RG:						return MTLPixelFormatRG16Unorm;
				case GL_RGB:					return MTLPixelFormatInvalid;
				case GL_BGR:					return MTLPixelFormatInvalid;
				case GL_RGBA:					return MTLPixelFormatRGBA16Unorm;
				case GL_BGRA:					return MTLPixelFormatInvalid;
				case GL_RED_INTEGER:			return MTLPixelFormatR16Uint;
				case GL_RG_INTEGER:				return MTLPixelFormatRG16Uint;
				case GL_RGB_INTEGER:			return MTLPixelFormatInvalid;
				case GL_BGR_INTEGER:			return MTLPixelFormatInvalid;
				case GL_RGBA_INTEGER:			return MTLPixelFormatRGBA16Uint;
				case GL_BGRA_INTEGER:			return MTLPixelFormatInvalid;
				case GL_STENCIL_INDEX:			return MTLPixelFormatInvalid;
				case GL_DEPTH_COMPONENT:		return MTLPixelFormatInvalid;
				case GL_DEPTH_STENCIL:			return MTLPixelFormatInvalid;
			}
			break;
		}
		case GL_SHORT:
		{
			switch ( format )
			{
				case GL_RED:					return MTLPixelFormatR16Snorm;
				case GL_RG:						return MTLPixelFormatRG16Snorm;
				case GL_RGB:					return MTLPixelFormatInvalid;
				case GL_BGR:					return MTLPixelFormatInvalid;
				case GL_RGBA:					return MTLPixelFormatRGBA16Snorm;
				case GL_BGRA:					return MTLPixelFormatInvalid;
				case GL_RED_INTEGER:			return MTLPixelFormatR16Sint;
				case GL_RG_INTEGER:				return MTLPixelFormatRG16Sint;
				case GL_RGB_INTEGER:			return MTLPixelFormatInvalid;
				case GL_BGR_INTEGER:			return MTLPixelFormatInvalid;
				case GL_RGBA_INTEGER:			return MTLPixelFormatRGBA16Sint;
				case GL_BGRA_INTEGER:			return MTLPixelFormatInvalid;
				case GL_STENCIL_INDEX:			return MTLPixelFormatInvalid;
				case GL_DEPTH_COMPONENT:		return MTLPixelFormatInvalid;
				case GL_DEPTH_STENCIL:			return MTLPixelFormatInvalid;
			}
			break;
		}
		case GL_HALF_FLOAT:
		case GL_HALF_FLOAT_OES:
		{
			switch ( format )
			{
				case GL_RED:					return MTLPixelFormatR16Float;
				case GL_RG:						return MTLPixelFormatRG16Float;
				case GL_RGB:					return MTLPixelFormatInvalid;
				case GL_BGR:					return MTLPixelFormatInvalid;
				case GL_RGBA:					return MTLPixelFormatRGBA16Float;
				case GL_BGRA:					return MTLPixelFormatInvalid;
				case GL_RED_INTEGER:			return MTLPixelFormatInvalid;
				case GL_RG_INTEGER:				return MTLPixelFormatInvalid;
				case GL_RGB_INTEGER:			return MTLPixelFormatInvalid;
				case GL_BGR_INTEGER:			return MTLPixelFormatInvalid;
				case GL_RGBA_INTEGER:			return MTLPixelFormatInvalid;
				case GL_BGRA_INTEGER:			return MTLPixelFormatInvalid;
				case GL_STENCIL_INDEX:			return MTLPixelFormatInvalid;
				case GL_DEPTH_COMPONENT:		return MTLPixelFormatInvalid;
				case GL_DEPTH_STENCIL:			return MTLPixelFormatInvalid;
			}
			break;
		}

		//
		// 32 bits per component
		//
		case GL_UNSIGNED_INT:
		{
			switch ( format )
			{
				case GL_RED:					return MTLPixelFormatR32Uint;
				case GL_RG:						return MTLPixelFormatRG32Uint;
				case GL_RGB:					return MTLPixelFormatInvalid;
				case GL_BGR:					return MTLPixelFormatInvalid;
				case GL_RGBA:					return MTLPixelFormatRGBA32Uint;
				case GL_BGRA:					return MTLPixelFormatInvalid;
				case GL_RED_INTEGER:			return MTLPixelFormatR32Uint;
				case GL_RG_INTEGER:				return MTLPixelFormatRG32Uint;
				case GL_RGB_INTEGER:			return MTLPixelFormatInvalid;
				case GL_BGR_INTEGER:			return MTLPixelFormatInvalid;
				case GL_RGBA_INTEGER:			return MTLPixelFormatRGBA32Uint;
				case GL_BGRA_INTEGER:			return MTLPixelFormatInvalid;
				case GL_STENCIL_INDEX:			return MTLPixelFormatInvalid;
				case GL_DEPTH_COMPONENT:		return MTLPixelFormatInvalid;
				case GL_DEPTH_STENCIL:			return MTLPixelFormatInvalid;
			}
			break;
		}
		case GL_INT:
		{
			switch ( format )
			{
				case GL_RED:					return MTLPixelFormatR32Sint;
				case GL_RG:						return MTLPixelFormatRG32Sint;
				case GL_RGB:					return MTLPixelFormatInvalid;
				case GL_BGR:					return MTLPixelFormatInvalid;
				case GL_RGBA:					return MTLPixelFormatRGBA32Sint;
				case GL_BGRA:					return MTLPixelFormatInvalid;
				case GL_RED_INTEGER:			return MTLPixelFormatR32Sint;
				case GL_RG_INTEGER:				return MTLPixelFormatRG32Sint;
				case GL_RGB_INTEGER:			return MTLPixelFormatInvalid;
				case GL_BGR_INTEGER:			return MTLPixelFormatInvalid;
				case GL_RGBA_INTEGER:			return MTLPixelFormatRGBA32Sint;
				case GL_BGRA_INTEGER:			return MTLPixelFormatInvalid;
				case GL_STENCIL_INDEX:			return MTLPixelFormatInvalid;
				case GL_DEPTH_COMPONENT:		return MTLPixelFormatInvalid;
				case GL_DEPTH_STENCIL:			return MTLPixelFormatInvalid;
			}
			break;
		}
		case GL_FLOAT:
		{
			switch ( format )
			{
				case GL_RED:					return MTLPixelFormatR32Float;
				case GL_RG:						return MTLPixelFormatRG32Float;
				case GL_RGB:					return MTLPixelFormatInvalid;
				case GL_BGR:					return MTLPixelFormatInvalid;
				case GL_RGBA:					return MTLPixelFormatRGBA32Float;
				case GL_BGRA:					return MTLPixelFormatInvalid;
				case GL_RED_INTEGER:			return MTLPixelFormatInvalid;
				case GL_RG_INTEGER:				return MTLPixelFormatInvalid;
				case GL_RGB_INTEGER:			return MTLPixelFormatInvalid;
				case GL_BGR_INTEGER:			return MTLPixelFormatInvalid;
				case GL_RGBA_INTEGER:			return MTLPixelFormatInvalid;
				case GL_BGRA_INTEGER:			return MTLPixelFormatInvalid;
				case GL_STENCIL_INDEX:			return MTLPixelFormatInvalid;
				case GL_DEPTH_COMPONENT:		return MTLPixelFormatDepth32Float;
				case GL_DEPTH_STENCIL:			return MTLPixelFormatInvalid;
			}
			break;
		}

		//
		// 64 bits per component
		//
		case GL_UNSIGNED_INT64:
		{
			switch ( format )
			{
				case GL_RED:					return MTLPixelFormatInvalid;
				case GL_RG:						return MTLPixelFormatInvalid;
				case GL_RGB:					return MTLPixelFormatInvalid;
				case GL_BGR:					return MTLPixelFormatInvalid;
				case GL_RGBA:					return MTLPixelFormatInvalid;
				case GL_BGRA:					return MTLPixelFormatInvalid;
				case GL_RED_INTEGER:			return MTLPixelFormatInvalid;
				case GL_RG_INTEGER:				return MTLPixelFormatInvalid;
				case GL_RGB_INTEGER:			return MTLPixelFormatInvalid;
				case GL_BGR_INTEGER:			return MTLPixelFormatInvalid;
				case GL_RGBA_INTEGER:			return MTLPixelFormatInvalid;
				case GL_BGRA_INTEGER:			return MTLPixelFormatInvalid;
				case GL_STENCIL_INDEX:			return MTLPixelFormatInvalid;
				case GL_DEPTH_COMPONENT:		return MTLPixelFormatInvalid;
				case GL_DEPTH_STENCIL:			return MTLPixelFormatInvalid;
			}
			break;
		}
		case GL_INT64:
		{
			switch ( format )
			{
				case GL_RED:					return MTLPixelFormatInvalid;
				case GL_RG:						return MTLPixelFormatInvalid;
				case GL_RGB:					return MTLPixelFormatInvalid;
				case GL_BGR:					return MTLPixelFormatInvalid;
				case GL_RGBA:					return MTLPixelFormatInvalid;
				case GL_BGRA:					return MTLPixelFormatInvalid;
				case GL_RED_INTEGER:			return MTLPixelFormatInvalid;
				case GL_RG_INTEGER:				return MTLPixelFormatInvalid;
				case GL_RGB_INTEGER:			return MTLPixelFormatInvalid;
				case GL_BGR_INTEGER:			return MTLPixelFormatInvalid;
				case GL_RGBA_INTEGER:			return MTLPixelFormatInvalid;
				case GL_BGRA_INTEGER:			return MTLPixelFormatInvalid;
				case GL_STENCIL_INDEX:			return MTLPixelFormatInvalid;
				case GL_DEPTH_COMPONENT:		return MTLPixelFormatInvalid;
				case GL_DEPTH_STENCIL:			return MTLPixelFormatInvalid;
			}
			break;
		}
		case GL_DOUBLE:
		{
			switch ( format )
			{
				case GL_RED:					return MTLPixelFormatInvalid;
				case GL_RG:						return MTLPixelFormatInvalid;
				case GL_RGB:					return MTLPixelFormatInvalid;
				case GL_BGR:					return MTLPixelFormatInvalid;
				case GL_RGBA:					return MTLPixelFormatInvalid;
				case GL_BGRA:					return MTLPixelFormatInvalid;
				case GL_RED_INTEGER:			return MTLPixelFormatInvalid;
				case GL_RG_INTEGER:				return MTLPixelFormatInvalid;
				case GL_RGB_INTEGER:			return MTLPixelFormatInvalid;
				case GL_BGR_INTEGER:			return MTLPixelFormatInvalid;
				case GL_RGBA_INTEGER:			return MTLPixelFormatInvalid;
				case GL_BGRA_INTEGER:			return MTLPixelFormatInvalid;
				case GL_STENCIL_INDEX:			return MTLPixelFormatInvalid;
				case GL_DEPTH_COMPONENT:		return MTLPixelFormatInvalid;
				case GL_DEPTH_STENCIL:			return MTLPixelFormatInvalid;
			}
			break;
		}

		//
		// Packed
		//
		case GL_UNSIGNED_BYTE_3_3_2:
			assert( format == GL_RGB || format == GL_RGB_INTEGER );
			return MTLPixelFormatInvalid;
		case GL_UNSIGNED_BYTE_2_3_3_REV:
			assert( format == GL_BGR || format == GL_BGR_INTEGER );
			return MTLPixelFormatInvalid;
		case GL_UNSIGNED_SHORT_5_6_5:
			assert( format == GL_RGB || format == GL_RGB_INTEGER );
			return MTLPixelFormatB5G6R5Unorm;
		case GL_UNSIGNED_SHORT_5_6_5_REV:
			assert( format == GL_BGR || format == GL_BGR_INTEGER );
			return MTLPixelFormatB5G6R5Unorm;
		case GL_UNSIGNED_SHORT_4_4_4_4:
			assert( format == GL_RGB || format == GL_BGRA || format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER );
			return MTLPixelFormatABGR4Unorm;
		case GL_UNSIGNED_SHORT_4_4_4_4_REV:
			assert( format == GL_RGB || format == GL_BGRA || format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER );
			return MTLPixelFormatABGR4Unorm;
		case GL_UNSIGNED_SHORT_5_5_5_1:
			assert( format == GL_RGB || format == GL_BGRA || format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER );
			return MTLPixelFormatBGR5A1Unorm;
		case GL_UNSIGNED_SHORT_1_5_5_5_REV:
			assert( format == GL_RGB || format == GL_BGRA || format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER );
			return MTLPixelFormatA1BGR5Unorm;
		case GL_UNSIGNED_INT_8_8_8_8:
			assert( format == GL_RGB || format == GL_BGRA || format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER );
			return ( format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER ) ? MTLPixelFormatRGBA8Uint : MTLPixelFormatRGBA8Unorm;
		case GL_UNSIGNED_INT_8_8_8_8_REV:
			assert( format == GL_RGB || format == GL_BGRA || format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER );
			return ( format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER ) ? MTLPixelFormatInvalid : MTLPixelFormatBGRA8Unorm;
		case GL_UNSIGNED_INT_10_10_10_2:
			assert( format == GL_RGB || format == GL_BGRA || format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER );
			return ( format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER ) ? MTLPixelFormatRGB10A2Uint : MTLPixelFormatRGB10A2Unorm;
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			assert( format == GL_RGB || format == GL_BGRA || format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER );
			return ( format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER ) ? MTLPixelFormatRGB10A2Uint : MTLPixelFormatRGB10A2Unorm;
		case GL_UNSIGNED_INT_10F_11F_11F_REV:
			assert( format == GL_RGB || format == GL_BGR );
			return MTLPixelFormatRG11B10Float;
		case GL_UNSIGNED_INT_5_9_9_9_REV:
			assert( format == GL_RGB || format == GL_BGR );
			return MTLPixelFormatRGB9E5Float;
		case GL_UNSIGNED_INT_24_8:
			assert( format == GL_DEPTH_STENCIL );
			return MTLPixelFormatDepth24Unorm_Stencil8;
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			assert( format == GL_DEPTH_STENCIL );
			return MTLPixelFormatDepth32Float_Stencil8;
	}

	return MTLPixelFormatInvalid;
}

static inline MTLPixelFormat mtlGetFormatFromOpenGLType( const GLenum type, const GLuint numComponents, const GLboolean normalized )
{
	switch ( type )
	{
		//
		// 8 bits per component
		//
		case GL_UNSIGNED_BYTE:
		{
			switch ( numComponents )
			{
				case 1:							return normalized ? MTLPixelFormatR8Unorm : MTLPixelFormatR8Uint;
				case 2:							return normalized ? MTLPixelFormatRG8Unorm : MTLPixelFormatRG8Uint;
				case 3:							return normalized ? MTLPixelFormatInvalid : MTLPixelFormatInvalid;
				case 4:							return normalized ? MTLPixelFormatRGBA8Unorm : MTLPixelFormatRGBA8Uint;
			}
			break;
		}
		case GL_BYTE:
		{
			switch ( numComponents )
			{
				case 1:							return normalized ? MTLPixelFormatR8Snorm : MTLPixelFormatR8Sint;
				case 2:							return normalized ? MTLPixelFormatRG8Snorm : MTLPixelFormatRG8Sint;
				case 3:							return normalized ? MTLPixelFormatInvalid : MTLPixelFormatInvalid;
				case 4:							return normalized ? MTLPixelFormatRGBA8Snorm : MTLPixelFormatRGBA8Sint;
			}
			break;
		}

		//
		// 16 bits per component
		//
		case GL_UNSIGNED_SHORT:
		{
			switch ( numComponents )
			{
				case 1:							return normalized ? MTLPixelFormatR16Unorm : MTLPixelFormatR16Uint;
				case 2:							return normalized ? MTLPixelFormatRG16Unorm : MTLPixelFormatRG16Uint;
				case 3:							return normalized ? MTLPixelFormatInvalid : MTLPixelFormatInvalid;
				case 4:							return normalized ? MTLPixelFormatRGBA16Unorm : MTLPixelFormatRGBA16Uint;
			}
			break;
		}
		case GL_SHORT:
		{
			switch ( numComponents )
			{
				case 1:							return normalized ? MTLPixelFormatR16Snorm : MTLPixelFormatR16Sint;
				case 2:							return normalized ? MTLPixelFormatRG16Snorm : MTLPixelFormatRG16Sint;
				case 3:							return normalized ? MTLPixelFormatInvalid : MTLPixelFormatInvalid;
				case 4:							return normalized ? MTLPixelFormatRGBA16Snorm : MTLPixelFormatRGBA16Sint;
			}
			break;
		}
		case GL_HALF_FLOAT:
		case GL_HALF_FLOAT_OES:
		{
			switch ( numComponents )
			{
				case 1:							return MTLPixelFormatR16Float;
				case 2:							return MTLPixelFormatRG16Float;
				case 3:							return MTLPixelFormatInvalid;
				case 4:							return MTLPixelFormatRGBA16Float;
			}
			break;
		}

		//
		// 32 bits per component
		//
		case GL_UNSIGNED_INT:
		{
			switch ( numComponents )
			{
				case 1:							return MTLPixelFormatR32Uint;
				case 2:							return MTLPixelFormatRG32Uint;
				case 3:							return MTLPixelFormatInvalid;
				case 4:							return MTLPixelFormatRGBA32Uint;
			}
			break;
		}
		case GL_INT:
		{
			switch ( numComponents )
			{
				case 1:							return MTLPixelFormatR32Sint;
				case 2:							return MTLPixelFormatRG32Sint;
				case 3:							return MTLPixelFormatInvalid;
				case 4:							return MTLPixelFormatRGBA32Sint;
			}
			break;
		}
		case GL_FLOAT:
		{
			switch ( numComponents )
			{
				case 1:							return MTLPixelFormatR32Float;
				case 2:							return MTLPixelFormatRG32Float;
				case 3:							return MTLPixelFormatInvalid;
				case 4:							return MTLPixelFormatRGBA32Float;
			}
			break;
		}

		//
		// 64 bits per component
		//
		case GL_UNSIGNED_INT64:
		{
			switch ( numComponents )
			{
				case 1:							return MTLPixelFormatInvalid;
				case 2:							return MTLPixelFormatInvalid;
				case 3:							return MTLPixelFormatInvalid;
				case 4:							return MTLPixelFormatInvalid;
			}
			break;
		}
		case GL_INT64:
		{
			switch ( numComponents )
			{
				case 1:							return MTLPixelFormatInvalid;
				case 2:							return MTLPixelFormatInvalid;
				case 3:							return MTLPixelFormatInvalid;
				case 4:							return MTLPixelFormatInvalid;
			}
			break;
		}
		case GL_DOUBLE:
		{
			switch ( numComponents )
			{
				case 1:							return MTLPixelFormatInvalid;
				case 2:							return MTLPixelFormatInvalid;
				case 3:							return MTLPixelFormatInvalid;
				case 4:							return MTLPixelFormatInvalid;
			}
			break;
		}

		//
		// Packed
		//
		case GL_UNSIGNED_BYTE_3_3_2:			return MTLPixelFormatInvalid;
		case GL_UNSIGNED_BYTE_2_3_3_REV:		return MTLPixelFormatInvalid;
		case GL_UNSIGNED_SHORT_5_6_5:			return MTLPixelFormatB5G6R5Unorm;
		case GL_UNSIGNED_SHORT_5_6_5_REV:		return MTLPixelFormatB5G6R5Unorm;
		case GL_UNSIGNED_SHORT_4_4_4_4:			return MTLPixelFormatABGR4Unorm;
		case GL_UNSIGNED_SHORT_4_4_4_4_REV:		return MTLPixelFormatABGR4Unorm;
		case GL_UNSIGNED_SHORT_5_5_5_1:			return MTLPixelFormatA1BGR5Unorm;
		case GL_UNSIGNED_SHORT_1_5_5_5_REV:		return MTLPixelFormatA1BGR5Unorm;
		case GL_UNSIGNED_INT_8_8_8_8:			return normalized ? MTLPixelFormatRGBA8Unorm : MTLPixelFormatRGBA8Uint;
		case GL_UNSIGNED_INT_8_8_8_8_REV:		return normalized ? MTLPixelFormatBGRA8Unorm : MTLPixelFormatInvalid;
		case GL_UNSIGNED_INT_10_10_10_2:		return normalized ? MTLPixelFormatRGB10A2Unorm : MTLPixelFormatRGB10A2Uint;
		case GL_UNSIGNED_INT_2_10_10_10_REV:	return normalized ? MTLPixelFormatRGB10A2Unorm : MTLPixelFormatRGB10A2Uint;
		case GL_UNSIGNED_INT_10F_11F_11F_REV:	return MTLPixelFormatRG11B10Float;
		case GL_UNSIGNED_INT_5_9_9_9_REV:		return MTLPixelFormatRGB9E5Float;
		case GL_UNSIGNED_INT_24_8:				return MTLPixelFormatDepth24Unorm_Stencil8;
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:	return MTLPixelFormatDepth32Float_Stencil8;
	}

	return MTLPixelFormatInvalid;
}

static inline MTLPixelFormat mtlGetFormatFromOpenGLInternalFormat( const GLenum internalFormat )
{
	switch ( internalFormat )
	{
		//
		// 8 bits per component
		//
		case GL_R8:												return MTLPixelFormatR8Unorm;				// 1-component, 8-bit unsigned normalized
		case GL_RG8:											return MTLPixelFormatRG8Unorm;				// 2-component, 8-bit unsigned normalized
		case GL_RGB8:											return MTLPixelFormatInvalid;				// 3-component, 8-bit unsigned normalized
		case GL_RGBA8:											return MTLPixelFormatRGBA8Unorm;			// 4-component, 8-bit unsigned normalized

		case GL_R8_SNORM:										return MTLPixelFormatR8Snorm;				// 1-component, 8-bit signed normalized
		case GL_RG8_SNORM:										return MTLPixelFormatRG8Snorm;				// 2-component, 8-bit signed normalized
		case GL_RGB8_SNORM:										return MTLPixelFormatInvalid;				// 3-component, 8-bit signed normalized
		case GL_RGBA8_SNORM:									return MTLPixelFormatRGBA8Snorm;			// 4-component, 8-bit signed normalized

		case GL_R8UI:											return MTLPixelFormatR8Uint;				// 1-component, 8-bit unsigned integer
		case GL_RG8UI:											return MTLPixelFormatRG8Uint;				// 2-component, 8-bit unsigned integer
		case GL_RGB8UI:											return MTLPixelFormatInvalid;				// 3-component, 8-bit unsigned integer
		case GL_RGBA8UI:										return MTLPixelFormatRGBA8Uint;				// 4-component, 8-bit unsigned integer

		case GL_R8I:											return MTLPixelFormatR8Sint;				// 1-component, 8-bit signed integer
		case GL_RG8I:											return MTLPixelFormatRG8Sint;				// 2-component, 8-bit signed integer
		case GL_RGB8I:											return MTLPixelFormatInvalid;				// 3-component, 8-bit signed integer
		case GL_RGBA8I:											return MTLPixelFormatRGBA8Sint;				// 4-component, 8-bit signed integer

		case GL_SR8:											return MTLPixelFormatR8Unorm_sRGB;			// 1-component, 8-bit sRGB
		case GL_SRG8:											return MTLPixelFormatRG8Unorm_sRGB;			// 2-component, 8-bit sRGB
		case GL_SRGB8:											return MTLPixelFormatInvalid;				// 3-component, 8-bit sRGB
		case GL_SRGB8_ALPHA8:									return MTLPixelFormatRGBA8Unorm_sRGB;		// 4-component, 8-bit sRGB

		//
		// 16 bits per component
		//
		case GL_R16:											return MTLPixelFormatR16Unorm;				// 1-component, 16-bit unsigned normalized
		case GL_RG16:											return MTLPixelFormatRG16Unorm;				// 2-component, 16-bit unsigned normalized
		case GL_RGB16:											return MTLPixelFormatInvalid;				// 3-component, 16-bit unsigned normalized
		case GL_RGBA16:											return MTLPixelFormatRGBA16Unorm;			// 4-component, 16-bit unsigned normalized

		case GL_R16_SNORM:										return MTLPixelFormatR16Snorm;				// 1-component, 16-bit signed normalized
		case GL_RG16_SNORM:										return MTLPixelFormatRG16Snorm;				// 2-component, 16-bit signed normalized
		case GL_RGB16_SNORM:									return MTLPixelFormatInvalid;				// 3-component, 16-bit signed normalized
		case GL_RGBA16_SNORM:									return MTLPixelFormatRGBA16Snorm;			// 4-component, 16-bit signed normalized

		case GL_R16UI:											return MTLPixelFormatR16Uint;				// 1-component, 16-bit unsigned integer
		case GL_RG16UI:											return MTLPixelFormatRG16Uint;				// 2-component, 16-bit unsigned integer
		case GL_RGB16UI:										return MTLPixelFormatInvalid;				// 3-component, 16-bit unsigned integer
		case GL_RGBA16UI:										return MTLPixelFormatRGBA16Uint;			// 4-component, 16-bit unsigned integer

		case GL_R16I:											return MTLPixelFormatR16Sint;				// 1-component, 16-bit signed integer
		case GL_RG16I:											return MTLPixelFormatRG16Sint;				// 2-component, 16-bit signed integer
		case GL_RGB16I:											return MTLPixelFormatInvalid;				// 3-component, 16-bit signed integer
		case GL_RGBA16I:										return MTLPixelFormatRGBA16Sint;			// 4-component, 16-bit signed integer

		case GL_R16F:											return MTLPixelFormatR16Float;				// 1-component, 16-bit floating-point
		case GL_RG16F:											return MTLPixelFormatRG16Float;				// 2-component, 16-bit floating-point
		case GL_RGB16F:											return MTLPixelFormatInvalid;				// 3-component, 16-bit floating-point
		case GL_RGBA16F:										return MTLPixelFormatRGBA16Float;			// 4-component, 16-bit floating-point

		//
		// 32 bits per component
		//
		case GL_R32UI:											return MTLPixelFormatR32Uint;				// 1-component, 32-bit unsigned integer
		case GL_RG32UI:											return MTLPixelFormatRG32Uint;				// 2-component, 32-bit unsigned integer
		case GL_RGB32UI:										return MTLPixelFormatInvalid;				// 3-component, 32-bit unsigned integer
		case GL_RGBA32UI:										return MTLPixelFormatRGBA32Uint;			// 4-component, 32-bit unsigned integer

		case GL_R32I:											return MTLPixelFormatR32Sint;				// 1-component, 32-bit signed integer
		case GL_RG32I:											return MTLPixelFormatRG32Sint;				// 2-component, 32-bit signed integer
		case GL_RGB32I:											return MTLPixelFormatInvalid;				// 3-component, 32-bit signed integer
		case GL_RGBA32I:										return MTLPixelFormatRGBA32Sint;			// 4-component, 32-bit signed integer

		case GL_R32F:											return MTLPixelFormatR32Float;				// 1-component, 32-bit floating-point
		case GL_RG32F:											return MTLPixelFormatRG32Float;				// 2-component, 32-bit floating-point
		case GL_RGB32F:											return MTLPixelFormatInvalid;				// 3-component, 32-bit floating-point
		case GL_RGBA32F:										return MTLPixelFormatRGBA32Float;			// 4-component, 32-bit floating-point

		//
		// Packed
		//
		case GL_R3_G3_B2:										return MTLPixelFormatInvalid;				// 3-component 3:3:2,       unsigned normalized
		case GL_RGB4:											return MTLPixelFormatInvalid;				// 3-component 4:4:4,       unsigned normalized
		case GL_RGB5:											return MTLPixelFormatBGR5A1Unorm;			// 3-component 5:5:5,       unsigned normalized
		case GL_RGB565:											return MTLPixelFormatB5G6R5Unorm;			// 3-component 5:6:5,       unsigned normalized
		case GL_RGB10:											return MTLPixelFormatRGB10A2Unorm;			// 3-component 10:10:10,    unsigned normalized
		case GL_RGB12:											return MTLPixelFormatInvalid;				// 3-component 12:12:12,    unsigned normalized
		case GL_RGBA2:											return MTLPixelFormatInvalid;				// 4-component 2:2:2:2,     unsigned normalized
		case GL_RGBA4:											return MTLPixelFormatABGR4Unorm;			// 4-component 4:4:4:4,     unsigned normalized
		case GL_RGBA12:											return MTLPixelFormatInvalid;				// 4-component 12:12:12:12, unsigned normalized
		case GL_RGB5_A1:										return MTLPixelFormatBGR5A1Unorm;			// 4-component 5:5:5:1,     unsigned normalized
		case GL_RGB10_A2:										return MTLPixelFormatRGB10A2Unorm;			// 4-component 10:10:10:2,  unsigned normalized
		case GL_RGB10_A2UI:										return MTLPixelFormatRGB10A2Uint;			// 4-component 10:10:10:2,  unsigned integer
		case GL_R11F_G11F_B10F:									return MTLPixelFormatRG11B10Float;			// 3-component 11:11:10,    floating-point
		case GL_RGB9_E5:										return MTLPixelFormatRGB9E5Float;			// 3-component/exp 9:9:9/5, floating-point

		//
		// S3TC/DXT/BC
		//

		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:					return MTLPixelFormatBC1_RGBA;				// line through 3D space, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:					return MTLPixelFormatBC1_RGBA;				// line through 3D space plus 1-bit alpha, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:					return MTLPixelFormatBC2_RGBA;				// line through 3D space plus line through 1D space, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:					return MTLPixelFormatBC3_RGBA;				// line through 3D space plus 4-bit alpha, 4x4 blocks, unsigned normalized

		case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:					return MTLPixelFormatBC1_RGBA_sRGB;			// line through 3D space, 4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:			return MTLPixelFormatBC1_RGBA_sRGB;			// line through 3D space plus 1-bit alpha, 4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:			return MTLPixelFormatBC2_RGBA_sRGB;			// line through 3D space plus line through 1D space, 4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:			return MTLPixelFormatBC3_RGBA_sRGB;			// line through 3D space plus 4-bit alpha, 4x4 blocks, sRGB

		case GL_COMPRESSED_LUMINANCE_LATC1_EXT:					return MTLPixelFormatBC4_RUnorm;			// line through 1D space, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:			return MTLPixelFormatBC5_RGUnorm;			// two lines through 1D space, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:			return MTLPixelFormatBC4_RSnorm;			// line through 1D space, 4x4 blocks, signed normalized
		case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:	return MTLPixelFormatBC5_RGSnorm;			// two lines through 1D space, 4x4 blocks, signed normalized

		case GL_COMPRESSED_RED_RGTC1:							return MTLPixelFormatBC4_RUnorm;			// line through 1D space, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RG_RGTC2:							return MTLPixelFormatBC5_RGUnorm;			// two lines through 1D space, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_SIGNED_RED_RGTC1:					return MTLPixelFormatBC4_RSnorm;			// line through 1D space, 4x4 blocks, signed normalized
		case GL_COMPRESSED_SIGNED_RG_RGTC2:						return MTLPixelFormatBC5_RGSnorm;			// two lines through 1D space, 4x4 blocks, signed normalized

		case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:				return MTLPixelFormatBC6H_RGBUfloat;		// 3-component, 4x4 blocks, unsigned floating-point
		case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:				return MTLPixelFormatBC6H_RGBFloat;			// 3-component, 4x4 blocks, signed floating-point
		case GL_COMPRESSED_RGBA_BPTC_UNORM:						return MTLPixelFormatBC7_RGBAUnorm;			// 4-component, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:				return MTLPixelFormatBC7_RGBAUnorm_sRGB;	// 4-component, 4x4 blocks, sRGB

		//
		// ETC
		//
		case GL_ETC1_RGB8_OES:									return MTLPixelFormatETC2_RGB8;				// 3-component ETC1, 4x4 blocks, unsigned normalized

		case GL_COMPRESSED_RGB8_ETC2:							return MTLPixelFormatETC2_RGB8;				// 3-component ETC2, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:		return MTLPixelFormatETC2_RGB8A1;			// 4-component ETC2 with 1-bit alpha, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA8_ETC2_EAC:						return MTLPixelFormatEAC_RGBA8;				// 4-component ETC2, 4x4 blocks, unsigned normalized

		case GL_COMPRESSED_SRGB8_ETC2:							return MTLPixelFormatETC2_RGB8_sRGB;		// 3-component ETC2, 4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:		return MTLPixelFormatETC2_RGB8A1_sRGB;		// 4-component ETC2 with 1-bit alpha, 4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:				return MTLPixelFormatEAC_RGBA8_sRGB;		// 4-component ETC2, 4x4 blocks, sRGB

		case GL_COMPRESSED_R11_EAC:								return MTLPixelFormatEAC_R11Unorm;			// 1-component ETC, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RG11_EAC:							return MTLPixelFormatEAC_RG11Unorm;			// 2-component ETC, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_SIGNED_R11_EAC:						return MTLPixelFormatEAC_R11Snorm;			// 1-component ETC, 4x4 blocks, signed normalized
		case GL_COMPRESSED_SIGNED_RG11_EAC:						return MTLPixelFormatEAC_RG11Snorm;			// 2-component ETC, 4x4 blocks, signed normalized

		//
		// PVRTC
		//
		case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:				return MTLPixelFormatPVRTC_RGB_2BPP;		// 3-component PVRTC, 16x8 blocks, unsigned normalized
		case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:				return MTLPixelFormatPVRTC_RGB_4BPP;		// 3-component PVRTC,  8x8 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:				return MTLPixelFormatPVRTC_RGBA_2BPP;		// 4-component PVRTC, 16x8 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:				return MTLPixelFormatPVRTC_RGBA_4BPP;		// 4-component PVRTC,  8x8 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG:				return MTLPixelFormatInvalid;				// 4-component PVRTC,  8x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG:				return MTLPixelFormatInvalid;				// 4-component PVRTC,  4x4 blocks, unsigned normalized

		case GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT:				return MTLPixelFormatPVRTC_RGB_2BPP_sRGB;	// 3-component PVRTC, 16x8 blocks, sRGB
		case GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT:				return MTLPixelFormatPVRTC_RGB_4BPP_sRGB;	// 3-component PVRTC,  8x8 blocks, sRGB
		case GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT:			return MTLPixelFormatPVRTC_RGBA_2BPP_sRGB;	// 4-component PVRTC, 16x8 blocks, sRGB
		case GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT:			return MTLPixelFormatPVRTC_RGBA_4BPP_sRGB;	// 4-component PVRTC,  8x8 blocks, sRGB
		case GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG:			return MTLPixelFormatInvalid;				// 4-component PVRTC,  8x4 blocks, sRGB
		case GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG:			return MTLPixelFormatInvalid;				// 4-component PVRTC,  4x4 blocks, sRGB

		//
		// ASTC
		//
		case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:					return MTLPixelFormatASTC_4x4_LDR;			// 4-component ASTC, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_5x4_KHR:					return MTLPixelFormatASTC_5x4_LDR;			// 4-component ASTC, 5x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_5x5_KHR:					return MTLPixelFormatASTC_5x5_LDR;			// 4-component ASTC, 5x5 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_6x5_KHR:					return MTLPixelFormatASTC_6x5_LDR;			// 4-component ASTC, 6x5 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_6x6_KHR:					return MTLPixelFormatASTC_6x6_LDR;			// 4-component ASTC, 6x6 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_8x5_KHR:					return MTLPixelFormatASTC_8x5_LDR;			// 4-component ASTC, 8x5 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_8x6_KHR:					return MTLPixelFormatASTC_8x6_LDR;			// 4-component ASTC, 8x6 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:					return MTLPixelFormatASTC_8x8_LDR;			// 4-component ASTC, 8x8 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_10x5_KHR:					return MTLPixelFormatASTC_10x5_LDR;			// 4-component ASTC, 10x5 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_10x6_KHR:					return MTLPixelFormatASTC_10x6_LDR;			// 4-component ASTC, 10x6 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_10x8_KHR:					return MTLPixelFormatASTC_10x8_LDR;			// 4-component ASTC, 10x8 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_10x10_KHR:					return MTLPixelFormatASTC_10x10_LDR;		// 4-component ASTC, 10x10 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_12x10_KHR:					return MTLPixelFormatASTC_12x10_LDR;		// 4-component ASTC, 12x10 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_12x12_KHR:					return MTLPixelFormatASTC_12x12_LDR;		// 4-component ASTC, 12x12 blocks, unsigned normalized

		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:			return MTLPixelFormatASTC_4x4_sRGB;			// 4-component ASTC, 4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:			return MTLPixelFormatASTC_5x4_sRGB;			// 4-component ASTC, 5x4 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:			return MTLPixelFormatASTC_5x5_sRGB;			// 4-component ASTC, 5x5 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:			return MTLPixelFormatASTC_6x5_sRGB;			// 4-component ASTC, 6x5 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:			return MTLPixelFormatASTC_6x6_sRGB;			// 4-component ASTC, 6x6 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:			return MTLPixelFormatASTC_8x5_sRGB;			// 4-component ASTC, 8x5 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:			return MTLPixelFormatASTC_8x6_sRGB;			// 4-component ASTC, 8x6 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:			return MTLPixelFormatASTC_8x8_sRGB;			// 4-component ASTC, 8x8 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:			return MTLPixelFormatASTC_10x5_sRGB;		// 4-component ASTC, 10x5 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:			return MTLPixelFormatASTC_10x6_sRGB;		// 4-component ASTC, 10x6 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:			return MTLPixelFormatASTC_10x8_sRGB;		// 4-component ASTC, 10x8 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:			return MTLPixelFormatASTC_10x10_sRGB;		// 4-component ASTC, 10x10 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:			return MTLPixelFormatASTC_12x10_sRGB;		// 4-component ASTC, 12x10 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:			return MTLPixelFormatASTC_12x12_sRGB;		// 4-component ASTC, 12x12 blocks, sRGB

		case GL_COMPRESSED_RGBA_ASTC_3x3x3_OES:					return MTLPixelFormatInvalid;				// 4-component ASTC, 3x3x3 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_4x3x3_OES:					return MTLPixelFormatInvalid;				// 4-component ASTC, 4x3x3 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_4x4x3_OES:					return MTLPixelFormatInvalid;				// 4-component ASTC, 4x4x3 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_4x4x4_OES:					return MTLPixelFormatInvalid;				// 4-component ASTC, 4x4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_5x4x4_OES:					return MTLPixelFormatInvalid;				// 4-component ASTC, 5x4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_5x5x4_OES:					return MTLPixelFormatInvalid;				// 4-component ASTC, 5x5x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_5x5x5_OES:					return MTLPixelFormatInvalid;				// 4-component ASTC, 5x5x5 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_6x5x5_OES:					return MTLPixelFormatInvalid;				// 4-component ASTC, 6x5x5 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_6x6x5_OES:					return MTLPixelFormatInvalid;				// 4-component ASTC, 6x6x5 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_6x6x6_OES:					return MTLPixelFormatInvalid;				// 4-component ASTC, 6x6x6 blocks, unsigned normalized

		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES:			return MTLPixelFormatInvalid;				// 4-component ASTC, 3x3x3 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES:			return MTLPixelFormatInvalid;				// 4-component ASTC, 4x3x3 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES:			return MTLPixelFormatInvalid;				// 4-component ASTC, 4x4x3 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES:			return MTLPixelFormatInvalid;				// 4-component ASTC, 4x4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES:			return MTLPixelFormatInvalid;				// 4-component ASTC, 5x4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES:			return MTLPixelFormatInvalid;				// 4-component ASTC, 5x5x4 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES:			return MTLPixelFormatInvalid;				// 4-component ASTC, 5x5x5 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES:			return MTLPixelFormatInvalid;				// 4-component ASTC, 6x5x5 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES:			return MTLPixelFormatInvalid;				// 4-component ASTC, 6x6x5 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES:			return MTLPixelFormatInvalid;				// 4-component ASTC, 6x6x6 blocks, sRGB

		//
		// ATC
		//
		case GL_ATC_RGB_AMD:									return MTLPixelFormatInvalid;				// 3-component, 4x4 blocks, unsigned normalized
		case GL_ATC_RGBA_EXPLICIT_ALPHA_AMD:					return MTLPixelFormatInvalid;				// 4-component, 4x4 blocks, unsigned normalized
		case GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD:				return MTLPixelFormatInvalid;				// 4-component, 4x4 blocks, unsigned normalized

		//
		// Palletized
		//
		case GL_PALETTE4_RGB8_OES:								return MTLPixelFormatInvalid;				// 3-component 8:8:8,   4-bit palette, unsigned normalized
		case GL_PALETTE4_RGBA8_OES:								return MTLPixelFormatInvalid;				// 4-component 8:8:8:8, 4-bit palette, unsigned normalized
		case GL_PALETTE4_R5_G6_B5_OES:							return MTLPixelFormatInvalid;				// 3-component 5:6:5,   4-bit palette, unsigned normalized
		case GL_PALETTE4_RGBA4_OES:								return MTLPixelFormatInvalid;				// 4-component 4:4:4:4, 4-bit palette, unsigned normalized
		case GL_PALETTE4_RGB5_A1_OES:							return MTLPixelFormatInvalid;				// 4-component 5:5:5:1, 4-bit palette, unsigned normalized
		case GL_PALETTE8_RGB8_OES:								return MTLPixelFormatInvalid;				// 3-component 8:8:8,   8-bit palette, unsigned normalized
		case GL_PALETTE8_RGBA8_OES:								return MTLPixelFormatInvalid;				// 4-component 8:8:8:8, 8-bit palette, unsigned normalized
		case GL_PALETTE8_R5_G6_B5_OES:							return MTLPixelFormatInvalid;				// 3-component 5:6:5,   8-bit palette, unsigned normalized
		case GL_PALETTE8_RGBA4_OES:								return MTLPixelFormatInvalid;				// 4-component 4:4:4:4, 8-bit palette, unsigned normalized
		case GL_PALETTE8_RGB5_A1_OES:							return MTLPixelFormatInvalid;				// 4-component 5:5:5:1, 8-bit palette, unsigned normalized

		//
		// Depth/stencil
		//
		case GL_DEPTH_COMPONENT16:								return MTLPixelFormatInvalid;
		case GL_DEPTH_COMPONENT24:								return MTLPixelFormatInvalid;
		case GL_DEPTH_COMPONENT32:								return MTLPixelFormatInvalid;
		case GL_DEPTH_COMPONENT32F:								return MTLPixelFormatDepth32Float;
		case GL_DEPTH_COMPONENT32F_NV:							return MTLPixelFormatDepth32Float;
		case GL_STENCIL_INDEX1:									return MTLPixelFormatInvalid;
		case GL_STENCIL_INDEX4:									return MTLPixelFormatInvalid;
		case GL_STENCIL_INDEX8:									return MTLPixelFormatInvalid;
		case GL_STENCIL_INDEX16:								return MTLPixelFormatInvalid;
		case GL_DEPTH24_STENCIL8:								return MTLPixelFormatInvalid;
		case GL_DEPTH32F_STENCIL8:								return MTLPixelFormatInvalid;
		case GL_DEPTH32F_STENCIL8_NV:							return MTLPixelFormatInvalid;

		default:												return MTLPixelFormatInvalid;
	}
}

typedef enum MtlFormatSizeFlagBits {
	MTL_FORMAT_SIZE_PACKED_BIT				= 0x00000001,
	MTL_FORMAT_SIZE_COMPRESSED_BIT			= 0x00000002,
	MTL_FORMAT_SIZE_PALETTIZED_BIT			= 0x00000004,
	MTL_FORMAT_SIZE_DEPTH_BIT				= 0x00000008,
	MTL_FORMAT_SIZE_STENCIL_BIT				= 0x00000010,
} MtlFormatSizeFlagBits;

typedef unsigned int MtlFormatSizeFlags;

typedef struct MtlFormatSize {
	MtlFormatSizeFlags	flags;
	unsigned int		paletteSizeInBits;
	unsigned int		blockSizeInBits;
	unsigned int		blockWidth;			// in texels
	unsigned int		blockHeight;		// in texels
	unsigned int		blockDepth;			// in texels
} MtlFormatSize;

static inline void mtlGetFormatSize( const MTLPixelFormat format, MtlFormatSize * pFormatSize )
{
	switch ( format )
	{
		case MTLPixelFormatABGR4Unorm:
		case MTLPixelFormatB5G6R5Unorm:
		case MTLPixelFormatBGR5A1Unorm:
		case MTLPixelFormatA1BGR5Unorm:
			pFormatSize->flags = MTL_FORMAT_SIZE_PACKED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 2 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatR8Unorm:
		case MTLPixelFormatR8Snorm:
		case MTLPixelFormatR8Uint:
		case MTLPixelFormatR8Sint:
		case MTLPixelFormatR8Unorm_sRGB:
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 1 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatRG8Unorm:
		case MTLPixelFormatRG8Snorm:
		case MTLPixelFormatRG8Uint:
		case MTLPixelFormatRG8Sint:
		case MTLPixelFormatRG8Unorm_sRGB:
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 2 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatRGBA8Unorm:
		case MTLPixelFormatRGBA8Snorm:
		case MTLPixelFormatRGBA8Uint:
		case MTLPixelFormatRGBA8Sint:
		case MTLPixelFormatRGBA8Unorm_sRGB:
		case MTLPixelFormatBGRA8Unorm:
		case MTLPixelFormatBGRA8Unorm_sRGB:
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 4 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatRGB10A2Unorm:
		case MTLPixelFormatRGB10A2Uint:
			pFormatSize->flags = MTL_FORMAT_SIZE_PACKED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 4 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatR16Unorm:
		case MTLPixelFormatR16Snorm:
		case MTLPixelFormatR16Uint:
		case MTLPixelFormatR16Sint:
		case MTLPixelFormatR16Float:
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 2 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatRG16Unorm:
		case MTLPixelFormatRG16Snorm:
		case MTLPixelFormatRG16Uint:
		case MTLPixelFormatRG16Sint:
		case MTLPixelFormatRG16Float:
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 4 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatRGBA16Unorm:
		case MTLPixelFormatRGBA16Snorm:
		case MTLPixelFormatRGBA16Uint:
		case MTLPixelFormatRGBA16Sint:
		case MTLPixelFormatRGBA16Float:
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 8 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatR32Uint:
		case MTLPixelFormatR32Sint:
		case MTLPixelFormatR32Float:
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 4 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatRG32Uint:
		case MTLPixelFormatRG32Sint:
		case MTLPixelFormatRG32Float:
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 8 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatRGBA32Uint:
		case MTLPixelFormatRGBA32Sint:
		case MTLPixelFormatRGBA32Float:
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatRG11B10Float:
		case MTLPixelFormatRGB9E5Float:
			pFormatSize->flags = MTL_FORMAT_SIZE_PACKED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 4 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatDepth32Float:
			pFormatSize->flags = MTL_FORMAT_SIZE_DEPTH_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 4 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatStencil8:
			pFormatSize->flags = MTL_FORMAT_SIZE_STENCIL_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 1 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatDepth24Unorm_Stencil8:
			pFormatSize->flags = MTL_FORMAT_SIZE_DEPTH_BIT | MTL_FORMAT_SIZE_STENCIL_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 4 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatDepth32Float_Stencil8:
			pFormatSize->flags = MTL_FORMAT_SIZE_DEPTH_BIT | MTL_FORMAT_SIZE_STENCIL_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 8 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatBC1_RGBA:
		case MTLPixelFormatBC1_RGBA_sRGB:
			pFormatSize->flags = MTL_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 8 * 8;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatBC2_RGBA:
		case MTLPixelFormatBC2_RGBA_sRGB:
		case MTLPixelFormatBC3_RGBA:
		case MTLPixelFormatBC3_RGBA_sRGB:
		case MTLPixelFormatBC4_RUnorm:
		case MTLPixelFormatBC4_RSnorm:
		case MTLPixelFormatBC5_RGUnorm:
		case MTLPixelFormatBC5_RGSnorm:
		case MTLPixelFormatBC6H_RGBUfloat:
		case MTLPixelFormatBC6H_RGBFloat:
		case MTLPixelFormatBC7_RGBAUnorm:
		case MTLPixelFormatBC7_RGBAUnorm_sRGB:
			pFormatSize->flags = MTL_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16 * 8;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatPVRTC_RGB_2BPP:
		case MTLPixelFormatPVRTC_RGB_2BPP_sRGB:
			pFormatSize->flags = MTL_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 32 * 8;
			pFormatSize->blockWidth = 16;
			pFormatSize->blockHeight = 8;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatPVRTC_RGB_4BPP:
		case MTLPixelFormatPVRTC_RGB_4BPP_sRGB:
			pFormatSize->flags = MTL_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 32 * 8;
			pFormatSize->blockWidth = 8;
			pFormatSize->blockHeight = 8;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatPVRTC_RGBA_2BPP:
		case MTLPixelFormatPVRTC_RGBA_2BPP_sRGB:
			pFormatSize->flags = MTL_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 32 * 8;
			pFormatSize->blockWidth = 16;
			pFormatSize->blockHeight = 8;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatPVRTC_RGBA_4BPP:
		case MTLPixelFormatPVRTC_RGBA_4BPP_sRGB:
			pFormatSize->flags = MTL_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 32 * 8;
			pFormatSize->blockWidth = 8;
			pFormatSize->blockHeight = 8;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatETC2_RGB8:
		case MTLPixelFormatETC2_RGB8_sRGB:
		case MTLPixelFormatETC2_RGB8A1:
		case MTLPixelFormatETC2_RGB8A1_sRGB:
			pFormatSize->flags = MTL_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 8 * 8;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatEAC_RGBA8:
		case MTLPixelFormatEAC_RGBA8_sRGB:
		case MTLPixelFormatEAC_R11Unorm:
		case MTLPixelFormatEAC_R11Snorm:
		case MTLPixelFormatEAC_RG11Unorm:
		case MTLPixelFormatEAC_RG11Snorm:
			pFormatSize->flags = MTL_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16 * 8;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatASTC_4x4_LDR:
		case MTLPixelFormatASTC_4x4_sRGB:
			pFormatSize->flags = MTL_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16 * 8;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatASTC_5x4_LDR:
		case MTLPixelFormatASTC_5x4_sRGB:
			pFormatSize->flags = MTL_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16 * 8;
			pFormatSize->blockWidth = 5;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatASTC_5x5_LDR:
		case MTLPixelFormatASTC_5x5_sRGB:
			pFormatSize->flags = MTL_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16 * 8;
			pFormatSize->blockWidth = 5;
			pFormatSize->blockHeight = 5;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatASTC_6x5_LDR:
		case MTLPixelFormatASTC_6x5_sRGB:
			pFormatSize->flags = MTL_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16 * 8;
			pFormatSize->blockWidth = 6;
			pFormatSize->blockHeight = 5;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatASTC_6x6_LDR:
		case MTLPixelFormatASTC_6x6_sRGB:
			pFormatSize->flags = MTL_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16 * 8;
			pFormatSize->blockWidth = 6;
			pFormatSize->blockHeight = 6;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatASTC_8x5_LDR:
		case MTLPixelFormatASTC_8x5_sRGB:
			pFormatSize->flags = MTL_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16 * 8;
			pFormatSize->blockWidth = 8;
			pFormatSize->blockHeight = 5;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatASTC_8x6_LDR:
		case MTLPixelFormatASTC_8x6_sRGB:
			pFormatSize->flags = MTL_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16 * 8;
			pFormatSize->blockWidth = 8;
			pFormatSize->blockHeight = 6;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatASTC_8x8_LDR:
		case MTLPixelFormatASTC_8x8_sRGB:
			pFormatSize->flags = MTL_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16 * 8;
			pFormatSize->blockWidth = 8;
			pFormatSize->blockHeight = 8;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatASTC_10x5_LDR:
		case MTLPixelFormatASTC_10x5_sRGB:
			pFormatSize->flags = MTL_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16 * 8;
			pFormatSize->blockWidth = 10;
			pFormatSize->blockHeight = 5;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatASTC_10x6_LDR:
		case MTLPixelFormatASTC_10x6_sRGB:
			pFormatSize->flags = MTL_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16 * 8;
			pFormatSize->blockWidth = 10;
			pFormatSize->blockHeight = 6;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatASTC_10x8_LDR: 
		case MTLPixelFormatASTC_10x8_sRGB:
			pFormatSize->flags = MTL_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16 * 8;
			pFormatSize->blockWidth = 10;
			pFormatSize->blockHeight = 8;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatASTC_10x10_LDR:
		case MTLPixelFormatASTC_10x10_sRGB:
			pFormatSize->flags = MTL_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16 * 8;
			pFormatSize->blockWidth = 10;
			pFormatSize->blockHeight = 10;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatASTC_12x10_LDR:
		case MTLPixelFormatASTC_12x10_sRGB:
			pFormatSize->flags = MTL_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16 * 8;
			pFormatSize->blockWidth = 12;
			pFormatSize->blockHeight = 10;
			pFormatSize->blockDepth = 1;
			break;
		case MTLPixelFormatASTC_12x12_LDR:
		case MTLPixelFormatASTC_12x12_sRGB:
			pFormatSize->flags = MTL_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16 * 8;
			pFormatSize->blockWidth = 12;
			pFormatSize->blockHeight = 12;
			pFormatSize->blockDepth = 1;
			break;
		default:
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 0 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
	}
}

#endif // !MTL_FORMAT_H
