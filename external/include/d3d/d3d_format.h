/*
================================================================================================

Description	:	D3D format properties and conversion from OpenGL.
Author		:	J.M.P. van Waveren
Date		:	12/15/2016
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
to D3D formats. These routines are particularly useful for loading file
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
     supported by the D3D implementation.


ENTRY POINTS
============

static inline DXGI_FORMAT d3dGetFormatFromOpenGLFormat( const GLenum format, const GLenum type );
static inline DXGI_FORMAT d3dGetFormatFromOpenGLType( const GLenum type, const GLuint numComponents, const GLboolean normalized );
static inline DXGI_FORMAT d3dGetFormatFromOpenGLInternalFormat( const GLenum internalFormat );
static inline void d3dGetFormatSize( const DXGI_FORMAT format, D3dFormatSize * pFormatSize );

================================================================================================
*/

#if !defined( D3D_FORMAT_H )
#define D3D_FORMAT_H

#include "../GL/gl_format.h"

static inline DXGI_FORMAT d3dGetFormatFromOpenGLFormat( const GLenum format, const GLenum type )
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
				case GL_RED:					return DXGI_FORMAT_R8_UNORM;
				case GL_RG:						return DXGI_FORMAT_R8G8_UNORM;
				case GL_RGB:					return DXGI_FORMAT_UNKNOWN;
				case GL_BGR:					return DXGI_FORMAT_UNKNOWN;
				case GL_RGBA:					return DXGI_FORMAT_R8G8B8A8_UNORM;
				case GL_BGRA:					return DXGI_FORMAT_B8G8R8A8_UNORM;
				case GL_RED_INTEGER:			return DXGI_FORMAT_R8_UINT;
				case GL_RG_INTEGER:				return DXGI_FORMAT_R8G8_UINT;
				case GL_RGB_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_BGR_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_RGBA_INTEGER:			return DXGI_FORMAT_R8G8B8A8_UINT;
				case GL_BGRA_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_STENCIL_INDEX:			return DXGI_FORMAT_UNKNOWN;
				case GL_DEPTH_COMPONENT:		return DXGI_FORMAT_UNKNOWN;
				case GL_DEPTH_STENCIL:			return DXGI_FORMAT_UNKNOWN;
			}
			break;
		}
		case GL_BYTE:
		{
			switch ( format )
			{
				case GL_RED:					return DXGI_FORMAT_R8_SNORM;
				case GL_RG:						return DXGI_FORMAT_R8G8_SNORM;
				case GL_RGB:					return DXGI_FORMAT_UNKNOWN;
				case GL_BGR:					return DXGI_FORMAT_UNKNOWN;
				case GL_RGBA:					return DXGI_FORMAT_R8G8B8A8_SNORM;
				case GL_BGRA:					return DXGI_FORMAT_UNKNOWN;
				case GL_RED_INTEGER:			return DXGI_FORMAT_R8_SINT;
				case GL_RG_INTEGER:				return DXGI_FORMAT_R8G8_SINT;
				case GL_RGB_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_BGR_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_RGBA_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_BGRA_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_STENCIL_INDEX:			return DXGI_FORMAT_UNKNOWN;
				case GL_DEPTH_COMPONENT:		return DXGI_FORMAT_UNKNOWN;
				case GL_DEPTH_STENCIL:			return DXGI_FORMAT_UNKNOWN;
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
				case GL_RED:					return DXGI_FORMAT_R16_UNORM;
				case GL_RG:						return DXGI_FORMAT_R16G16_UNORM;
				case GL_RGB:					return DXGI_FORMAT_UNKNOWN;
				case GL_BGR:					return DXGI_FORMAT_UNKNOWN;
				case GL_RGBA:					return DXGI_FORMAT_R16G16B16A16_UNORM;
				case GL_BGRA:					return DXGI_FORMAT_UNKNOWN;
				case GL_RED_INTEGER:			return DXGI_FORMAT_R16_UINT;
				case GL_RG_INTEGER:				return DXGI_FORMAT_R16G16_UINT;
				case GL_RGB_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_BGR_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_RGBA_INTEGER:			return DXGI_FORMAT_R16G16B16A16_UINT;
				case GL_BGRA_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_STENCIL_INDEX:			return DXGI_FORMAT_UNKNOWN;
				case GL_DEPTH_COMPONENT:		return DXGI_FORMAT_D16_UNORM;
				case GL_DEPTH_STENCIL:			return DXGI_FORMAT_UNKNOWN;
			}
			break;
		}
		case GL_SHORT:
		{
			switch ( format )
			{
				case GL_RED:					return DXGI_FORMAT_R16_SNORM;
				case GL_RG:						return DXGI_FORMAT_R16G16_SNORM;
				case GL_RGB:					return DXGI_FORMAT_UNKNOWN;
				case GL_BGR:					return DXGI_FORMAT_UNKNOWN;
				case GL_RGBA:					return DXGI_FORMAT_R16G16B16A16_SNORM;
				case GL_BGRA:					return DXGI_FORMAT_UNKNOWN;
				case GL_RED_INTEGER:			return DXGI_FORMAT_R16_SINT;
				case GL_RG_INTEGER:				return DXGI_FORMAT_R16G16_SINT;
				case GL_RGB_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_BGR_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_RGBA_INTEGER:			return DXGI_FORMAT_R16G16B16A16_SINT;
				case GL_BGRA_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_STENCIL_INDEX:			return DXGI_FORMAT_UNKNOWN;
				case GL_DEPTH_COMPONENT:		return DXGI_FORMAT_UNKNOWN;
				case GL_DEPTH_STENCIL:			return DXGI_FORMAT_UNKNOWN;
			}
			break;
		}
		case GL_HALF_FLOAT:
		case GL_HALF_FLOAT_OES:
		{
			switch ( format )
			{
				case GL_RED:					return DXGI_FORMAT_R16_FLOAT;
				case GL_RG:						return DXGI_FORMAT_R16G16_FLOAT;
				case GL_RGB:					return DXGI_FORMAT_UNKNOWN;
				case GL_BGR:					return DXGI_FORMAT_UNKNOWN;
				case GL_RGBA:					return DXGI_FORMAT_R16G16B16A16_FLOAT;
				case GL_BGRA:					return DXGI_FORMAT_UNKNOWN;
				case GL_RED_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_RG_INTEGER:				return DXGI_FORMAT_UNKNOWN;
				case GL_RGB_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_BGR_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_RGBA_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_BGRA_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_STENCIL_INDEX:			return DXGI_FORMAT_UNKNOWN;
				case GL_DEPTH_COMPONENT:		return DXGI_FORMAT_UNKNOWN;
				case GL_DEPTH_STENCIL:			return DXGI_FORMAT_UNKNOWN;
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
				case GL_RED:					return DXGI_FORMAT_R32_UINT;
				case GL_RG:						return DXGI_FORMAT_R32G32_UINT;
				case GL_RGB:					return DXGI_FORMAT_R32G32B32_UINT;
				case GL_BGR:					return DXGI_FORMAT_UNKNOWN;
				case GL_RGBA:					return DXGI_FORMAT_R32G32B32A32_UINT;
				case GL_BGRA:					return DXGI_FORMAT_UNKNOWN;
				case GL_RED_INTEGER:			return DXGI_FORMAT_R32_UINT;
				case GL_RG_INTEGER:				return DXGI_FORMAT_R32G32_UINT;
				case GL_RGB_INTEGER:			return DXGI_FORMAT_R32G32B32_UINT;
				case GL_BGR_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_RGBA_INTEGER:			return DXGI_FORMAT_R32G32B32A32_UINT;
				case GL_BGRA_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_STENCIL_INDEX:			return DXGI_FORMAT_UNKNOWN;
				case GL_DEPTH_COMPONENT:		return DXGI_FORMAT_UNKNOWN;
				case GL_DEPTH_STENCIL:			return DXGI_FORMAT_D24_UNORM_S8_UINT;
			}
			break;
		}
		case GL_INT:
		{
			switch ( format )
			{
				case GL_RED:					return DXGI_FORMAT_R32_SINT;
				case GL_RG:						return DXGI_FORMAT_R32G32_SINT;
				case GL_RGB:					return DXGI_FORMAT_R32G32B32_SINT;
				case GL_BGR:					return DXGI_FORMAT_UNKNOWN;
				case GL_RGBA:					return DXGI_FORMAT_R32G32B32A32_SINT;
				case GL_BGRA:					return DXGI_FORMAT_UNKNOWN;
				case GL_RED_INTEGER:			return DXGI_FORMAT_R32_SINT;
				case GL_RG_INTEGER:				return DXGI_FORMAT_R32G32_SINT;
				case GL_RGB_INTEGER:			return DXGI_FORMAT_R32G32B32_SINT;
				case GL_BGR_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_RGBA_INTEGER:			return DXGI_FORMAT_R32G32B32A32_SINT;
				case GL_BGRA_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_STENCIL_INDEX:			return DXGI_FORMAT_UNKNOWN;
				case GL_DEPTH_COMPONENT:		return DXGI_FORMAT_UNKNOWN;
				case GL_DEPTH_STENCIL:			return DXGI_FORMAT_UNKNOWN;
			}
			break;
		}
		case GL_FLOAT:
		{
			switch ( format )
			{
				case GL_RED:					return DXGI_FORMAT_R32_FLOAT;
				case GL_RG:						return DXGI_FORMAT_R32G32_FLOAT;
				case GL_RGB:					return DXGI_FORMAT_R32G32B32_FLOAT;
				case GL_BGR:					return DXGI_FORMAT_UNKNOWN;
				case GL_RGBA:					return DXGI_FORMAT_R32G32B32A32_FLOAT;
				case GL_BGRA:					return DXGI_FORMAT_UNKNOWN;
				case GL_RED_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_RG_INTEGER:				return DXGI_FORMAT_UNKNOWN;
				case GL_RGB_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_BGR_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_RGBA_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_BGRA_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_STENCIL_INDEX:			return DXGI_FORMAT_UNKNOWN;
				case GL_DEPTH_COMPONENT:		return DXGI_FORMAT_D32_FLOAT;
				case GL_DEPTH_STENCIL:			return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
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
				case GL_RED:					return DXGI_FORMAT_UNKNOWN;
				case GL_RG:						return DXGI_FORMAT_UNKNOWN;
				case GL_RGB:					return DXGI_FORMAT_UNKNOWN;
				case GL_BGR:					return DXGI_FORMAT_UNKNOWN;
				case GL_RGBA:					return DXGI_FORMAT_UNKNOWN;
				case GL_BGRA:					return DXGI_FORMAT_UNKNOWN;
				case GL_RED_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_RG_INTEGER:				return DXGI_FORMAT_UNKNOWN;
				case GL_RGB_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_BGR_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_RGBA_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_BGRA_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_STENCIL_INDEX:			return DXGI_FORMAT_UNKNOWN;
				case GL_DEPTH_COMPONENT:		return DXGI_FORMAT_UNKNOWN;
				case GL_DEPTH_STENCIL:			return DXGI_FORMAT_UNKNOWN;
			}
			break;
		}
		case GL_INT64:
		{
			switch ( format )
			{
				case GL_RED:					return DXGI_FORMAT_UNKNOWN;
				case GL_RG:						return DXGI_FORMAT_UNKNOWN;
				case GL_RGB:					return DXGI_FORMAT_UNKNOWN;
				case GL_BGR:					return DXGI_FORMAT_UNKNOWN;
				case GL_RGBA:					return DXGI_FORMAT_UNKNOWN;
				case GL_BGRA:					return DXGI_FORMAT_UNKNOWN;
				case GL_RED_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_RG_INTEGER:				return DXGI_FORMAT_UNKNOWN;
				case GL_RGB_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_BGR_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_RGBA_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_BGRA_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_STENCIL_INDEX:			return DXGI_FORMAT_UNKNOWN;
				case GL_DEPTH_COMPONENT:		return DXGI_FORMAT_UNKNOWN;
				case GL_DEPTH_STENCIL:			return DXGI_FORMAT_UNKNOWN;
			}
			break;
		}
		case GL_DOUBLE:
		{
			switch ( format )
			{
				case GL_RED:					return DXGI_FORMAT_UNKNOWN;
				case GL_RG:						return DXGI_FORMAT_UNKNOWN;
				case GL_RGB:					return DXGI_FORMAT_UNKNOWN;
				case GL_BGR:					return DXGI_FORMAT_UNKNOWN;
				case GL_RGBA:					return DXGI_FORMAT_UNKNOWN;
				case GL_BGRA:					return DXGI_FORMAT_UNKNOWN;
				case GL_RED_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_RG_INTEGER:				return DXGI_FORMAT_UNKNOWN;
				case GL_RGB_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_BGR_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_RGBA_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_BGRA_INTEGER:			return DXGI_FORMAT_UNKNOWN;
				case GL_STENCIL_INDEX:			return DXGI_FORMAT_UNKNOWN;
				case GL_DEPTH_COMPONENT:		return DXGI_FORMAT_UNKNOWN;
				case GL_DEPTH_STENCIL:			return DXGI_FORMAT_UNKNOWN;
			}
			break;
		}

		//
		// Packed
		//
		case GL_UNSIGNED_BYTE_3_3_2:
			assert( format == GL_RGB || format == GL_RGB_INTEGER );
			return DXGI_FORMAT_UNKNOWN;
		case GL_UNSIGNED_BYTE_2_3_3_REV:
			assert( format == GL_BGR || format == GL_BGR_INTEGER );
			return DXGI_FORMAT_UNKNOWN;
		case GL_UNSIGNED_SHORT_5_6_5:
			assert( format == GL_RGB || format == GL_RGB_INTEGER );
			return DXGI_FORMAT_UNKNOWN;
		case GL_UNSIGNED_SHORT_5_6_5_REV:
			assert( format == GL_BGR || format == GL_BGR_INTEGER );
			return DXGI_FORMAT_B5G6R5_UNORM;
		case GL_UNSIGNED_SHORT_4_4_4_4:
			assert( format == GL_RGB || format == GL_BGRA || format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER );
			return DXGI_FORMAT_UNKNOWN;
		case GL_UNSIGNED_SHORT_4_4_4_4_REV:
			assert( format == GL_RGB || format == GL_BGRA || format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER );
			return DXGI_FORMAT_B4G4R4A4_UNORM;
		case GL_UNSIGNED_SHORT_5_5_5_1:
			assert( format == GL_RGB || format == GL_BGRA || format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER );
			return DXGI_FORMAT_B5G5R5A1_UNORM;
		case GL_UNSIGNED_SHORT_1_5_5_5_REV:
			assert( format == GL_RGB || format == GL_BGRA || format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER );
			return DXGI_FORMAT_B5G5R5A1_UNORM;
		case GL_UNSIGNED_INT_8_8_8_8:
			assert( format == GL_RGB || format == GL_BGRA || format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER );
			return ( format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER ) ? DXGI_FORMAT_R8G8B8A8_UINT : DXGI_FORMAT_R8G8B8A8_UNORM;
		case GL_UNSIGNED_INT_8_8_8_8_REV:
			assert( format == GL_RGB || format == GL_BGRA || format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER );
			return ( format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER ) ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_B8G8R8A8_UNORM;
		case GL_UNSIGNED_INT_10_10_10_2:
			assert( format == GL_RGB || format == GL_BGRA || format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER );
			return ( format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER ) ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_R10G10B10A2_UNORM;
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			assert( format == GL_RGB || format == GL_BGRA || format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER );
			return ( format == GL_RGB_INTEGER || format == GL_BGRA_INTEGER ) ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_R10G10B10A2_UNORM;
		case GL_UNSIGNED_INT_10F_11F_11F_REV:
			assert( format == GL_RGB || format == GL_BGR );
			return DXGI_FORMAT_UNKNOWN;
		case GL_UNSIGNED_INT_5_9_9_9_REV:
			assert( format == GL_RGB || format == GL_BGR );
			return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
		case GL_UNSIGNED_INT_24_8:
			assert( format == GL_DEPTH_STENCIL );
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			assert( format == GL_DEPTH_STENCIL );
			return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	}

	return DXGI_FORMAT_UNKNOWN;
}

static inline DXGI_FORMAT d3dGetFormatFromOpenGLType( const GLenum type, const GLuint numComponents, const GLboolean normalized )
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
				case 1:							return normalized ? DXGI_FORMAT_R8_UNORM : DXGI_FORMAT_R8_UINT;
				case 2:							return normalized ? DXGI_FORMAT_R8G8_UNORM : DXGI_FORMAT_R8G8_UINT;
				case 3:							return normalized ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_UNKNOWN;
				case 4:							return normalized ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R8G8B8A8_UINT;
			}
			break;
		}
		case GL_BYTE:
		{
			switch ( numComponents )
			{
				case 1:							return normalized ? DXGI_FORMAT_R8_SNORM : DXGI_FORMAT_R8_SINT;
				case 2:							return normalized ? DXGI_FORMAT_R8G8_SNORM : DXGI_FORMAT_R8G8_SINT;
				case 3:							return normalized ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_UNKNOWN;
				case 4:							return normalized ? DXGI_FORMAT_R8G8B8A8_SNORM : DXGI_FORMAT_R8G8B8A8_SINT;
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
				case 1:							return normalized ? DXGI_FORMAT_R16_UNORM : DXGI_FORMAT_R16_UINT;
				case 2:							return normalized ? DXGI_FORMAT_R16G16_UNORM : DXGI_FORMAT_R16G16_UINT;
				case 3:							return normalized ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_UNKNOWN;
				case 4:							return normalized ? DXGI_FORMAT_R16G16B16A16_UNORM : DXGI_FORMAT_R16G16B16A16_UINT;
			}
			break;
		}
		case GL_SHORT:
		{
			switch ( numComponents )
			{
				case 1:							return normalized ? DXGI_FORMAT_R16_SNORM : DXGI_FORMAT_R16_SINT;
				case 2:							return normalized ? DXGI_FORMAT_R16G16_SNORM : DXGI_FORMAT_R16G16_SINT;
				case 3:							return normalized ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_UNKNOWN;
				case 4:							return normalized ? DXGI_FORMAT_R16G16B16A16_SNORM : DXGI_FORMAT_R16G16B16A16_SINT;
			}
			break;
		}
		case GL_HALF_FLOAT:
		case GL_HALF_FLOAT_OES:
		{
			switch ( numComponents )
			{
				case 1:							return DXGI_FORMAT_R16_FLOAT;
				case 2:							return DXGI_FORMAT_R16G16_FLOAT;
				case 3:							return DXGI_FORMAT_UNKNOWN;
				case 4:							return DXGI_FORMAT_R16G16B16A16_FLOAT;
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
				case 1:							return DXGI_FORMAT_R32_UINT;
				case 2:							return DXGI_FORMAT_R32G32_UINT;
				case 3:							return DXGI_FORMAT_R32G32B32_UINT;
				case 4:							return DXGI_FORMAT_R32G32B32A32_UINT;
			}
			break;
		}
		case GL_INT:
		{
			switch ( numComponents )
			{
				case 1:							return DXGI_FORMAT_R32_SINT;
				case 2:							return DXGI_FORMAT_R32G32_SINT;
				case 3:							return DXGI_FORMAT_R32G32B32_SINT;
				case 4:							return DXGI_FORMAT_R32G32B32A32_SINT;
			}
			break;
		}
		case GL_FLOAT:
		{
			switch ( numComponents )
			{
				case 1:							return DXGI_FORMAT_R32_FLOAT;
				case 2:							return DXGI_FORMAT_R32G32_FLOAT;
				case 3:							return DXGI_FORMAT_R32G32B32_FLOAT;
				case 4:							return DXGI_FORMAT_R32G32B32A32_FLOAT;
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
				case 1:							return DXGI_FORMAT_UNKNOWN;
				case 2:							return DXGI_FORMAT_UNKNOWN;
				case 3:							return DXGI_FORMAT_UNKNOWN;
				case 4:							return DXGI_FORMAT_UNKNOWN;
			}
			break;
		}
		case GL_INT64:
		{
			switch ( numComponents )
			{
				case 1:							return DXGI_FORMAT_UNKNOWN;
				case 2:							return DXGI_FORMAT_UNKNOWN;
				case 3:							return DXGI_FORMAT_UNKNOWN;
				case 4:							return DXGI_FORMAT_UNKNOWN;
			}
			break;
		}
		case GL_DOUBLE:
		{
			switch ( numComponents )
			{
				case 1:							return DXGI_FORMAT_UNKNOWN;
				case 2:							return DXGI_FORMAT_UNKNOWN;
				case 3:							return DXGI_FORMAT_UNKNOWN;
				case 4:							return DXGI_FORMAT_UNKNOWN;
			}
			break;
		}

		//
		// Packed
		//
		case GL_UNSIGNED_BYTE_3_3_2:			return DXGI_FORMAT_UNKNOWN;
		case GL_UNSIGNED_BYTE_2_3_3_REV:		return DXGI_FORMAT_UNKNOWN;
		case GL_UNSIGNED_SHORT_5_6_5:			return DXGI_FORMAT_UNKNOWN;
		case GL_UNSIGNED_SHORT_5_6_5_REV:		return DXGI_FORMAT_B5G6R5_UNORM;
		case GL_UNSIGNED_SHORT_4_4_4_4:			return DXGI_FORMAT_UNKNOWN;
		case GL_UNSIGNED_SHORT_4_4_4_4_REV:		return DXGI_FORMAT_B4G4R4A4_UNORM;
		case GL_UNSIGNED_SHORT_5_5_5_1:			return DXGI_FORMAT_UNKNOWN;
		case GL_UNSIGNED_SHORT_1_5_5_5_REV:		return DXGI_FORMAT_B5G5R5A1_UNORM;
		case GL_UNSIGNED_INT_8_8_8_8:			return normalized ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R8G8B8A8_UINT;
		case GL_UNSIGNED_INT_8_8_8_8_REV:		return normalized ? DXGI_FORMAT_B8G8R8A8_UNORM : DXGI_FORMAT_UNKNOWN;
		case GL_UNSIGNED_INT_10_10_10_2:		return normalized ? DXGI_FORMAT_R10G10B10A2_UNORM : DXGI_FORMAT_R10G10B10A2_UINT;
		case GL_UNSIGNED_INT_2_10_10_10_REV:	return normalized ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_UNKNOWN;
		case GL_UNSIGNED_INT_10F_11F_11F_REV:	return DXGI_FORMAT_R11G11B10_FLOAT;
		case GL_UNSIGNED_INT_5_9_9_9_REV:		return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
		case GL_UNSIGNED_INT_24_8:				return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:	return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	}

	return DXGI_FORMAT_UNKNOWN;
}

static inline DXGI_FORMAT d3dGetFormatFromOpenGLInternalFormat( const GLenum internalFormat )
{
	switch ( internalFormat )
	{
		//
		// 8 bits per component
		//
		case GL_R8:												return DXGI_FORMAT_R8_UNORM;				// 1-component, 8-bit unsigned normalized
		case GL_RG8:											return DXGI_FORMAT_R8G8_UNORM;				// 2-component, 8-bit unsigned normalized
		case GL_RGB8:											return DXGI_FORMAT_UNKNOWN;					// 3-component, 8-bit unsigned normalized
		case GL_RGBA8:											return DXGI_FORMAT_R8G8B8A8_UNORM;			// 4-component, 8-bit unsigned normalized

		case GL_R8_SNORM:										return DXGI_FORMAT_R8_SNORM;				// 1-component, 8-bit signed normalized
		case GL_RG8_SNORM:										return DXGI_FORMAT_R8G8_SNORM;				// 2-component, 8-bit signed normalized
		case GL_RGB8_SNORM:										return DXGI_FORMAT_UNKNOWN;					// 3-component, 8-bit signed normalized
		case GL_RGBA8_SNORM:									return DXGI_FORMAT_R8G8B8A8_SNORM;			// 4-component, 8-bit signed normalized

		case GL_R8UI:											return DXGI_FORMAT_R8_UINT;					// 1-component, 8-bit unsigned integer
		case GL_RG8UI:											return DXGI_FORMAT_R8G8_UINT;				// 2-component, 8-bit unsigned integer
		case GL_RGB8UI:											return DXGI_FORMAT_UNKNOWN;					// 3-component, 8-bit unsigned integer
		case GL_RGBA8UI:										return DXGI_FORMAT_R8G8B8A8_UINT;			// 4-component, 8-bit unsigned integer

		case GL_R8I:											return DXGI_FORMAT_R8_SINT;					// 1-component, 8-bit signed integer
		case GL_RG8I:											return DXGI_FORMAT_R8G8_SINT;				// 2-component, 8-bit signed integer
		case GL_RGB8I:											return DXGI_FORMAT_UNKNOWN;					// 3-component, 8-bit signed integer
		case GL_RGBA8I:											return DXGI_FORMAT_R8G8B8A8_SINT;			// 4-component, 8-bit signed integer

		case GL_SR8:											return DXGI_FORMAT_UNKNOWN;					// 1-component, 8-bit sRGB
		case GL_SRG8:											return DXGI_FORMAT_UNKNOWN;					// 2-component, 8-bit sRGB
		case GL_SRGB8:											return DXGI_FORMAT_UNKNOWN;					// 3-component, 8-bit sRGB
		case GL_SRGB8_ALPHA8:									return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;		// 4-component, 8-bit sRGB

		//
		// 16 bits per component
		//
		case GL_R16:											return DXGI_FORMAT_R16_UNORM;				// 1-component, 16-bit unsigned normalized
		case GL_RG16:											return DXGI_FORMAT_R16G16_UNORM;			// 2-component, 16-bit unsigned normalized
		case GL_RGB16:											return DXGI_FORMAT_UNKNOWN;					// 3-component, 16-bit unsigned normalized
		case GL_RGBA16:											return DXGI_FORMAT_R16G16B16A16_UNORM;		// 4-component, 16-bit unsigned normalized

		case GL_R16_SNORM:										return DXGI_FORMAT_R16_SNORM;				// 1-component, 16-bit signed normalized
		case GL_RG16_SNORM:										return DXGI_FORMAT_R16G16_SNORM;			// 2-component, 16-bit signed normalized
		case GL_RGB16_SNORM:									return DXGI_FORMAT_UNKNOWN;					// 3-component, 16-bit signed normalized
		case GL_RGBA16_SNORM:									return DXGI_FORMAT_R16G16B16A16_SNORM;		// 4-component, 16-bit signed normalized

		case GL_R16UI:											return DXGI_FORMAT_R16_UINT;				// 1-component, 16-bit unsigned integer
		case GL_RG16UI:											return DXGI_FORMAT_R16G16_UINT;				// 2-component, 16-bit unsigned integer
		case GL_RGB16UI:										return DXGI_FORMAT_UNKNOWN;					// 3-component, 16-bit unsigned integer
		case GL_RGBA16UI:										return DXGI_FORMAT_R16G16B16A16_UINT;		// 4-component, 16-bit unsigned integer

		case GL_R16I:											return DXGI_FORMAT_R16_SINT;				// 1-component, 16-bit signed integer
		case GL_RG16I:											return DXGI_FORMAT_R16G16_SINT;				// 2-component, 16-bit signed integer
		case GL_RGB16I:											return DXGI_FORMAT_UNKNOWN;					// 3-component, 16-bit signed integer
		case GL_RGBA16I:										return DXGI_FORMAT_R16G16B16A16_SINT;		// 4-component, 16-bit signed integer

		case GL_R16F:											return DXGI_FORMAT_R16_FLOAT;				// 1-component, 16-bit floating-point
		case GL_RG16F:											return DXGI_FORMAT_R16G16_FLOAT;			// 2-component, 16-bit floating-point
		case GL_RGB16F:											return DXGI_FORMAT_UNKNOWN;					// 3-component, 16-bit floating-point
		case GL_RGBA16F:										return DXGI_FORMAT_R16G16B16A16_FLOAT;		// 4-component, 16-bit floating-point

		//
		// 32 bits per component
		//
		case GL_R32UI:											return DXGI_FORMAT_R32_UINT;				// 1-component, 32-bit unsigned integer
		case GL_RG32UI:											return DXGI_FORMAT_R32G32_UINT;				// 2-component, 32-bit unsigned integer
		case GL_RGB32UI:										return DXGI_FORMAT_R32G32B32_UINT;			// 3-component, 32-bit unsigned integer
		case GL_RGBA32UI:										return DXGI_FORMAT_R32G32B32A32_UINT;		// 4-component, 32-bit unsigned integer

		case GL_R32I:											return DXGI_FORMAT_R32_SINT;				// 1-component, 32-bit signed integer
		case GL_RG32I:											return DXGI_FORMAT_R32G32_SINT;				// 2-component, 32-bit signed integer
		case GL_RGB32I:											return DXGI_FORMAT_R32G32B32_SINT;			// 3-component, 32-bit signed integer
		case GL_RGBA32I:										return DXGI_FORMAT_R32G32B32A32_SINT;		// 4-component, 32-bit signed integer

		case GL_R32F:											return DXGI_FORMAT_R32_FLOAT;				// 1-component, 32-bit floating-point
		case GL_RG32F:											return DXGI_FORMAT_R32G32_FLOAT;			// 2-component, 32-bit floating-point
		case GL_RGB32F:											return DXGI_FORMAT_R32G32B32_FLOAT;			// 3-component, 32-bit floating-point
		case GL_RGBA32F:										return DXGI_FORMAT_R32G32B32A32_FLOAT;		// 4-component, 32-bit floating-point

		//
		// Packed
		//
		case GL_R3_G3_B2:										return DXGI_FORMAT_UNKNOWN;					// 3-component 3:3:2,       unsigned normalized
		case GL_RGB4:											return DXGI_FORMAT_UNKNOWN;					// 3-component 4:4:4,       unsigned normalized
		case GL_RGB5:											return DXGI_FORMAT_B5G5R5A1_UNORM;			// 3-component 5:5:5,       unsigned normalized
		case GL_RGB565:											return DXGI_FORMAT_B5G6R5_UNORM;			// 3-component 5:6:5,       unsigned normalized
		case GL_RGB10:											return DXGI_FORMAT_R10G10B10A2_UNORM;		// 3-component 10:10:10,    unsigned normalized
		case GL_RGB12:											return DXGI_FORMAT_UNKNOWN;					// 3-component 12:12:12,    unsigned normalized
		case GL_RGBA2:											return DXGI_FORMAT_UNKNOWN;					// 4-component 2:2:2:2,     unsigned normalized
		case GL_RGBA4:											return DXGI_FORMAT_B4G4R4A4_UNORM;			// 4-component 4:4:4:4,     unsigned normalized
		case GL_RGBA12:											return DXGI_FORMAT_UNKNOWN;					// 4-component 12:12:12:12, unsigned normalized
		case GL_RGB5_A1:										return DXGI_FORMAT_B5G5R5A1_UNORM;			// 4-component 5:5:5:1,     unsigned normalized
		case GL_RGB10_A2:										return DXGI_FORMAT_R10G10B10A2_UNORM;		// 4-component 10:10:10:2,  unsigned normalized
		case GL_RGB10_A2UI:										return DXGI_FORMAT_R10G10B10A2_UINT;		// 4-component 10:10:10:2,  unsigned integer
		case GL_R11F_G11F_B10F:									return DXGI_FORMAT_R11G11B10_FLOAT;			// 3-component 11:11:10,    floating-point
		case GL_RGB9_E5:										return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;		// 3-component/exp 9:9:9/5, floating-point

		//
		// S3TC/DXT/BC
		//

		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:					return DXGI_FORMAT_BC1_UNORM;				// line through 3D space, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:					return DXGI_FORMAT_BC1_UNORM;				// line through 3D space plus 1-bit alpha, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:					return DXGI_FORMAT_BC2_UNORM;				// line through 3D space plus line through 1D space, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:					return DXGI_FORMAT_BC3_UNORM;				// line through 3D space plus 4-bit alpha, 4x4 blocks, unsigned normalized

		case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:					return DXGI_FORMAT_BC1_UNORM_SRGB;			// line through 3D space, 4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:			return DXGI_FORMAT_BC1_UNORM_SRGB;			// line through 3D space plus 1-bit alpha, 4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:			return DXGI_FORMAT_BC2_UNORM_SRGB;			// line through 3D space plus line through 1D space, 4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:			return DXGI_FORMAT_BC3_UNORM_SRGB;			// line through 3D space plus 4-bit alpha, 4x4 blocks, sRGB

		case GL_COMPRESSED_LUMINANCE_LATC1_EXT:					return DXGI_FORMAT_BC4_UNORM;				// line through 1D space, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:			return DXGI_FORMAT_BC5_UNORM;				// two lines through 1D space, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:			return DXGI_FORMAT_BC4_SNORM;				// line through 1D space, 4x4 blocks, signed normalized
		case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:	return DXGI_FORMAT_BC5_SNORM;				// two lines through 1D space, 4x4 blocks, signed normalized

		case GL_COMPRESSED_RED_RGTC1:							return DXGI_FORMAT_BC4_UNORM;				// line through 1D space, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RG_RGTC2:							return DXGI_FORMAT_BC5_UNORM;				// two lines through 1D space, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_SIGNED_RED_RGTC1:					return DXGI_FORMAT_BC4_SNORM;				// line through 1D space, 4x4 blocks, signed normalized
		case GL_COMPRESSED_SIGNED_RG_RGTC2:						return DXGI_FORMAT_BC5_SNORM;				// two lines through 1D space, 4x4 blocks, signed normalized

		case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:				return DXGI_FORMAT_BC6H_UF16;				// 3-component, 4x4 blocks, unsigned floating-point
		case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:				return DXGI_FORMAT_BC6H_SF16;				// 3-component, 4x4 blocks, signed floating-point
		case GL_COMPRESSED_RGBA_BPTC_UNORM:						return DXGI_FORMAT_BC7_UNORM;				// 4-component, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:				return DXGI_FORMAT_BC7_UNORM_SRGB;			// 4-component, 4x4 blocks, sRGB

		//
		// ETC
		//
		case GL_ETC1_RGB8_OES:									return DXGI_FORMAT_UNKNOWN;					// 3-component ETC1, 4x4 blocks, unsigned normalized

		case GL_COMPRESSED_RGB8_ETC2:							return DXGI_FORMAT_UNKNOWN;					// 3-component ETC2, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:		return DXGI_FORMAT_UNKNOWN;					// 4-component ETC2 with 1-bit alpha, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA8_ETC2_EAC:						return DXGI_FORMAT_UNKNOWN;					// 4-component ETC2, 4x4 blocks, unsigned normalized

		case GL_COMPRESSED_SRGB8_ETC2:							return DXGI_FORMAT_UNKNOWN;					// 3-component ETC2, 4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:		return DXGI_FORMAT_UNKNOWN;					// 4-component ETC2 with 1-bit alpha, 4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:				return DXGI_FORMAT_UNKNOWN;					// 4-component ETC2, 4x4 blocks, sRGB

		case GL_COMPRESSED_R11_EAC:								return DXGI_FORMAT_UNKNOWN;					// 1-component ETC, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RG11_EAC:							return DXGI_FORMAT_UNKNOWN;					// 2-component ETC, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_SIGNED_R11_EAC:						return DXGI_FORMAT_UNKNOWN;					// 1-component ETC, 4x4 blocks, signed normalized
		case GL_COMPRESSED_SIGNED_RG11_EAC:						return DXGI_FORMAT_UNKNOWN;					// 2-component ETC, 4x4 blocks, signed normalized

		//
		// PVRTC
		//
		case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:				return DXGI_FORMAT_UNKNOWN;					// 3-component PVRTC, 16x8 blocks, unsigned normalized
		case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:				return DXGI_FORMAT_UNKNOWN;					// 3-component PVRTC,  8x8 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:				return DXGI_FORMAT_UNKNOWN;					// 4-component PVRTC, 16x8 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:				return DXGI_FORMAT_UNKNOWN;					// 4-component PVRTC,  8x8 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG:				return DXGI_FORMAT_UNKNOWN;					// 4-component PVRTC,  8x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG:				return DXGI_FORMAT_UNKNOWN;					// 4-component PVRTC,  4x4 blocks, unsigned normalized

		case GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT:				return DXGI_FORMAT_UNKNOWN;					// 3-component PVRTC, 16x8 blocks, sRGB
		case GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT:				return DXGI_FORMAT_UNKNOWN;					// 3-component PVRTC,  8x8 blocks, sRGB
		case GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT:			return DXGI_FORMAT_UNKNOWN;					// 4-component PVRTC, 16x8 blocks, sRGB
		case GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT:			return DXGI_FORMAT_UNKNOWN;					// 4-component PVRTC,  8x8 blocks, sRGB
		case GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG:			return DXGI_FORMAT_UNKNOWN;					// 4-component PVRTC,  8x4 blocks, sRGB
		case GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG:			return DXGI_FORMAT_UNKNOWN;					// 4-component PVRTC,  4x4 blocks, sRGB

		//
		// ASTC
		//
		case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_5x4_KHR:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 5x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_5x5_KHR:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 5x5 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_6x5_KHR:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 6x5 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_6x6_KHR:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 6x6 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_8x5_KHR:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 8x5 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_8x6_KHR:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 8x6 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 8x8 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_10x5_KHR:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 10x5 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_10x6_KHR:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 10x6 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_10x8_KHR:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 10x8 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_10x10_KHR:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 10x10 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_12x10_KHR:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 12x10 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_12x12_KHR:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 12x12 blocks, unsigned normalized

		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 5x4 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 5x5 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 6x5 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 6x6 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 8x5 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 8x6 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 8x8 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 10x5 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 10x6 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 10x8 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 10x10 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 12x10 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 12x12 blocks, sRGB

		case GL_COMPRESSED_RGBA_ASTC_3x3x3_OES:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 3x3x3 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_4x3x3_OES:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 4x3x3 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_4x4x3_OES:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 4x4x3 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_4x4x4_OES:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 4x4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_5x4x4_OES:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 5x4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_5x5x4_OES:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 5x5x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_5x5x5_OES:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 5x5x5 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_6x5x5_OES:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 6x5x5 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_6x6x5_OES:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 6x6x5 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_6x6x6_OES:					return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 6x6x6 blocks, unsigned normalized

		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 3x3x3 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 4x3x3 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 4x4x3 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 4x4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 5x4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 5x5x4 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 5x5x5 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 6x5x5 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 6x6x5 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES:			return DXGI_FORMAT_UNKNOWN;					// 4-component ASTC, 6x6x6 blocks, sRGB

		//
		// ATC
		//
		case GL_ATC_RGB_AMD:									return DXGI_FORMAT_UNKNOWN;					// 3-component, 4x4 blocks, unsigned normalized
		case GL_ATC_RGBA_EXPLICIT_ALPHA_AMD:					return DXGI_FORMAT_UNKNOWN;					// 4-component, 4x4 blocks, unsigned normalized
		case GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD:				return DXGI_FORMAT_UNKNOWN;					// 4-component, 4x4 blocks, unsigned normalized

		//
		// Palletized
		//
		case GL_PALETTE4_RGB8_OES:								return DXGI_FORMAT_UNKNOWN;					// 3-component 8:8:8,   4-bit palette, unsigned normalized
		case GL_PALETTE4_RGBA8_OES:								return DXGI_FORMAT_UNKNOWN;					// 4-component 8:8:8:8, 4-bit palette, unsigned normalized
		case GL_PALETTE4_R5_G6_B5_OES:							return DXGI_FORMAT_UNKNOWN;					// 3-component 5:6:5,   4-bit palette, unsigned normalized
		case GL_PALETTE4_RGBA4_OES:								return DXGI_FORMAT_UNKNOWN;					// 4-component 4:4:4:4, 4-bit palette, unsigned normalized
		case GL_PALETTE4_RGB5_A1_OES:							return DXGI_FORMAT_UNKNOWN;					// 4-component 5:5:5:1, 4-bit palette, unsigned normalized
		case GL_PALETTE8_RGB8_OES:								return DXGI_FORMAT_UNKNOWN;					// 3-component 8:8:8,   8-bit palette, unsigned normalized
		case GL_PALETTE8_RGBA8_OES:								return DXGI_FORMAT_UNKNOWN;					// 4-component 8:8:8:8, 8-bit palette, unsigned normalized
		case GL_PALETTE8_R5_G6_B5_OES:							return DXGI_FORMAT_UNKNOWN;					// 3-component 5:6:5,   8-bit palette, unsigned normalized
		case GL_PALETTE8_RGBA4_OES:								return DXGI_FORMAT_UNKNOWN;					// 4-component 4:4:4:4, 8-bit palette, unsigned normalized
		case GL_PALETTE8_RGB5_A1_OES:							return DXGI_FORMAT_UNKNOWN;					// 4-component 5:5:5:1, 8-bit palette, unsigned normalized

		//
		// Depth/stencil
		//
		case GL_DEPTH_COMPONENT16:								return DXGI_FORMAT_D16_UNORM;
		case GL_DEPTH_COMPONENT24:								return DXGI_FORMAT_UNKNOWN;
		case GL_DEPTH_COMPONENT32:								return DXGI_FORMAT_UNKNOWN;
		case GL_DEPTH_COMPONENT32F:								return DXGI_FORMAT_D32_FLOAT;
		case GL_DEPTH_COMPONENT32F_NV:							return DXGI_FORMAT_D32_FLOAT;
		case GL_STENCIL_INDEX1:									return DXGI_FORMAT_UNKNOWN;
		case GL_STENCIL_INDEX4:									return DXGI_FORMAT_UNKNOWN;
		case GL_STENCIL_INDEX8:									return DXGI_FORMAT_UNKNOWN;
		case GL_STENCIL_INDEX16:								return DXGI_FORMAT_UNKNOWN;
		case GL_DEPTH24_STENCIL8:								return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case GL_DEPTH32F_STENCIL8:								return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		case GL_DEPTH32F_STENCIL8_NV:							return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

		default:												return DXGI_FORMAT_UNKNOWN;
	}
}

typedef enum D3dFormatSizeFlagBits {
	DXGI_FORMAT_SIZE_PACKED_BIT				= 0x00000001,
	DXGI_FORMAT_SIZE_COMPRESSED_BIT			= 0x00000002,
	DXGI_FORMAT_SIZE_PALETTIZED_BIT			= 0x00000004,
	DXGI_FORMAT_SIZE_DEPTH_BIT				= 0x00000008,
	DXGI_FORMAT_SIZE_STENCIL_BIT			= 0x00000010,
} D3dFormatSizeFlagBits;

typedef unsigned int D3dFormatSizeFlags;

typedef struct D3dFormatSize {
	D3dFormatSizeFlags	flags;
	unsigned int		paletteSizeInBits;
	unsigned int		blockSizeInBits;
	unsigned int		blockWidth;			// in texels
	unsigned int		blockHeight;		// in texels
	unsigned int		blockDepth;			// in texels
} D3dFormatSize;

static inline void d3dGetFormatSize( const DXGI_FORMAT format, D3dFormatSize * pFormatSize )
{
	switch ( format )
	{
		case DXGI_FORMAT_B4G4R4A4_UNORM:
		case DXGI_FORMAT_B5G6R5_UNORM:
		case DXGI_FORMAT_B5G5R5A1_UNORM:
			pFormatSize->flags = DXGI_FORMAT_SIZE_PACKED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 2 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case DXGI_FORMAT_R8_UNORM:
		case DXGI_FORMAT_R8_SNORM:
		case DXGI_FORMAT_R8_UINT:
		case DXGI_FORMAT_R8_SINT:
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 1 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case DXGI_FORMAT_R8G8_UNORM:
		case DXGI_FORMAT_R8G8_SNORM:
		case DXGI_FORMAT_R8G8_UINT:
		case DXGI_FORMAT_R8G8_SINT:
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 2 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SINT:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 4 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case DXGI_FORMAT_R10G10B10A2_UNORM:
		case DXGI_FORMAT_R10G10B10A2_UINT:
			pFormatSize->flags = DXGI_FORMAT_SIZE_PACKED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 4 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case DXGI_FORMAT_R16_UNORM:
		case DXGI_FORMAT_R16_SNORM:
		case DXGI_FORMAT_R16_UINT:
		case DXGI_FORMAT_R16_SINT:
		case DXGI_FORMAT_R16_FLOAT:
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 2 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case DXGI_FORMAT_R16G16_UNORM:
		case DXGI_FORMAT_R16G16_SNORM:
		case DXGI_FORMAT_R16G16_UINT:
		case DXGI_FORMAT_R16G16_SINT:
		case DXGI_FORMAT_R16G16_FLOAT:
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 4 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case DXGI_FORMAT_R16G16B16A16_UNORM:
		case DXGI_FORMAT_R16G16B16A16_SNORM:
		case DXGI_FORMAT_R16G16B16A16_UINT:
		case DXGI_FORMAT_R16G16B16A16_SINT:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 8 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case DXGI_FORMAT_R32_UINT:
		case DXGI_FORMAT_R32_SINT:
		case DXGI_FORMAT_R32_FLOAT:
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 4 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case DXGI_FORMAT_R32G32_UINT:
		case DXGI_FORMAT_R32G32_SINT:
		case DXGI_FORMAT_R32G32_FLOAT:
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 8 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case DXGI_FORMAT_R32G32B32_UINT:
		case DXGI_FORMAT_R32G32B32_SINT:
		case DXGI_FORMAT_R32G32B32_FLOAT:
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 12 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case DXGI_FORMAT_R32G32B32A32_UINT:
		case DXGI_FORMAT_R32G32B32A32_SINT:
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case DXGI_FORMAT_R11G11B10_FLOAT:
		case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
			pFormatSize->flags = DXGI_FORMAT_SIZE_PACKED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 4 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case DXGI_FORMAT_D16_UNORM:
			pFormatSize->flags = DXGI_FORMAT_SIZE_DEPTH_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 2 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case DXGI_FORMAT_D32_FLOAT:
			pFormatSize->flags = DXGI_FORMAT_SIZE_DEPTH_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 4 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			pFormatSize->flags = DXGI_FORMAT_SIZE_DEPTH_BIT | DXGI_FORMAT_SIZE_STENCIL_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 4 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			pFormatSize->flags = DXGI_FORMAT_SIZE_DEPTH_BIT | DXGI_FORMAT_SIZE_STENCIL_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 8 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
			pFormatSize->flags = DXGI_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 8 * 8;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC4_SNORM:
		case DXGI_FORMAT_BC5_UNORM:
		case DXGI_FORMAT_BC5_SNORM:
		case DXGI_FORMAT_BC6H_UF16:
		case DXGI_FORMAT_BC6H_SF16:
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			pFormatSize->flags = DXGI_FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16 * 8;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
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

#endif // !D3D_FORMAT_H
