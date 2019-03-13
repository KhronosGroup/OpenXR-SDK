//
//  MTLTypes.h
//  Metal
//
//  Copyright (c) 2014 Apple Inc. All rights reserved.
//
// https://github.com/mstg/iOS-full-sdk/blob/master/iPhoneOS9.3.sdk/System/Library/Frameworks/Metal.framework/Headers/MTLPixelFormat.h
// https://github.com/mstg/iOS-full-sdk/blob/master/iPhoneOS9.3.sdk/System/Library/Frameworks/Metal.framework/Headers/MTLVertexDescriptor.h
// https://github.com/mstg/iOS-full-sdk/blob/master/iPhoneOS9.3.sdk/System/Library/Frameworks/Metal.framework/Headers/MTLRenderPipeline.h
// https://github.com/mstg/iOS-full-sdk/blob/master/iPhoneOS9.3.sdk/System/Library/Frameworks/Metal.framework/Headers/MTLRenderCommandEncoder.h
//

typedef unsigned int NSError;

#define NS_AVAILABLE_IOS( version )
#define NS_AVAILABLE_MAC( version )

typedef enum 
{
	MTLPixelFormatInvalid = 0,

	/* Normal 8 bit formats */
    
	MTLPixelFormatA8Unorm      = 1,
    
	MTLPixelFormatR8Unorm                            = 10,
	MTLPixelFormatR8Unorm_sRGB NS_AVAILABLE_IOS(8_0) = 11,

	MTLPixelFormatR8Snorm      = 12,
	MTLPixelFormatR8Uint       = 13,
	MTLPixelFormatR8Sint       = 14,
    
	/* Normal 16 bit formats */

	MTLPixelFormatR16Unorm     = 20,
	MTLPixelFormatR16Snorm     = 22,
	MTLPixelFormatR16Uint      = 23,
	MTLPixelFormatR16Sint      = 24,
	MTLPixelFormatR16Float     = 25,

	MTLPixelFormatRG8Unorm                            = 30,
	MTLPixelFormatRG8Unorm_sRGB NS_AVAILABLE_IOS(8_0) = 31,
	MTLPixelFormatRG8Snorm                            = 32,
	MTLPixelFormatRG8Uint                             = 33,
	MTLPixelFormatRG8Sint                             = 34,

	/* Packed 16 bit formats */
    
	MTLPixelFormatB5G6R5Unorm NS_AVAILABLE_IOS(8_0) = 40,
	MTLPixelFormatA1BGR5Unorm NS_AVAILABLE_IOS(8_0) = 41,
	MTLPixelFormatABGR4Unorm  NS_AVAILABLE_IOS(8_0) = 42,
	MTLPixelFormatBGR5A1Unorm NS_AVAILABLE_IOS(8_0) = 43,

	/* Normal 32 bit formats */

	MTLPixelFormatR32Uint  = 53,
	MTLPixelFormatR32Sint  = 54,
	MTLPixelFormatR32Float = 55,

	MTLPixelFormatRG16Unorm  = 60,
	MTLPixelFormatRG16Snorm  = 62,
	MTLPixelFormatRG16Uint   = 63,
	MTLPixelFormatRG16Sint   = 64,
	MTLPixelFormatRG16Float  = 65,

	MTLPixelFormatRGBA8Unorm      = 70,
	MTLPixelFormatRGBA8Unorm_sRGB = 71,
	MTLPixelFormatRGBA8Snorm      = 72,
	MTLPixelFormatRGBA8Uint       = 73,
	MTLPixelFormatRGBA8Sint       = 74,

	MTLPixelFormatBGRA8Unorm      = 80,
	MTLPixelFormatBGRA8Unorm_sRGB = 81,

	/* Packed 32 bit formats */

	MTLPixelFormatRGB10A2Unorm = 90,
	MTLPixelFormatRGB10A2Uint  = 91,

	MTLPixelFormatRG11B10Float = 92,
	MTLPixelFormatRGB9E5Float = 93,

	/* Normal 64 bit formats */

	MTLPixelFormatRG32Uint  = 103,
	MTLPixelFormatRG32Sint  = 104,
	MTLPixelFormatRG32Float = 105,

	MTLPixelFormatRGBA16Unorm  = 110,
	MTLPixelFormatRGBA16Snorm  = 112,
	MTLPixelFormatRGBA16Uint   = 113,
	MTLPixelFormatRGBA16Sint   = 114,
	MTLPixelFormatRGBA16Float  = 115,

	/* Normal 128 bit formats */

	MTLPixelFormatRGBA32Uint  = 123,
	MTLPixelFormatRGBA32Sint  = 124,
	MTLPixelFormatRGBA32Float = 125,

	/* Compressed formats. */

	/* S3TC/DXT */
	MTLPixelFormatBC1_RGBA              NS_AVAILABLE_MAC(10_11) = 130,
	MTLPixelFormatBC1_RGBA_sRGB         NS_AVAILABLE_MAC(10_11) = 131,
	MTLPixelFormatBC2_RGBA              NS_AVAILABLE_MAC(10_11) = 132,
	MTLPixelFormatBC2_RGBA_sRGB         NS_AVAILABLE_MAC(10_11) = 133,
	MTLPixelFormatBC3_RGBA              NS_AVAILABLE_MAC(10_11) = 134,
	MTLPixelFormatBC3_RGBA_sRGB         NS_AVAILABLE_MAC(10_11) = 135,

	/* RGTC */
	MTLPixelFormatBC4_RUnorm            NS_AVAILABLE_MAC(10_11) = 140,
	MTLPixelFormatBC4_RSnorm            NS_AVAILABLE_MAC(10_11) = 141,
	MTLPixelFormatBC5_RGUnorm           NS_AVAILABLE_MAC(10_11) = 142,
	MTLPixelFormatBC5_RGSnorm           NS_AVAILABLE_MAC(10_11) = 143,

	/* BPTC */
	MTLPixelFormatBC6H_RGBFloat         NS_AVAILABLE_MAC(10_11) = 150,
	MTLPixelFormatBC6H_RGBUfloat        NS_AVAILABLE_MAC(10_11) = 151,
	MTLPixelFormatBC7_RGBAUnorm         NS_AVAILABLE_MAC(10_11) = 152,
	MTLPixelFormatBC7_RGBAUnorm_sRGB    NS_AVAILABLE_MAC(10_11) = 153,

	/* PVRTC */
	MTLPixelFormatPVRTC_RGB_2BPP        NS_AVAILABLE_IOS(8_0) = 160,
	MTLPixelFormatPVRTC_RGB_2BPP_sRGB   NS_AVAILABLE_IOS(8_0) = 161,
	MTLPixelFormatPVRTC_RGB_4BPP        NS_AVAILABLE_IOS(8_0) = 162,
	MTLPixelFormatPVRTC_RGB_4BPP_sRGB   NS_AVAILABLE_IOS(8_0) = 163,
	MTLPixelFormatPVRTC_RGBA_2BPP       NS_AVAILABLE_IOS(8_0) = 164,
	MTLPixelFormatPVRTC_RGBA_2BPP_sRGB  NS_AVAILABLE_IOS(8_0) = 165,
	MTLPixelFormatPVRTC_RGBA_4BPP       NS_AVAILABLE_IOS(8_0) = 166,
	MTLPixelFormatPVRTC_RGBA_4BPP_sRGB  NS_AVAILABLE_IOS(8_0) = 167,

	/* ETC2 */
	MTLPixelFormatEAC_R11Unorm          NS_AVAILABLE_IOS(8_0) = 170,
	MTLPixelFormatEAC_R11Snorm          NS_AVAILABLE_IOS(8_0) = 172,
	MTLPixelFormatEAC_RG11Unorm         NS_AVAILABLE_IOS(8_0) = 174,
	MTLPixelFormatEAC_RG11Snorm         NS_AVAILABLE_IOS(8_0) = 176,
	MTLPixelFormatEAC_RGBA8             NS_AVAILABLE_IOS(8_0) = 178,
	MTLPixelFormatEAC_RGBA8_sRGB        NS_AVAILABLE_IOS(8_0) = 179,

	MTLPixelFormatETC2_RGB8             NS_AVAILABLE_IOS(8_0) = 180,
	MTLPixelFormatETC2_RGB8_sRGB        NS_AVAILABLE_IOS(8_0) = 181,
	MTLPixelFormatETC2_RGB8A1           NS_AVAILABLE_IOS(8_0) = 182,
	MTLPixelFormatETC2_RGB8A1_sRGB      NS_AVAILABLE_IOS(8_0) = 183,

	/* ASTC */
	MTLPixelFormatASTC_4x4_sRGB         NS_AVAILABLE_IOS(8_0) = 186,
	MTLPixelFormatASTC_5x4_sRGB         NS_AVAILABLE_IOS(8_0) = 187,
	MTLPixelFormatASTC_5x5_sRGB         NS_AVAILABLE_IOS(8_0) = 188,
	MTLPixelFormatASTC_6x5_sRGB         NS_AVAILABLE_IOS(8_0) = 189,
	MTLPixelFormatASTC_6x6_sRGB         NS_AVAILABLE_IOS(8_0) = 190,
	MTLPixelFormatASTC_8x5_sRGB         NS_AVAILABLE_IOS(8_0) = 192,
	MTLPixelFormatASTC_8x6_sRGB         NS_AVAILABLE_IOS(8_0) = 193,
	MTLPixelFormatASTC_8x8_sRGB         NS_AVAILABLE_IOS(8_0) = 194,
	MTLPixelFormatASTC_10x5_sRGB        NS_AVAILABLE_IOS(8_0) = 195,
	MTLPixelFormatASTC_10x6_sRGB        NS_AVAILABLE_IOS(8_0) = 196,
	MTLPixelFormatASTC_10x8_sRGB        NS_AVAILABLE_IOS(8_0) = 197,
	MTLPixelFormatASTC_10x10_sRGB       NS_AVAILABLE_IOS(8_0) = 198,
	MTLPixelFormatASTC_12x10_sRGB       NS_AVAILABLE_IOS(8_0) = 199,
	MTLPixelFormatASTC_12x12_sRGB       NS_AVAILABLE_IOS(8_0) = 200,

	MTLPixelFormatASTC_4x4_LDR          NS_AVAILABLE_IOS(8_0) = 204,
	MTLPixelFormatASTC_5x4_LDR          NS_AVAILABLE_IOS(8_0) = 205,
	MTLPixelFormatASTC_5x5_LDR          NS_AVAILABLE_IOS(8_0) = 206,
	MTLPixelFormatASTC_6x5_LDR          NS_AVAILABLE_IOS(8_0) = 207,
	MTLPixelFormatASTC_6x6_LDR          NS_AVAILABLE_IOS(8_0) = 208,
	MTLPixelFormatASTC_8x5_LDR          NS_AVAILABLE_IOS(8_0) = 210,
	MTLPixelFormatASTC_8x6_LDR          NS_AVAILABLE_IOS(8_0) = 211,
	MTLPixelFormatASTC_8x8_LDR          NS_AVAILABLE_IOS(8_0) = 212,
	MTLPixelFormatASTC_10x5_LDR         NS_AVAILABLE_IOS(8_0) = 213,
	MTLPixelFormatASTC_10x6_LDR         NS_AVAILABLE_IOS(8_0) = 214,
	MTLPixelFormatASTC_10x8_LDR         NS_AVAILABLE_IOS(8_0) = 215,
	MTLPixelFormatASTC_10x10_LDR        NS_AVAILABLE_IOS(8_0) = 216,
	MTLPixelFormatASTC_12x10_LDR        NS_AVAILABLE_IOS(8_0) = 217,
	MTLPixelFormatASTC_12x12_LDR        NS_AVAILABLE_IOS(8_0) = 218,

	/*!
		@constant MTLPixelFormatGBGR422
		@abstract A pixel format where the red and green channels are subsampled horizontally.  Two pixels are stored in 32 bits, with shared red and blue values, and unique green values.
		@discussion This format is equivelent to YUY2, YUYV, yuvs, or GL_RGB_422_APPLE/GL_UNSIGNED_SHORT_8_8_REV_APPLE.   The component order, from lowest addressed byte to highest, is Y0, Cb, Y1, Cr.  There is no implicit colorspace conversion from YUV to RGB, the shader will receive (Cr, Y, Cb, 1).  422 textures must have a width that is a multiple of 2, and can only be used for 2D non-mipmap textures.  When sampling, ClampToEdge is the only usable wrap mode.
		*/
	MTLPixelFormatGBGR422 = 240,

	/*!
		@constant MTLPixelFormatBGRG422
		@abstract A pixel format where the red and green channels are subsampled horizontally.  Two pixels are stored in 32 bits, with shared red and blue values, and unique green values.
		@discussion This format is equivelent to UYVY, 2vuy, or GL_RGB_422_APPLE/GL_UNSIGNED_SHORT_8_8_APPLE. The component order, from lowest addressed byte to highest, is Cb, Y0, Cr, Y1.  There is no implicit colorspace conversion from YUV to RGB, the shader will receive (Cr, Y, Cb, 1).  422 textures must have a width that is a multiple of 2, and can only be used for 2D non-mipmap textures.  When sampling, ClampToEdge is the only usable wrap mode.
		*/
	MTLPixelFormatBGRG422 = 241,

	/* Depth */

	MTLPixelFormatDepth32Float  = 252,

	/* Stencil */

	MTLPixelFormatStencil8        = 253,

	/* Depth Stencil */
    
	MTLPixelFormatDepth24Unorm_Stencil8  NS_AVAILABLE_MAC(10_11) = 255,
	MTLPixelFormatDepth32Float_Stencil8  NS_AVAILABLE_MAC(10_11) NS_AVAILABLE_IOS(9_0) = 260,
} MTLPixelFormat;

typedef enum
{
	MTLVertexFormatInvalid = 0,
    
	MTLVertexFormatUChar2 = 1,
	MTLVertexFormatUChar3 = 2,
	MTLVertexFormatUChar4 = 3,
    
	MTLVertexFormatChar2 = 4,
	MTLVertexFormatChar3 = 5,
	MTLVertexFormatChar4 = 6,
    
	MTLVertexFormatUChar2Normalized = 7,
	MTLVertexFormatUChar3Normalized = 8,
	MTLVertexFormatUChar4Normalized = 9,
    
	MTLVertexFormatChar2Normalized = 10,
	MTLVertexFormatChar3Normalized = 11,
	MTLVertexFormatChar4Normalized = 12,
    
	MTLVertexFormatUShort2 = 13,
	MTLVertexFormatUShort3 = 14,
	MTLVertexFormatUShort4 = 15,
    
	MTLVertexFormatShort2 = 16,
	MTLVertexFormatShort3 = 17,
	MTLVertexFormatShort4 = 18,
    
	MTLVertexFormatUShort2Normalized = 19,
	MTLVertexFormatUShort3Normalized = 20,
	MTLVertexFormatUShort4Normalized = 21,
    
	MTLVertexFormatShort2Normalized = 22,
	MTLVertexFormatShort3Normalized = 23,
	MTLVertexFormatShort4Normalized = 24,
    
	MTLVertexFormatHalf2 = 25,
	MTLVertexFormatHalf3 = 26,
	MTLVertexFormatHalf4 = 27,
    
	MTLVertexFormatFloat = 28,
	MTLVertexFormatFloat2 = 29,
	MTLVertexFormatFloat3 = 30,
	MTLVertexFormatFloat4 = 31,
    
	MTLVertexFormatInt = 32,
	MTLVertexFormatInt2 = 33,
	MTLVertexFormatInt3 = 34,
	MTLVertexFormatInt4 = 35,
    
	MTLVertexFormatUInt = 36,
	MTLVertexFormatUInt2 = 37,
	MTLVertexFormatUInt3 = 38,
	MTLVertexFormatUInt4 = 39,
    
	MTLVertexFormatInt1010102Normalized = 40,
	MTLVertexFormatUInt1010102Normalized = 41,
} MTLVertexFormat;

typedef enum
{
	MTLBlendFactorZero = 0,
	MTLBlendFactorOne = 1,
	MTLBlendFactorSourceColor = 2,
	MTLBlendFactorOneMinusSourceColor = 3,
	MTLBlendFactorSourceAlpha = 4,
	MTLBlendFactorOneMinusSourceAlpha = 5,
	MTLBlendFactorDestinationColor = 6,
	MTLBlendFactorOneMinusDestinationColor = 7,
	MTLBlendFactorDestinationAlpha = 8,
	MTLBlendFactorOneMinusDestinationAlpha = 9,
	MTLBlendFactorSourceAlphaSaturated = 10,
	MTLBlendFactorBlendColor = 11,
	MTLBlendFactorOneMinusBlendColor = 12,
	MTLBlendFactorBlendAlpha = 13,
	MTLBlendFactorOneMinusBlendAlpha = 14,
} MTLBlendFactor;

typedef enum
{
	MTLBlendOperationAdd = 0,
	MTLBlendOperationSubtract = 1,
	MTLBlendOperationReverseSubtract = 2,
	MTLBlendOperationMin = 3,
	MTLBlendOperationMax = 4,
} MTLBlendOperation;

typedef enum
{
	MTLColorWriteMaskNone  = 0,
	MTLColorWriteMaskRed   = 0x1 << 3,
	MTLColorWriteMaskGreen = 0x1 << 2,
	MTLColorWriteMaskBlue  = 0x1 << 1,
	MTLColorWriteMaskAlpha = 0x1 << 0,
	MTLColorWriteMaskAll   = 0xf
} MTLColorWriteMask;

typedef enum
{
	MTLCullModeNone = 0,
	MTLCullModeFront = 1,
	MTLCullModeBack = 2,
} MTLCullMode;

typedef enum
{
	MTLWindingClockwise = 0,
	MTLWindingCounterClockwise = 1,
} MTLWinding;

typedef enum
{
	MTLCompareFunctionNever = 0,
	MTLCompareFunctionLess = 1,
	MTLCompareFunctionEqual = 2,
	MTLCompareFunctionLessEqual = 3,
	MTLCompareFunctionGreater = 4,
	MTLCompareFunctionNotEqual = 5,
	MTLCompareFunctionGreaterEqual = 6,
	MTLCompareFunctionAlways = 7,
} MTLCompareFunction;