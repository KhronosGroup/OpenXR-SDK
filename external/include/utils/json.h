/*
================================================================================================

Description	:	DOM-style JSON reader/writer.
Author		:	J.M.P. van Waveren
Date		:	08/07/2016
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


JSON
====

The JavaScript Object Notation (JSON) Data Interchange Format.

http://www.json.org/
http://rfc7159.net/rfc7159
http://www.ietf.org/rfc/rfc7159.txt
http://www.ecma-international.org/publications/files/ECMA-ST/ECMA-404.pdf


DESCRIPTION
===========

This is a Document Object Model (DOM) style JSON implementation where
JSON data is represented in memory as an object tree. For maximum
portability the code is written in straight C99 without using any
third party libraries.

There are various syntactically pretty C++ JSON parsers and writers,
but the syntactical sugar tends to come at a performance cost.
For instance, this JSON implementation is on the order of 3 to 6 times
faster than a std::map based JSON implementation.

It is generally questionable whether a DOM-style JSON implementation
should use a map or hash table to access the members of an object.
An array may have many elements, but these elements are never looked
up by name. The members of an object are looked up by name, but objects
rarely have more than a dozen to a couple of dozen members. Building a
map or hash table for relatively few members tends to be more costly
than just iterating over the members and comparing the names.

As a trivial optimization this implementation continues iterating over
the members of an object where it left off during the last lookup.
If the DOM is traversed in the order it is stored, then this results
in only a single string comparison per lookup. This implementation keeps
objects in the same order in the DOM as they appear in the JSON text.

This implementation stores the members of an object, or the elements
of an array, in an exponentially growing mapped array per object or
array. The mapping may need to be re-allocated but this is very rare
and the mapping only holds pointers. The actual members are never
re-allocated. Instead, exponentially growing chunks are added to the
array. The mapping does result in one, and only one additional pointer
dereference, but that is still way faster than iterating a linked list
of objects that are all separately allocated. By not re-allocating the
actual array of members there is no danger of pointers to previous members
going stale when additional members are added. Using an exponentially
growing mapped array is not only cache friendly but also allows
direct indexing of an array.

This implementation includes several trivial optimizations that allow
it to compete with 'rapidjson' when it comes to parsing performance.
However, in most cases simplicity and correctness are favored over
aggressive or low level optimizations that may break compatibility
or conformance.

The JSON specification allows an implementation to set limits on the
range and precision of numbers. This implementation can accurately
represent the full 8-bit, 16-bit, 32-bit and 64-bit, signed and
unsigned integer ranges. This implementation can also accurately
represent all 32-bit and 64-bit floating-point values. The minimum
and maximum floating-point numbers are parsed with full precision
and all other floating-point numbers are represented within 4 ulps.
For instance, this implementation parses the following numbers with
full precision.

	INT8_MIN         -128
	INT16_MIN        -32768
	INT32_MIN        -2147483648
	INT64_MIN        -9223372036854775808
	INT8_MAX         127
	INT16_MAX        32767
	INT32_MAX        2147483647
	INT64_MAX        9223372036854775807
	UINT8_MAX        255
	UINT16_MAX       65535
	UINT32_MAX       4294967295
	UINT64_MAX       18446744073709551615
	FLT_MIN          1.175494351e-38
	FLT_MAX          3.402823466e+38
	DBL_MIN          2.2250738585072014e-308
	DBL_MAX          1.7976931348623158e+308

Integer values less than INT64_MIN or greater than UINT64_MAX are
stored as floating-point. JSON does not support floating-point values
like infinity and NaN. Therefore floating-point values are clamped
to the range [-DBL_MAX, DBL_MAX].

The JSON specification allows an implementation to set limits on the
length and character contents of strings. This implementation supports
both UTF8 and UTF16 with surrogate pairs. UTF32 is not supported.
This implementation does not set limits on the length of strings.

The JSON specification allows an implementation to set limits on
the maximum depth of nesting. This implementation uses a recursive
parser with a maximum recursion depth of 128.

The JSON specification allows an implementation to set limits on the
size of texts that it accepts. This implementation does not have any
such limitations and can parse any JSON text that fits in memory.

This implementation is designed to be robust, allowing completely
invalid JSON text to be parsed without fatal run-time exceptions.
All interface functions continue to work, even if a NULL ksJson
pointer is passed in.


INTERFACE
=========

struct ksJson;

ksJson *		ksJson_Create();
void			ksJson_Destroy( ksJson * rootNode );

bool			ksJson_ReadFromBuffer( ksJson * rootNode, const char * buffer, const char ** errorStringOut );
bool			ksJson_ReadFromFile( ksJson * rootNode, const char * fileName, const char ** errorStringOut );
bool			ksJson_WriteToBuffer( const ksJson * rootNode, char ** bufferOut, int * lengthOut );	// Buffer is allocated with malloc.
bool			ksJson_WriteToFile( const ksJson * rootNode, const char * fileName );

//
// query
//

int				ksJson_GetMemberCount( const ksJson * node );							// Get the number of object members or array elements.
ksJson *		ksJson_GetMemberByIndex( const ksJson * node, const int index );		// Get an object member or array element by index.
ksJson *		ksJson_GetMemberByName( const ksJson * node, const char * name );		// Case-sensitive lookup of an object member by name.
const char *	ksJson_GetMemberName( const ksJson * node );							// Returns the name of this member.

bool			ksJson_IsNull( const ksJson * node );									// Returns true if the node != NULL and the value is 'null'.
bool			ksJson_IsBoolean( const ksJson * node );								// Returns true if the node != NULL and the value is 'true' or 'false'.
bool			ksJson_IsNumber( const ksJson * node );									// Returns true if the node != NULL and the value is a number.
bool			ksJson_IsInteger( const ksJson * node );								// Returns true if the node != NULL and the value is an integer number.
bool			ksJson_IsUnsigned( const ksJson * node );								// Returns true if the node != NULL and the value is an unsigned integer number.
bool			ksJson_IsFloatingPoint( const ksJson * node );							// Returns true if the node != NULL and the value is a floating-point number.
bool			ksJson_IsString( const ksJson * node );									// Returns true if the node != NULL and the value is a string.
bool			ksJson_IsObject( const ksJson * node );									// Returns true if the node != NULL and the node is an object.
bool			ksJson_IsArray( const ksJson * node );									// Returns true if the node != NULL and the node is an array.

bool			ksJson_GetBoolean( const ksJson * node, const bool defaultValue );		// Returns 'defaultValue' if IsBoolean( node ) == false.
int8_t			ksJson_GetInt8( const ksJson * node, const int8_t defaultValue );		// Returns 'defaultValue' if IsNumber( node ) == false.
uint8_t			ksJson_GetUint8( const ksJson * node, const uint8_t defaultValue );		// Returns 'defaultValue' if IsNumber( node ) == false.
int16_t			ksJson_GetInt16( const ksJson * node, const int16_t defaultValue );		// Returns 'defaultValue' if IsNumber( node ) == false.
uint16_t		ksJson_GetUint16( const ksJson * node, const uint16_t defaultValue );	// Returns 'defaultValue' if IsNumber( node ) == false.
int32_t			ksJson_GetInt32( const ksJson * node, const int32_t defaultValue );		// Returns 'defaultValue' if IsNumber( node ) == false.
uint32_t		ksJson_GetUint32( const ksJson * node, const uint32_t defaultValue );	// Returns 'defaultValue' if IsNumber( node ) == false.
int64_t			ksJson_GetInt64( const ksJson * node, const int64_t defaultValue );		// Returns 'defaultValue' if IsNumber( node ) == false.
uint64_t		ksJson_GetUint64( const ksJson * node, const uint64_t defaultValue );	// Returns 'defaultValue' if IsNumber( node ) == false.
float			ksJson_GetFloat( const ksJson * node, const float defaultValue );		// Returns 'defaultValue' if IsNumber( node ) == false.
double			ksJson_GetDouble( const ksJson * node, const double defaultValue );		// Returns 'defaultValue' if IsNumber( node ) == false.
const char *	ksJson_GetString( const ksJson * node, const char * defaultValue );		// Returns 'defaultValue' if IsString( node ) == false.

//
// create & modify
//

ksJson *		ksJson_SetObject( ksJson * node );										// Turns the node into an empty object.
ksJson *		ksJson_SetArray( ksJson * node );										// Turns the node into an empty array.

ksJson *		ksJson_AddObjectMember( ksJson * node, const char * name );				// Adds and returns a new object member with the given name. The node must be an object.
ksJson *		ksJson_AddArrayElement( ksJson * node );								// Adds and returns a new array element. The node must be an array.

ksJson *		ksJson_SetNull( ksJson * node );										// Turns the node into null.
ksJson *		ksJson_SetBoolean( ksJson * node, const bool value );					// Turns the node into a boolean with the given value.
ksJson *		ksJson_SetInt8( ksJson * node, const int8_t value );					// Turns the node into a 8-bit signed integer with the given value.
ksJson *		ksJson_SetUint8( ksJson * node, const uint8_t value );					// Turns the node into a 8-bit unsigned integer with the given value.
ksJson *		ksJson_SetInt16( ksJson * node, const int16_t value );					// Turns the node into a 16-bit signed integer with the given value.
ksJson *		ksJson_SetUint16( ksJson * node, const uint16_t value );				// Turns the node into a 16-bit unsigned integer with the given value.
ksJson *		ksJson_SetInt32( ksJson * node, const int32_t value );					// Turns the node into a 32-bit signed integer with the given value.
ksJson *		ksJson_SetUint32( ksJson * node, const uint32_t value );				// Turns the node into a 32-bit unsigned integer with the given value.
ksJson *		ksJson_SetInt64( ksJson * node, const int64_t value );					// Turns the node into a 64-bit signed integer with the given value.
ksJson *		ksJson_SetUint64( ksJson * node, const uint64_t value );				// Turns the node into a 64-bit unsigned integer with the given value.
ksJson *		ksJson_SetFloat( ksJson * node, const float value );					// Turns the node into a 32-bit floating-point number with the given value.
ksJson *		ksJson_SetDouble( ksJson * node, const double value );					// Turns the node into a 64-bit floating-point number with the given value.
ksJson *		ksJson_SetString( ksJson * node, const char * value );					// Turns the node into a string with the given value.


USAGE
=====

The ksJson_Create() function creates an empty DOM. The default value of this
empty DOM is null:

    ksJson_IsNull( ksJson_Create() ) == true

The ksJson_Read* functions can be used to initialize the DOM from JSON text.
The ksJson_Is* and ksJson_Get* functions can be used to query the DOM from code.
The ksJson_Set* and ksJson_Add* functions can be used to create/modify the DOM.
The ksJson_Write* functions can be used to write the DOM to JSON text.
The ksJson_Destroy() function is used to destroy the complete DOM.

The functions ksJson_GetMemberByIndex() and ksJson_GetMemberByName() are used
to access the elements of an array and/or members of an object. These functions
may return NULL if the node is not an object or array, the index is out of range,
or no member by the given name exists. All the ksJson_Is* functions will return
false when a NULL pointer is passed in. All the ksJson_Get* functions will return
the default value when a NULL pointer is passed in.

The interface to create/modify the JSON DOM does not allow nodes to be
inserted into the DOM because this can easily lead to cyclic references
and testing for cyclic references would be time consuming. Instead, object
members and array elements are allocated from a JSON object or array after
which the member or element can be assigned a value. The default value of
a new object member or array element is null.

    ksJson_IsNull( ksJson_AddObjectMember( object, "count" ) ) == true
    ksJson_IsNull( ksJson_AddArrayElement( array ) ) == true

A value in the DOM can be changed at any time by calling one of the ksJson_Set*
functions. For instance:

    ksJson_SetInt32( ksJson_AddObjectMember( object, "count" ), 10 );
    ksJson_SetFloat( ksJson_AddArrayElement( array ), 2.0f );

The object members and array elements are kept in the order they are added.
A JSON object or array can be cleared by calling ksJson_SetObject() or
ksJson_SetArray() respectively.


EXAMPLES
========

char * buffer = NULL;
int length = 0;

{
	ksJson * rootNode = ksJson_SetObject( ksJson_Create() );
	ksJson * vertices = ksJson_SetArray( ksJson_AddObjectMember( rootNode, "vertices" ) );
	for ( int i = 0; i < 3; i++ )
	{
		ksJson * vertex = ksJson_SetObject( ksJson_AddArrayElement( vertices ) );

		ksJson * position = ksJson_SetObject( ksJson_AddObjectMember( vertex, "position" ) );
		ksJson_SetFloat( ksJson_AddObjectMember( position, "x" ), (float)( ( i & 1 ) >> 0 ) );
		ksJson_SetFloat( ksJson_AddObjectMember( position, "y" ), (float)( ( i & 2 ) >> 1 ) );
		ksJson_SetFloat( ksJson_AddObjectMember( position, "z" ), (float)( ( i & 4 ) >> 2 ) );

		ksJson * normal = ksJson_SetObject( ksJson_AddObjectMember( vertex, "normal" ) );
		ksJson_SetFloat( ksJson_AddObjectMember( normal, "x" ), 0.5f );
		ksJson_SetFloat( ksJson_AddObjectMember( normal, "y" ), 0.6f );
		ksJson_SetFloat( ksJson_AddObjectMember( normal, "z" ), 0.7f );
	}
	ksJson * indices = ksJson_SetArray( ksJson_AddObjectMember( rootNode, "indices" ) );
	for ( unsigned int i = 0; i < 3; i++ )
	{
		ksJson_SetUint32( ksJson_AddArrayElement( indices ), i );
	}
	ksJson_WriteToBuffer( rootNode, &buffer, &length );
	ksJson_Destroy( rootNode );
}

{
	ksJson * rootNode = ksJson_Create();
	if ( ksJson_ReadFromBuffer( rootNode, buffer, NULL ) )
	{
		const ksJson * vertices = ksJson_GetMemberByName( rootNode, "vertices" );
		for ( int i = 0; i < ksJson_GetMemberCount( vertices ); i++ )
		{
			const ksJson * vertex = ksJson_GetMemberByIndex( vertices, i );

			const ksJson * position = ksJson_GetMemberByName( vertex, "position" );
			const float position_x = ksJson_GetFloat( ksJson_GetMemberByName( position, "x" ), 0.0f );
			const float position_y = ksJson_GetFloat( ksJson_GetMemberByName( position, "y" ), 0.0f );
			const float position_z = ksJson_GetFloat( ksJson_GetMemberByName( position, "z" ), 0.0f );

			const ksJson * normal = ksJson_GetMemberByName( vertex, "normal" );
			const float normal_x = ksJson_GetFloat( ksJson_GetMemberByName( normal, "x" ), 0.0f );
			const float normal_y = ksJson_GetFloat( ksJson_GetMemberByName( normal, "y" ), 0.0f );
			const float normal_z = ksJson_GetFloat( ksJson_GetMemberByName( normal, "z" ), 0.0f );
		}
		const ksJson * indices = ksJson_GetMemberByName( rootNode, "indices" );
		for ( int i = 0; i < ksJson_GetMemberCount( indices ); i++ )
		{
			const unsigned int index = ksJson_GetUint32( ksJson_GetMemberByIndex( indices, i ), 0 );
		}
	}
	ksJson_Destroy( rootNode );
}

free( buffer );


COMPARISON
==========

The following DOM-style implementations were tested:
                                                            
Name        Version               Source                                          Language
------------------------------------------------------------------------------------------
ksJson      1.0                   https://github.com/KhronosGroup/Vulkan-Samples  C99
rapidjson   1.02                  https://github.com/miloyip/rapidjson            C++
sajson      Mar 24, 2016          https://github.com/chadaustin/sajson            C++
gason       Sep 16, 2015          https://github.com/vivkin/gason                 C++
ujson4c     1.0 (ultrajson 1.34)  https://github.com/esnme/ujson4c                C99
cJSON       Mar 19, 2016          https://github.com/DaveGamble/cJSON             C99
OVR::JSON   Apr 9, 2013           https://developer3.oculus.com/downloads         C++
jsoncons    Aug 19, 2016          http://danielaparker.github.io/jsoncons         C++11
json11      Jun 20, 2016          https://github.com/dropbox/json11               C++11
nlohmann    2.0.0                 https://github.com/nlohmann/json                C++11

These implementations have the following general properties:

           Const   Throws  Mutable  Maint. Member       Case       Supports  Supports  Supports  Supports  Supports  <= 4u   Clamp   Robust
Name       source  except. DOM      order  storage      sensitive  UTF16     int32_t   uint32_t  int64_t   uint64_t  double  double  
-------------------------------------------------------------------------------------------------------------------------------------------
ksJson     yes     no      yes      yes    mapped-array yes        yes       yes       yes       yes       yes       yes     yes     yes
rapidjson  yes     yes     yes      yes    array        yes        yes       yes       yes       yes       yes       yes     no      no
sajson     yes     no      no       no     array        yes        yes       no        no        no        no        no      no      no
gason      no      no      no       yes    linked-list  N/A        no        yes       yes       no        no        no      no      no
ujson4c    yes     no      yes      yes    linked-list  N/A        no        yes       yes       yes       no        no      no      no
cJSON      yes     no      yes      yes    linked-list  no         broken    yes       no        no        no        no      no      no
OVR::JSON  yes     no      yes      yes    linked-list  yes        broken    yes       yes       no        no        no      no      no
jsoncons   yes     yes     yes      yes    std::vector  yes        yes       yes       yes       yes       yes       yes     no      yes
json11     yes     no      yes      no     std::map     yes        yes       yes       no        no        no        yes     no      yes
nlohmann   yes     yes     yes      no     std::map     yes        yes       yes       yes       yes       yes       yes     no      no

  Const source       = Is the JSON source text left unmodified and can it be freed after creating the DOM?
  Throws except.     = Does this implementation throw exceptions during parsing?
  Mutable DOM        = Can the DOM be modified after it is parsed or created?
  Maint. order       = Are object members stored in the same order in the JSON text and DOM?
  Member storage     = How are object members stored in the DOM?
  Case sensitive     = Are object member lookups case sensitive?
  Supports UTF16     = Does this implementation support UTF16 with surrogate pairs?
  Supports int32_t   = Can this implementation represent the full 32-bit signed integer range?
  Supports uint32_t  = Can this implementation represent the full 32-bit unsigned integer range?
  Supports int64_t   = Can this implementation represent the full 64-bit signed integer range?
  Supports uint64_t  = Can this implementation represent the full 64-bit unsigned integer range?
  <= 4u double       = Are all 64-bit floating-point values in the range [DBL_MIN, DBL_MAX] accurately represented within 4 ulps?
  Clamp double       = Are numbers clamped to the range [-DBL_MAX, DBL_MAX] to avoid infinity?
  Robust             = Is looking up a missing value, or value of the wrong type,
                       safe without causing a fatal run-time exception?


BENCHMARK
=========

This benchmark shows the time in milliseconds it takes to parse 'twitter_public_timeline.json' 1000 times.

  https://github.com/TouchCode/TouchJSON/blob/master/Support/Test%20Data/twitter_public_timeline.json

----------------------------------------------------------------------------------------------------------
Text to DOM     2.6 GHz        2.6 GHz        2.1 GHz        2.1 GHz        2.1 GHz        2.1 GHz 
                Intel Core i7  Intel Core i7  Qualcomm Kryo  Qualcomm Kryo  Qualcomm Kryo  Qualcomm Kryo
                win32          win64          armeabi-v7a    arm64-v8a      armeabi-v7a    arm64-v8a
                VS2015-up3     VS2015-up3     GCC 4.9        GCC 4.9        Clang 3.8      Clang 3.8
                c++11          c++11          gnustl_static  gnustl_static  c++_static     c++_static
----------------------------------------------------------------------------------------------------------
  ksJson        123.704 ms     113.623 ms      260.471 ms     229.713 ms     284.767 ms     244.413 ms
  rapidjson     113.475 ms     117.906 ms      294.813 ms     217.756 ms     318.686 ms     267.120 ms
  sajson         99.068 ms      69.175 ms      287.485 ms     167.319 ms     173.386 ms     182.552 ms
  gason          64.896 ms      57.836 ms      116.311 ms     115.667 ms     195.282 ms     123.649 ms
  ujson4c       118.496 ms     106.542 ms      417.591 ms     330.118 ms     364.956 ms     287.685 ms
  cJSON         189.076 ms     180.745 ms      363.644 ms     315.248 ms     423.802 ms     319.888 ms
  OVR::JSON     334.811 ms     300.541 ms      791.750 ms     883.271 ms     812.664 ms     879.435 ms
  jsoncons      276.632 ms     232.985 ms      616.342 ms     582.109 ms     618.514 ms     451.056 ms
  json11        517.014 ms     467.236 ms     2592.904 ms    2327.689 ms    2004.852 ms    1476.452 ms
  nlohmann      469.432 ms     422.636 ms     1305.589 ms    1296.296 ms    1410.566 ms     988.472 ms

----------------------------------------------------------------------------------------------------------
Traverse DOM    2.6 GHz        2.6 GHz        2.1 GHz        2.1 GHz        2.1 GHz        2.1 GHz 
                Intel Core i7  Intel Core i7  Qualcomm Kryo  Qualcomm Kryo  Qualcomm Kryo  Qualcomm Kryo
                win32          win64          armeabi-v7a    arm64-v8a      armeabi-v7a    arm64-v8a
                VS2015-up3     VS2015-up3     GCC 4.9        GCC 4.9        Clang 3.8      Clang 3.8
                c++11          c++11          gnustl_static  gnustl_static  c++_static     c++_static
----------------------------------------------------------------------------------------------------------
  ksJson         15.963 ms      15.765 ms      106.081 ms      39.828 ms      84.220 ms      42.936 ms
  rapidjson      39.516 ms      31.692 ms       82.376 ms      48.170 ms      75.268 ms      49.578 ms
  sajson         39.083 ms      43.925 ms      153.721 ms     113.389 ms      95.203 ms      56.443 ms
  gason          21.963 ms      23.685 ms      212.653 ms      99.502 ms     218.437 ms     103.013 ms
  ujson4c        25.110 ms      25.750 ms      141.612 ms      82.157 ms      96.826 ms      87.144 ms
  cJSON         302.371 ms     380.556 ms      522.007 ms     468.991 ms     600.157 ms     571.723 ms
  OVR::JSON      55.423 ms      55.772 ms      334.563 ms     205.061 ms     332.576 ms     202.970 ms
  jsoncons      154.659 ms     138.156 ms      542.477 ms     421.545 ms     963.076 ms     796.742 ms
  json11        144.897 ms     126.033 ms      576.560 ms     449.121 ms     462.324 ms     292.826 ms
  nlohmann      101.242 ms      99.691 ms      401.062 ms     501.726 ms     267.339 ms     169.525 ms

================================================================================================================================
*/

#if !defined( KSJSON_H )
#define KSJSON_H

#ifdef _MSC_VER
	#pragma warning( disable : 4201 )	// nonstandard extension used: nameless struct/union
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <float.h>
#include <assert.h>
#include <string.h>
#if !defined( __APPLE__ )
#include <malloc.h>
#endif
#if defined( _MSC_VER )
#include <intrin.h>
#endif

#define JSON_MIN( x, y )			( ( x <= y ) ? x : y )
#define JSON_MAX( x, y )			( ( x >= y ) ? x : y )
#define JSON_CLAMP( x, min, max )	( ( x >= min ) ? ( ( x <= max ) ? x : max ) : min )
#define JSON_MAX_RECURSION			128
#define JSON_MAP_GRANULARITY		4	// 128, 2048 etc. members
#define JSON_BASE_ALLOC_PWR			4	// [16, 32, 64, 128], [256, 512, 1024, 2048] etc. members

// JSON value type
typedef enum
{
	JSON_NONE		= 0,
	JSON_NULL		= 1,			// null
	JSON_BOOLEAN	= 2,			// true | false
	JSON_INT		= 3,			// signed integer
	JSON_UINT		= 4,			// unsigned integer
	JSON_FLOAT		= 5,			// integer[.fraction][exponent]
	JSON_STRING		= 6,			// "string"
	JSON_OBJECT		= 7,			// { "name" : <value>, "name" : <value>, ... }
	JSON_ARRAY		= 8,			// [ <value>, <value>, ... ]
	JSON_MAX_ENUM	= 0x7FFFFFFF	// Make sure this enum is 32 bits.
} ksJsonType;

// JSON node
// 32-bit sizeof( ksJson ) = 64-bit sizeof( ksJson ) = 32
typedef struct ksJson
{
	union
	{
		char *			name;				// only != NULL for named members
		uint64_t		pad;				// make the structure size the same between 32-bit and 64-bit
	};
	union
	{
		int64_t				valueInt64;		// 64-bit signed integer value
		uint64_t			valueUint64;	// 64-bit unsigned integer value
		double				valueDouble;	// 64-bit floating-point value
		char *				valueString;	// string value
		struct ksJson **	memberMap;		// object/array members
	};
	ksJsonType		type;					// type of value
	int				membersAllocated;		// number of allocated members
	int				memberCount;			// number of actual members
	int				memberIndex;			// mutable member index for faster lookups
} ksJson;

static ksJson * ksJson_Create()
{
	ksJson * json = (ksJson *) calloc( 1, sizeof( ksJson ) );
	json->valueString = (char *)"null";
	json->type = JSON_NULL;
	return json;
}

static int MemberIndexToMapIndex( int index )
{
	index >>= JSON_BASE_ALLOC_PWR;
#if defined( _MSC_VER )
	unsigned long offset;
	if ( _BitScanReverse( &offset, (unsigned int) index ) )
	{
		return offset + 1;
	}
	return 0;
#elif defined( __GNUC__ ) || defined( __clang__ )
	return 32 - __builtin_clz( (unsigned int) index );
#else
	int r = 0;
	int t;
	index <<= 1;
	t = ( (~( ( index >> 16 ) + ~0U ) ) >> 27 ) & 0x10; r |= t; index >>= t;
	t = ( (~( ( index >>  8 ) + ~0U ) ) >> 28 ) & 0x08; r |= t; index >>= t;
	t = ( (~( ( index >>  4 ) + ~0U ) ) >> 29 ) & 0x04; r |= t; index >>= t;
	t = ( (~( ( index >>  2 ) + ~0U ) ) >> 30 ) & 0x02; r |= t; index >>= t;
	return ( r | ( index >> 1 ) );
#endif
}

static int MapMemberOffset( int mapIndex )
{
	const int offset = ( 1 << ( mapIndex - 1 ) ) << JSON_BASE_ALLOC_PWR;
	assert( mapIndex >= 1 || offset == 0 );	// Protect againt negative shift not producing zero.
	return offset;
}

static int MapMemberCount( int mapIndex, int memberCount )
{
	return JSON_MIN( ( 1 << ( JSON_BASE_ALLOC_PWR + mapIndex ) ), memberCount ) - MapMemberOffset( mapIndex );
}

static ksJson * ksJson_AllocMember( ksJson * node )
{
	const int mapIndex = MemberIndexToMapIndex( node->memberCount );
	if ( node->memberCount >= node->membersAllocated )
	{
		if ( ( mapIndex & ( JSON_MAP_GRANULARITY - 1 ) ) == 0 )
		{
			ksJson ** newMemberMap = (ksJson **) malloc( ( mapIndex + JSON_MAP_GRANULARITY ) * sizeof( ksJson * ) );
			if ( mapIndex > 0 )
			{
				memcpy( newMemberMap, node->memberMap, mapIndex * sizeof( ksJson * ) );
				free( node->memberMap );
			}
			node->memberMap = newMemberMap;
		}
		const int mapSize = JSON_MAX( MapMemberOffset( mapIndex ), ( 1 << JSON_BASE_ALLOC_PWR ) );
		node->memberMap[mapIndex] = (ksJson *) malloc( mapSize * sizeof( ksJson ) );
		node->membersAllocated += mapSize;
	}
	const int memberOffset = MapMemberOffset( mapIndex );
	ksJson * member = &node->memberMap[mapIndex][node->memberCount++ - memberOffset];
	member->name = NULL;
	member->valueInt64 = 0;
	member->valueString = (char *)"null";
	member->type = JSON_NULL;
	member->membersAllocated = 0;
	member->memberCount = 0;
	member->memberIndex = 0;
	return member;
}

static void ksJson_FreeNode( ksJson * node, const bool freeName )
{
	assert( node->type >= JSON_NULL && node->type <= JSON_ARRAY );		// stale ksJson pointer?
	if ( freeName )
	{
		free( node->name );
		node->name = NULL;
	}
	if ( node->type == JSON_OBJECT || node->type == JSON_ARRAY )
	{
		if ( node->memberCount > 0 )
		{
			const int endMapIndex = MemberIndexToMapIndex( node->memberCount - 1 );
			for ( int mapIndex = 0; mapIndex <= endMapIndex; mapIndex++ )
			{
				ksJson * members = node->memberMap[mapIndex];
				const int mapMemberCount = MapMemberCount( mapIndex, node->memberCount ); 
				for ( int i = 0; i < mapMemberCount; i++ )
				{
					ksJson_FreeNode( &members[i], true );
				}
				free( members );
			}
			free( node->memberMap );
		}
	}
	else if ( node->type == JSON_STRING )
	{
		free( node->valueString );
	}
	node->valueInt64 = 0;
	node->valueString = (char *)"null";
	node->type = JSON_NULL;
	node->membersAllocated = 0;
	node->memberCount = 0;
	node->memberIndex = 0;
}

static void ksJson_Destroy( ksJson * rootNode )
{
	if ( rootNode != NULL /* && is an actual root */ )
	{
		ksJson_FreeNode( rootNode, true );
		free( rootNode );
	}
}

// Parses white space.
// Returns a pointer to the first character after the white space.
static const char * ksJson_ParseWhiteSpace( const char * buffer )
{
	while ( buffer[0] != '\0' && (unsigned char)buffer[0] <= ' ' )
	{
		buffer++;
	}
	return buffer;
}

// Parses a hexadecimal string up to four digits and stores the integer result in 'value'.
// Returns a pointer to the first character after the string.
static const char * ksJson_ParseHex4( unsigned int * value, const char * buffer )
{
	int v = 0;
	for( unsigned int digit = 0; digit < 4; digit++, buffer++ )
	{
		unsigned int d = buffer[0];

		if ( ( d >= '0' ) && ( d <= '9' ) ) d = d - '0';
		else if ( ( d >= 'a') && ( d <= 'f' ) ) d = 10 + d - 'a';
		else if ( ( d >= 'A' ) && ( d <= 'F' ) ) d = 10 + d - 'A';
		else break;

		v = v * 16 + d;
	}
	*value = v;
	return buffer;
}

static const char json_escape[256] =
{
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,'\"',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, '/',
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,'\\',   0,   0,   0,
	0,   0,'\b',   0,   0,   0,'\f',   0,   0,   0,   0,   0,   0,   0,'\n',   0,
	0,   0,'\r',   0,'\t',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

static const unsigned char json_firstByteMark[7] =
{
	0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC
};

// Parses a string and stores the result in 'value'.
// Returns a pointer to the first character after the string.
static const char * ksJson_ParseString( char ** value, const char * buffer, const char ** errorStringOut )
{
	assert( buffer[0] == '\"' );
	buffer++;

	int length = 0;
	for ( const char * str = buffer; *str != '\"' && *str != '\0'; length++ )
	{
		// Collapse escaped characters and skip escaped quotes.
		if ( str[0] == '\\' && str[1] != '\0' ) str++;
		str++;
	}

	char * out = (char *) malloc( length + 1 );
	char * outPtr = out;

	while ( buffer[0] != '\"' && buffer[0] != '\0' )
	{
		if ( buffer[0] != '\\' )
		{
			*outPtr++ = *buffer++;
		}
		else if ( json_escape[(unsigned char)buffer[1]] )
		{
			*outPtr++ = json_escape[(unsigned char)buffer[1]];
			buffer += 2;
		}
		else if ( buffer[1] == 'u' )
		{
			// Transcode UTF16 to UTF8.
			unsigned int uc = 0;
			buffer = ksJson_ParseHex4( &uc, buffer + 2 );
			// Check for UTF16 surrogate pairs.
			if ( uc >= 0xD800 && uc <= 0xDBFF )
			{
				// Second-half of surrogate.
				if ( buffer[0] == '\\' && buffer[1] == 'u' )
				{
					unsigned int uc2 = 0;
					buffer = ksJson_ParseHex4( &uc2, buffer + 2 );
					if ( uc2 < 0xDC00 || uc2 > 0xDFFF )
					{
						free( out );
						*errorStringOut = "invalid unicode";
						return buffer;
					}
					uc = ( ( ( uc - 0xD800 ) << 10 ) | ( uc2 - 0xDC00 ) ) + 0x10000;
				}
			}

			const int outLength =	( ( uc < 0x80 ) ? 1 :
									( ( uc < 0x800 ) ? 2 :
									( ( uc < 0x10000 ) ? 3 : 4 ) ) );
			outPtr += outLength;
			switch ( outLength )
			{
				case 4: *--outPtr = (char)( ( uc | 0x80 ) & 0xBF ); uc >>= 6;	// fall through
				case 3: *--outPtr = (char)( ( uc | 0x80 ) & 0xBF ); uc >>= 6;	// fall through
				case 2: *--outPtr = (char)( ( uc | 0x80 ) & 0xBF ); uc >>= 6;	// fall through
				case 1: *--outPtr = (char)( uc | json_firstByteMark[outLength] );
			}
			outPtr += outLength;
		}
		else
		{
			*outPtr++ = *buffer++;
		}
	}

	*outPtr = 0;
	if ( *buffer != '\"' )
	{
		free( out );
		*errorStringOut = "missing trailing quote";
		return buffer;
	}
	buffer++;

	*value = out;

	return buffer;
}

// from 10^-308 to 10^+308
static const double json_pow10[] =
{
	1e-308,1e-307,1e-306,1e-305,
	1e-304,1e-303,1e-302,1e-301,1e-300,1e-299,1e-298,1e-297,1e-296,1e-295,1e-294,1e-293,1e-292,1e-291,1e-290,1e-289,
	1e-288,1e-287,1e-286,1e-285,1e-284,1e-283,1e-282,1e-281,1e-280,1e-279,1e-278,1e-277,1e-276,1e-275,1e-274,1e-273,
	1e-272,1e-271,1e-270,1e-269,1e-268,1e-267,1e-266,1e-265,1e-264,1e-263,1e-262,1e-261,1e-260,1e-259,1e-258,1e-257,
	1e-256,1e-255,1e-254,1e-253,1e-252,1e-251,1e-250,1e-249,1e-248,1e-247,1e-246,1e-245,1e-244,1e-243,1e-242,1e-241,
	1e-240,1e-239,1e-238,1e-237,1e-236,1e-235,1e-234,1e-233,1e-232,1e-231,1e-230,1e-229,1e-228,1e-227,1e-226,1e-225,
	1e-224,1e-223,1e-222,1e-221,1e-220,1e-219,1e-218,1e-217,1e-216,1e-215,1e-214,1e-213,1e-212,1e-211,1e-210,1e-209,
	1e-208,1e-207,1e-206,1e-205,1e-204,1e-203,1e-202,1e-201,1e-200,1e-199,1e-198,1e-197,1e-196,1e-195,1e-194,1e-193,
	1e-192,1e-191,1e-190,1e-189,1e-188,1e-187,1e-186,1e-185,1e-184,1e-183,1e-182,1e-181,1e-180,1e-179,1e-178,1e-177,
	1e-176,1e-175,1e-174,1e-173,1e-172,1e-171,1e-170,1e-169,1e-168,1e-167,1e-166,1e-165,1e-164,1e-163,1e-162,1e-161,
	1e-160,1e-159,1e-158,1e-157,1e-156,1e-155,1e-154,1e-153,1e-152,1e-151,1e-150,1e-149,1e-148,1e-147,1e-146,1e-145,
	1e-144,1e-143,1e-142,1e-141,1e-140,1e-139,1e-138,1e-137,1e-136,1e-135,1e-134,1e-133,1e-132,1e-131,1e-130,1e-129,
	1e-128,1e-127,1e-126,1e-125,1e-124,1e-123,1e-122,1e-121,1e-120,1e-119,1e-118,1e-117,1e-116,1e-115,1e-114,1e-113,
	1e-112,1e-111,1e-110,1e-109,1e-108,1e-107,1e-106,1e-105,1e-104,1e-103,1e-102,1e-101,1e-100, 1e-99, 1e-98, 1e-97,
	 1e-96, 1e-95, 1e-94, 1e-93, 1e-92, 1e-91, 1e-90, 1e-89, 1e-88, 1e-87, 1e-86, 1e-85, 1e-84, 1e-83, 1e-82, 1e-81,
	 1e-80, 1e-79, 1e-78, 1e-77, 1e-76, 1e-75, 1e-74, 1e-73, 1e-72, 1e-71, 1e-70, 1e-69, 1e-68, 1e-67, 1e-66, 1e-65,
	 1e-64, 1e-63, 1e-62, 1e-61, 1e-60, 1e-59, 1e-58, 1e-57, 1e-56, 1e-55, 1e-54, 1e-53, 1e-52, 1e-51, 1e-50, 1e-49,
	 1e-48, 1e-47, 1e-46, 1e-45, 1e-44, 1e-43, 1e-42, 1e-41, 1e-40, 1e-39, 1e-38, 1e-37, 1e-36, 1e-35, 1e-34, 1e-33,
	 1e-32, 1e-31, 1e-30, 1e-29, 1e-28, 1e-27, 1e-26, 1e-25, 1e-24, 1e-23, 1e-22, 1e-21, 1e-20, 1e-19, 1e-18, 1e-17,
	 1e-16, 1e-15, 1e-14, 1e-13, 1e-12, 1e-11, 1e-10,  1e-9,  1e-8,  1e-7,  1e-6,  1e-5,  1e-4,  1e-3,  1e-2,  1e-1,
	  1e+0,  1e+1,  1e+2,  1e+3,  1e+4,  1e+5,  1e+6,  1e+7,  1e+8,  1e+9, 1e+10, 1e+11, 1e+12, 1e+13, 1e+14, 1e+15,
	 1e+16, 1e+17, 1e+18, 1e+19, 1e+20, 1e+21, 1e+22, 1e+23, 1e+24, 1e+25, 1e+26, 1e+27, 1e+28, 1e+29, 1e+30, 1e+31,
	 1e+32, 1e+33, 1e+34, 1e+35, 1e+36, 1e+37, 1e+38, 1e+39, 1e+40, 1e+41, 1e+42, 1e+43, 1e+44, 1e+45, 1e+46, 1e+47,
	 1e+48, 1e+49, 1e+50, 1e+51, 1e+52, 1e+53, 1e+54, 1e+55, 1e+56, 1e+57, 1e+58, 1e+59, 1e+60, 1e+61, 1e+62, 1e+63,
	 1e+64, 1e+65, 1e+66, 1e+67, 1e+68, 1e+69, 1e+70, 1e+71, 1e+72, 1e+73, 1e+74, 1e+75, 1e+76, 1e+77, 1e+78, 1e+79,
	 1e+80, 1e+81, 1e+82, 1e+83, 1e+84, 1e+85, 1e+86, 1e+87, 1e+88, 1e+89, 1e+90, 1e+91, 1e+92, 1e+93, 1e+94, 1e+95,
	 1e+96, 1e+97, 1e+98, 1e+99,1e+100,1e+101,1e+102,1e+103,1e+104,1e+105,1e+106,1e+107,1e+108,1e+109,1e+110,1e+111,
	1e+112,1e+113,1e+114,1e+115,1e+116,1e+117,1e+118,1e+119,1e+120,1e+121,1e+122,1e+123,1e+124,1e+125,1e+126,1e+127,
	1e+128,1e+129,1e+130,1e+131,1e+132,1e+133,1e+134,1e+135,1e+136,1e+137,1e+138,1e+139,1e+140,1e+141,1e+142,1e+143,
	1e+144,1e+145,1e+146,1e+147,1e+148,1e+149,1e+150,1e+151,1e+152,1e+153,1e+154,1e+155,1e+156,1e+157,1e+158,1e+159,
	1e+160,1e+161,1e+162,1e+163,1e+164,1e+165,1e+166,1e+167,1e+168,1e+169,1e+170,1e+171,1e+172,1e+173,1e+174,1e+175,
	1e+176,1e+177,1e+178,1e+179,1e+180,1e+181,1e+182,1e+183,1e+184,1e+185,1e+186,1e+187,1e+188,1e+189,1e+190,1e+191,
	1e+192,1e+193,1e+194,1e+195,1e+196,1e+197,1e+198,1e+199,1e+200,1e+201,1e+202,1e+203,1e+204,1e+205,1e+206,1e+207,
	1e+208,1e+209,1e+210,1e+211,1e+212,1e+213,1e+214,1e+215,1e+216,1e+217,1e+218,1e+219,1e+220,1e+221,1e+222,1e+223,
	1e+224,1e+225,1e+226,1e+227,1e+228,1e+229,1e+230,1e+231,1e+232,1e+233,1e+234,1e+235,1e+236,1e+237,1e+238,1e+239,
	1e+240,1e+241,1e+242,1e+243,1e+244,1e+245,1e+246,1e+247,1e+248,1e+249,1e+250,1e+251,1e+252,1e+253,1e+254,1e+255,
	1e+256,1e+257,1e+258,1e+259,1e+260,1e+261,1e+262,1e+263,1e+264,1e+265,1e+266,1e+267,1e+268,1e+269,1e+270,1e+271,
	1e+272,1e+273,1e+274,1e+275,1e+276,1e+277,1e+278,1e+279,1e+280,1e+281,1e+282,1e+283,1e+284,1e+285,1e+286,1e+287,
	1e+288,1e+289,1e+290,1e+291,1e+292,1e+293,1e+294,1e+295,1e+296,1e+297,1e+298,1e+299,1e+300,1e+301,1e+302,1e+303,
	1e+304,1e+305,1e+306,1e+307,1e+308
};

// Parses a number and stores the result in 'valueInt64', 'valueUint64' or 'valueDouble'.
// Returns a pointer to the first character after the number.
static const char * ksJson_ParseNumber( ksJsonType * type, int64_t * valueInt64, uint64_t * valueUint64, double * valueDouble,
										const char * buffer, const char ** errorStringOut )
{
	(void)errorStringOut;

	int sign = 1;
	int nonZeroDigits = 0;
	int fractionalDigits = 0;
	int exponentSign = 1;
	int exponentValue = 0;
	uint64_t uint64Value = 0;
	double doubleValue = 0.0;
	bool uint64Overflowed = false;

	if ( buffer[0] == '-' )
	{
		sign = -1;
		buffer++;
	}
	else if ( buffer[0] == '+' )
	{
		sign = 1;
		buffer++;
	}

	// Skip leading zeros and parse integer digits.
	while ( buffer[0] >= '0' && buffer[0] <= '9' )
	{
		const char d = ( buffer[0] - '0' );
		uint64Overflowed |= ( uint64Value > ( UINT64_MAX / 10 ) ) | ( ( uint64Value * 10 ) > UINT64_MAX - d );
		uint64Value = ( uint64Value * 10 ) + d;
		doubleValue = ( doubleValue * 10.0 ) + d;
		nonZeroDigits += ( uint64Value != 0 );
		buffer++;
	}
	// Parse fractional digits.
	if ( buffer[0] == '.' )
	{
		buffer++;
		while ( buffer[0] >= '0' && buffer[0] <= '9' && nonZeroDigits < 17 )
		{
			doubleValue = ( doubleValue * 10.0 ) + ( buffer[0] - '0' );
			nonZeroDigits += ( doubleValue != 0.0 );
			fractionalDigits++;
			buffer++;
		}
		// Skip the remaining digits.
		while ( buffer[0] >= '0' && buffer[0] <= '9' )
		{
			buffer++;
		}
	}
	// Parse exponent.
	if ( buffer[0] == 'e' || buffer[0] == 'E' )
	{
		buffer++;
		exponentSign = 1;
		if ( buffer[0] == '+' )
		{
			exponentSign = 1;
			buffer++;
		}
		else if ( buffer[0] == '-' )
		{
			exponentSign = -1;
			buffer++;
		}
		while ( buffer[0] >= '0' && buffer[0] <= '9' )
		{
			exponentValue = ( exponentValue * 10 ) + ( buffer[0] - '0' );
			buffer++;
		}
	}

	if ( fractionalDigits != 0 || exponentValue != 0 || uint64Overflowed || ( sign < 0 && uint64Value > (uint64_t)INT64_MAX + 1 ) )
	{
		*type = JSON_FLOAT;
		const int exp = exponentSign * exponentValue - fractionalDigits;
		if ( exp > 308 )
		{
			*valueDouble = sign * DBL_MAX;
		}
		else if ( exp >= -284 )
		{
			const int pow0 = 308 + 16;
			const int pow1 = 308 * 2 + exp - pow0;
			assert( pow0 >= 0 && pow0 < sizeof( json_pow10 ) / sizeof( json_pow10[0] ) );
			assert( pow1 >= 0 && pow1 < sizeof( json_pow10 ) / sizeof( json_pow10[0] ) );
			const double v = doubleValue * json_pow10[pow0] * json_pow10[pow1];
			*valueDouble = sign * ( v <= DBL_MAX ? v : DBL_MAX );
		}
		else if ( exp >= -324 )
		{
			const int pow0 = 16;
			const int pow1 = 308 * 2 + exp - pow0;
			assert( pow0 >= 0 && pow0 < sizeof( json_pow10 ) / sizeof( json_pow10[0] ) );
			assert( pow1 >= 0 && pow1 < sizeof( json_pow10 ) / sizeof( json_pow10[0] ) );
			*valueDouble = sign * doubleValue * json_pow10[pow0] * json_pow10[pow1];
		}
		else
		{
			*valueDouble = 0.0;
		}
	}
	else if ( sign < 0 )
	{
		*type = JSON_INT;
		*valueInt64 = -(int64_t)uint64Value;
	}
	else
	{
		*type = JSON_UINT;
		*valueUint64 = uint64Value;
	}

	return buffer;
}

static const char * ksJson_ParseValue( ksJson * json, const int recursion, const char * buffer, const char ** errorStringOut )
{
	assert( errorStringOut != NULL );
	if ( recursion > JSON_MAX_RECURSION )
	{
		*errorStringOut = "maximum recursion";
		return buffer;
	}

	buffer = ksJson_ParseWhiteSpace( buffer );

	if ( buffer[0] == 'n' )
	{
		assert( strncmp( buffer, "null", 4 ) == 0 );
		json->type = JSON_NULL;
		json->valueString = (char *)"null";
		return buffer + 4;
	}
	else if ( buffer[0] == 'f' )
	{ 
		assert( strncmp( buffer, "false", 5 ) == 0 );
		json->type = JSON_BOOLEAN;
		json->valueString = (char *)"false";
		return buffer + 5;
	}
	else if ( buffer[0] == 't' )
	{
		assert( strncmp( buffer, "true", 4 ) == 0 );
		json->type = JSON_BOOLEAN;
		json->valueString = (char *)"true";
		return buffer + 4;
	}
	else if ( buffer[0] == '\"' )
	{
		json->type = JSON_STRING;
		return ksJson_ParseString( &json->valueString, buffer, errorStringOut );
	}
	else if ( buffer[0] == '{' )
	{
		json->type = JSON_OBJECT;
		json->memberMap = NULL;

		buffer++;
		while ( *errorStringOut == NULL )
		{
			buffer = ksJson_ParseWhiteSpace( buffer );
			if ( buffer[0] == '}' )
			{
				buffer++;
				break;
			}
			if ( json->memberCount > 0 )
			{
				if ( buffer[0] != ',' )
				{
					*errorStringOut = "missing comma";
					return buffer;
				}
				buffer++;
			}
			ksJson * member = ksJson_AllocMember( json );
			buffer = ksJson_ParseWhiteSpace( buffer );
			buffer = ksJson_ParseString( &member->name, buffer, errorStringOut );
			buffer = ksJson_ParseWhiteSpace( buffer );
			if ( buffer[0] != ':' )
			{
				*errorStringOut = "missing colon";
				return buffer;
			}
			buffer++;
			buffer = ksJson_ParseValue( member, recursion + 1, buffer, errorStringOut );
		}
		return buffer;
	}
	else if ( buffer[0] == '[' )
	{ 
		json->type = JSON_ARRAY;
		json->memberMap = NULL;

		buffer++;
		while ( *errorStringOut == NULL )
		{
			buffer = ksJson_ParseWhiteSpace( buffer );
			if ( buffer[0] == ']' )
			{
				buffer++;
				break;
			}
			if ( json->memberCount > 0 )
			{
				if ( buffer[0] != ',' )
				{
					*errorStringOut = "missing comma";
					return buffer;
				}
				buffer++;
			}
			ksJson * member = ksJson_AllocMember( json );
			buffer = ksJson_ParseValue( member, recursion + 1, buffer, errorStringOut );
		}
		return buffer;
	}
	else
	{
		return ksJson_ParseNumber( &json->type, &json->valueInt64, &json->valueUint64, &json->valueDouble, buffer, errorStringOut );
	}
}

static bool ksJson_ReadFromBuffer( ksJson * rootNode, const char * buffer, const char ** errorStringOut )
{
	if ( rootNode == NULL || buffer == NULL )
	{
		return false;
	}
	if ( errorStringOut != NULL )
	{
		*errorStringOut = NULL;
	}
	ksJson_FreeNode( rootNode, true );

	const char * error = NULL;
	ksJson_ParseValue( rootNode, 0, buffer, &error );
	if ( error != NULL )
	{
		if ( errorStringOut != NULL )
		{
			*errorStringOut = error;
		}
		ksJson_FreeNode( rootNode, true );
		return false;
	}
	return true;
}

static bool ksJson_ReadFromFile( ksJson * rootNode, const char * fileName, const char ** errorStringOut )
{
	if ( rootNode == NULL || fileName == NULL )
	{
		return false;
	}
	if ( errorStringOut != NULL )
	{
		*errorStringOut = NULL;
	}
	ksJson_FreeNode( rootNode, true );

	FILE * file = fopen( fileName, "rb" );
	if ( file == NULL )
	{
		if ( errorStringOut != NULL )
		{
			*errorStringOut = "failed to open file";
		}
		return false;
	}

	fseek( file, 0L, SEEK_END );
	size_t bufferSize = ftell( file );
	fseek( file, 0L, SEEK_SET );

	char * buffer = (char *) malloc( bufferSize + 1 );
	if ( fread( buffer, 1, bufferSize, file ) != bufferSize )
	{
		if ( errorStringOut != NULL )
		{
			*errorStringOut = "failed to read file";
		}
		free( buffer );
		fclose( file );
		return false;
	}
	buffer[bufferSize] = '\0';	// make sure the buffer is zero terminated
	fclose( file );

	const char * error = NULL;
	ksJson_ParseValue( rootNode, 0, buffer, &error );
	if ( error != NULL )
	{
		if ( errorStringOut != NULL )
		{
			*errorStringOut = error;
		}
		ksJson_FreeNode( rootNode, true );
		free( buffer );
		return false;
	}

	free( buffer );
	return true;
}

static void ksJson_Printf( char ** bufferInOut, int * lengthInOut, int * offsetInOut, const int extraLength, const char * format, ... )
{
	if ( *offsetInOut + extraLength + 1 > *lengthInOut )
	{
		int newLength = ( *lengthInOut <= 0 ) ? 128 : *lengthInOut * 2;
		while ( newLength < *offsetInOut + extraLength + 1 )
		{
			newLength *= 2;
		}
		char * newBuffer = (char *) malloc( newLength );
		memcpy( newBuffer, *bufferInOut, *offsetInOut );
		free( *bufferInOut );
		*bufferInOut = newBuffer;
		*lengthInOut = newLength;
	}
	va_list args;
	va_start( args, format );
	*offsetInOut += vsprintf( &(*bufferInOut)[*offsetInOut], format, args );
	va_end( args );
	assert( *offsetInOut <= *lengthInOut );
}

static void ksJson_WriteValue( const ksJson * node, int recursion, char ** bufferInOut, int * lengthInOut, int * offsetInOut, const int indent, const bool lastChild )
{
	if ( recursion > JSON_MAX_RECURSION )
	{
		return;
	}

	const int maxIndent = 32;
	const char * indentTable = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
	assert( indent <= maxIndent );

	if ( node->type == JSON_NULL || node->type == JSON_BOOLEAN )
	{
		ksJson_Printf( bufferInOut, lengthInOut, offsetInOut, (int)strlen( node->valueString ) + 2, "%s%s\n", node->valueString, lastChild ? "" : "," );
	}
	else if ( node->type == JSON_INT )
	{
		char temp[1024];
		const int length = sprintf( temp, "%lld", (long long int)node->valueInt64 );
		ksJson_Printf( bufferInOut, lengthInOut, offsetInOut, length + 2, "%s%s\n", temp, lastChild ? "" : "," );
	}
	else if ( node->type == JSON_UINT )
	{
		char temp[1024];
		const int length = sprintf( temp, "%llu", (unsigned long long int)node->valueUint64 );
		ksJson_Printf( bufferInOut, lengthInOut, offsetInOut, length + 2, "%s%s\n", temp, lastChild ? "" : "," );
	}
	else if ( node->type == JSON_FLOAT )
	{
		char temp[1024];
		int length = sprintf( temp, "%g", node->valueDouble );
		ksJson_Printf( bufferInOut, lengthInOut, offsetInOut, length + 2, "%s%s\n", temp, lastChild ? "" : "," );
	}
	else if ( node->type == JSON_STRING )
	{
		for ( const char * ptr = node->valueString; ptr[0] != '\0'; ptr++ )
		{
			if ( (unsigned char)ptr[0] < ' ' || ptr[0] == '\"' || ptr[0] == '\\' )
			{
				switch ( ptr[0] )
				{
					case '\\': ksJson_Printf( bufferInOut, lengthInOut, offsetInOut, 2, "\\\\" ); break;
					case '\"': ksJson_Printf( bufferInOut, lengthInOut, offsetInOut, 2, "\\\"" ); break;
					case '\b': ksJson_Printf( bufferInOut, lengthInOut, offsetInOut, 2, "\\b" ); break;
					case '\f': ksJson_Printf( bufferInOut, lengthInOut, offsetInOut, 2, "\\f" ); break;
					case '\n': ksJson_Printf( bufferInOut, lengthInOut, offsetInOut, 2, "\\n" ); break;
					case '\r': ksJson_Printf( bufferInOut, lengthInOut, offsetInOut, 2, "\\r" ); break;
					case '\t': ksJson_Printf( bufferInOut, lengthInOut, offsetInOut, 2, "\\t" ); break;
					default: ksJson_Printf( bufferInOut, lengthInOut, offsetInOut, 6, "\\u%04x", ptr[0] ); break;
				}
			}
			else
			{
				ksJson_Printf( bufferInOut, lengthInOut, offsetInOut, 1, "%c", ptr[0] );
			}
		}
		ksJson_Printf( bufferInOut, lengthInOut, offsetInOut, 2, "%s\n", lastChild ? "" : "," );
	}
	else if ( node->type == JSON_OBJECT )
	{
		ksJson_Printf( bufferInOut, lengthInOut, offsetInOut, 2, "{\n" );
		if ( node->memberCount > 0 )
		{
			const int endMapIndex = MemberIndexToMapIndex( node->memberCount - 1 );
			for ( int mapIndex = 0; mapIndex <= endMapIndex; mapIndex++ )
			{
				const ksJson * members = node->memberMap[mapIndex];
				const int mapMemberCount = MapMemberCount( mapIndex, node->memberCount ); 
				for ( int i = 0; i < mapMemberCount; i++ )
				{
					const ksJson * member = &members[i];
					ksJson_Printf( bufferInOut, lengthInOut, offsetInOut, indent + 1 + (int)strlen( member->name ) + 5, "%s\"%s\" : ", &indentTable[maxIndent - ( indent + 1 )], member->name );
					ksJson_WriteValue( member, recursion + 1, bufferInOut, lengthInOut, offsetInOut, indent + 1, ( mapIndex == endMapIndex && i == mapMemberCount - 1 ) );
				}
			}
		}
		ksJson_Printf( bufferInOut, lengthInOut, offsetInOut, indent + 2, "%s}%s\n", &indentTable[maxIndent - indent], lastChild ? "" : "," );
	}
	else if ( node->type == JSON_ARRAY )
	{
		ksJson_Printf( bufferInOut, lengthInOut, offsetInOut, 2, "[\n" );
		if ( node->memberCount > 0 )
		{
			const int endMapIndex = MemberIndexToMapIndex( node->memberCount - 1 );
			for ( int mapIndex = 0; mapIndex <= endMapIndex; mapIndex++ )
			{
				const ksJson * members = node->memberMap[mapIndex];
				const int mapMemberCount = MapMemberCount( mapIndex, node->memberCount ); 
				for ( int i = 0; i < mapMemberCount; i++ )
				{
					const ksJson * member = &members[i];
					ksJson_Printf( bufferInOut, lengthInOut, offsetInOut, indent + 1, "%s", &indentTable[maxIndent - ( indent + 1 )] );
					ksJson_WriteValue( member, recursion + 1, bufferInOut, lengthInOut, offsetInOut, indent + 1, ( mapIndex == endMapIndex && i == mapMemberCount - 1 ) );
				}
			}
		}
		ksJson_Printf( bufferInOut, lengthInOut, offsetInOut, indent + 2, "%s]%s\n", &indentTable[maxIndent - indent], lastChild ? "" : "," );
	}
}

// 'lengthOut' is the length of 'bufferOut' without trailing zero.
static bool ksJson_WriteToBuffer( const ksJson * rootNode, char ** bufferOut, int * lengthOut )
{
	if ( rootNode == NULL || bufferOut == NULL || lengthOut == NULL )
	{
		return false;
	}
	*bufferOut = NULL;
	*lengthOut = 0;
	int offset = 0;
	ksJson_WriteValue( rootNode, 0, bufferOut, lengthOut, &offset, 0, true );
	*lengthOut = offset;
	return true;
}

static bool ksJson_WriteToFile( const ksJson * rootNode, const char * fileName )
{
	if ( rootNode == NULL || fileName == NULL )
	{
		return false;
	}
	char * buffer = NULL;
	int length = 0;
	int offset = 0;
	ksJson_WriteValue( rootNode, 0, &buffer, &length, &offset, 0, true );

	FILE * file = fopen( fileName, "wb" );
	if ( file == NULL )
	{
		free( buffer );
		return false;
	}
	if ( fwrite( buffer, 1, offset, file ) != (size_t) offset )
	{
		free( buffer );
		fclose( file );
		return false;
	}
	fclose( file );
	free( buffer );
	return true;
}

static int ksJson_GetMemberCount( const ksJson * node )
{
	if ( node != NULL )
	{
		assert( node->type >= JSON_NULL && node->type <= JSON_ARRAY );
		return node->memberCount;
	}
	return 0;
}

static ksJson * ksJson_GetMemberByIndex( const ksJson * node, const int index )
{
	if ( node != NULL )
	{
		assert( node->type >= JSON_NULL && node->type <= JSON_ARRAY );
		if ( node->type == JSON_OBJECT || node->type == JSON_ARRAY )
		{
			if ( index >= 0 && index < node->memberCount )
			{
				const int mapIndex = MemberIndexToMapIndex( index );
				const int memberOffset = MapMemberOffset( mapIndex );
				return &node->memberMap[mapIndex][index - memberOffset];
			}
		}
	}
	return NULL;
}

static ksJson * ksJson_GetMemberByName( const ksJson * node, const char * name )
{
	if ( node != NULL && node->type == JSON_OBJECT && node->memberCount > 0 )
	{
		assert( name != NULL );
		const int startMapIndex = MemberIndexToMapIndex( node->memberIndex );
		const int endMapIndex = MemberIndexToMapIndex( node->memberCount - 1 );
		int firstMemberOffset = node->memberIndex - MapMemberOffset( startMapIndex );
		for ( int mapIndex = startMapIndex; mapIndex <= endMapIndex; mapIndex++ )
		{
			ksJson * members = node->memberMap[mapIndex];
			const int mapMemberCount = MapMemberCount( mapIndex, node->memberCount ); 
			for ( int i = firstMemberOffset; i < mapMemberCount; i++ )
			{
				if ( strcmp( members[i].name, name ) == 0 )
				{
					const int newMemberIndex = MapMemberOffset( mapIndex ) + i + 1;
					*(int *)&node->memberIndex = ( newMemberIndex < node->memberCount ) ? newMemberIndex : 0;	// mutable
					return &members[i];
				}
			}
			firstMemberOffset = 0;
		}
		for ( int mapIndex = 0; mapIndex <= startMapIndex; mapIndex++ )
		{
			ksJson * members = node->memberMap[mapIndex];
			const int mapMemberCount = MapMemberCount( mapIndex, node->memberIndex ); 
			for ( int i = 0; i < mapMemberCount; i++ )
			{
				if ( strcmp( members[i].name, name ) == 0 )
				{
					const int newMemberIndex = MapMemberOffset( mapIndex ) + i + 1;
					*(int *)&node->memberIndex = ( newMemberIndex < node->memberCount ) ? newMemberIndex : 0;	// mutable
					return &members[i];
				}
			}
		}
	}
	return NULL;
}

const char * ksJson_GetMemberName( const ksJson * node )
{
	if ( node != NULL && node->name != NULL )
	{
		assert( node->type >= JSON_NULL && node->type <= JSON_ARRAY );
		return node->name;
	}
	return "";
}

static inline bool ksJson_IsNull( const ksJson * node )
{
	return ( node != NULL && node->type == JSON_NULL );
}

static inline bool ksJson_IsBoolean( const ksJson * node )
{
	return ( node != NULL && node->type == JSON_BOOLEAN );
}

static inline bool ksJson_IsNumber( const ksJson * node )
{
	return ( node != NULL && ( node->type == JSON_INT || node->type == JSON_UINT || node->type == JSON_FLOAT ) );
}

static inline bool ksJson_IsInteger( const ksJson * node )
{
	return ( node != NULL && ( node->type == JSON_INT || node->type == JSON_UINT ) );
}

static inline bool ksJson_IsUnsigned( const ksJson * node )
{
	return ( node != NULL && node->type == JSON_UINT );
}

static inline bool ksJson_IsFloatingPoint( const ksJson * node )
{
	return ( node != NULL && node->type == JSON_FLOAT );
}

static inline bool ksJson_IsString( const ksJson * node )
{
	return ( node != NULL && node->type == JSON_STRING );
}

static inline bool ksJson_IsObject( const ksJson * node )
{
	return ( node != NULL && node->type == JSON_OBJECT );
}

static inline bool ksJson_IsArray( const ksJson * node )
{
	return ( node != NULL && node->type == JSON_ARRAY );
}

static inline bool ksJson_GetBool( const ksJson * node, const bool defaultValue )
{
	return ksJson_IsBoolean( node ) ? node->valueString[0] == 't' : defaultValue;
}

static inline int8_t ksJson_GetInt8( const ksJson * node, const int8_t defaultValue )
{
	return	( ( node == NULL ) ?				defaultValue :
			( ( node->type == JSON_INT ) ?		(int8_t)JSON_CLAMP( node->valueInt64, INT8_MIN, INT8_MAX ) :
			( ( node->type == JSON_UINT ) ?		(int8_t)JSON_MIN( node->valueUint64, INT8_MAX ) :
			( ( node->type == JSON_FLOAT ) ?	(int8_t)JSON_CLAMP( node->valueDouble, INT8_MIN, INT8_MAX ) : defaultValue ) ) ) );
}

static inline uint8_t ksJson_GetUint8( const ksJson * node, const uint8_t defaultValue )
{
	return	( ( node == NULL ) ?				defaultValue :
			( ( node->type == JSON_UINT ) ?		(uint8_t)JSON_MIN( node->valueUint64, UINT8_MAX ) :
			( ( node->type == JSON_INT ) ?		(uint8_t)JSON_CLAMP( node->valueInt64, 0, UINT8_MAX ) :
			( ( node->type == JSON_FLOAT ) ?	(uint8_t)JSON_CLAMP( node->valueDouble, 0, UINT8_MAX ) : defaultValue ) ) ) );
}

static inline int16_t ksJson_GetInt16( const ksJson * node, const int16_t defaultValue )
{
	return	( ( node == NULL ) ?				defaultValue :
			( ( node->type == JSON_INT ) ?		(int16_t)JSON_CLAMP( node->valueInt64, INT16_MIN, INT16_MAX ) :
			( ( node->type == JSON_UINT ) ?		(int16_t)JSON_MIN( node->valueUint64, INT16_MAX ) :
			( ( node->type == JSON_FLOAT ) ?	(int16_t)JSON_CLAMP( node->valueDouble, INT16_MIN, INT16_MAX ) : defaultValue ) ) ) );
}

static inline uint16_t ksJson_GetUint16( const ksJson * node, const uint16_t defaultValue )
{
	return	( ( node == NULL ) ?				defaultValue :
			( ( node->type == JSON_UINT ) ?		(uint16_t)JSON_MIN( node->valueUint64, UINT16_MAX ) :
			( ( node->type == JSON_INT ) ?		(uint16_t)JSON_CLAMP( node->valueInt64, 0, UINT16_MAX ) :
			( ( node->type == JSON_FLOAT ) ?	(uint16_t)JSON_CLAMP( node->valueDouble, 0, UINT16_MAX ) : defaultValue ) ) ) );
}

static inline int32_t ksJson_GetInt32( const ksJson * node, const int32_t defaultValue )
{
	return	( ( node == NULL ) ?				defaultValue :
			( ( node->type == JSON_INT ) ?		(int32_t)JSON_CLAMP( node->valueInt64, INT32_MIN, INT32_MAX ) :
			( ( node->type == JSON_UINT ) ?		(int32_t)JSON_MIN( node->valueUint64, INT32_MAX ) :
			( ( node->type == JSON_FLOAT ) ?	(int32_t)JSON_CLAMP( node->valueDouble, INT32_MIN, INT32_MAX ) : defaultValue ) ) ) );
}

static inline uint32_t ksJson_GetUint32( const ksJson * node, const uint32_t defaultValue )
{
	return	( ( node == NULL ) ?				defaultValue :
			( ( node->type == JSON_UINT ) ?		(uint32_t)JSON_MIN( node->valueUint64, UINT32_MAX ) :
			( ( node->type == JSON_INT ) ?		(uint32_t)JSON_CLAMP( node->valueInt64, 0, UINT32_MAX ) :
			( ( node->type == JSON_FLOAT ) ?	(uint32_t)JSON_CLAMP( node->valueDouble, 0, UINT32_MAX ) : defaultValue ) ) ) );
}

static inline int64_t ksJson_GetInt64( const ksJson * node, const int64_t defaultValue )
{
	return	( ( node == NULL ) ?				defaultValue :
			( ( node->type == JSON_INT ) ?		(int64_t)node->valueInt64 :
			( ( node->type == JSON_UINT ) ?		(int64_t)JSON_MIN( node->valueUint64, INT64_MAX ) :
			( ( node->type == JSON_FLOAT ) ?	(int64_t)JSON_CLAMP( node->valueDouble, INT64_MIN, INT64_MAX ) : defaultValue ) ) ) );
}

static inline uint64_t ksJson_GetUint64( const ksJson * node, const uint64_t defaultValue )
{
	return	( ( node == NULL ) ?				defaultValue :
			( ( node->type == JSON_UINT ) ?		(uint64_t)node->valueUint64 :
			( ( node->type == JSON_INT ) ?		(uint64_t)JSON_CLAMP( node->valueInt64, 0, INT64_MAX ) :
			( ( node->type == JSON_FLOAT ) ?	(uint64_t)JSON_CLAMP( node->valueDouble, 0, UINT64_MAX ) : defaultValue ) ) ) );
}

static inline float ksJson_GetFloat( const ksJson * node, const float defaultValue )
{
	return	( ( node == NULL ) ?				defaultValue :
			( ( node->type == JSON_FLOAT ) ?	(float)JSON_CLAMP( node->valueDouble, -FLT_MAX, FLT_MAX ) :
			( ( node->type == JSON_INT ) ?		(float)node->valueInt64 :
			( ( node->type == JSON_UINT ) ?		(float)node->valueUint64 : defaultValue ) ) ) );
}

static inline double ksJson_GetDouble( const ksJson * node, const double defaultValue )
{
	return	( ( node == NULL ) ?				defaultValue :
			( ( node->type == JSON_FLOAT ) ?	(double)node->valueDouble :
			( ( node->type == JSON_INT ) ?		(double)node->valueInt64 :
			( ( node->type == JSON_UINT ) ?		(double)node->valueUint64 : defaultValue ) ) ) );
}

static inline const char * ksJson_GetString( const ksJson * node, const char * defaultValue )
{
	return ksJson_IsString( node ) ? node->valueString : defaultValue;
}

static ksJson * ksJson_AddObjectMember( ksJson * node, const char * name )
{
	if ( node != NULL && node->type == JSON_OBJECT )
	{
		assert( name != NULL );
		ksJson * member = ksJson_AllocMember( node );
		const size_t length = strlen( name );
		member->name = (char *) malloc( length + 1 );
		strcpy( member->name, name );
		member->name[length] = '\0';
		return member;
	}
	return NULL;
}

static ksJson * ksJson_AddArrayElement( ksJson * node )
{
	if ( node != NULL && node->type == JSON_ARRAY )
	{
		ksJson * element = ksJson_AllocMember( node );
		return element;
	}
	return NULL;
}

static inline ksJson * ksJson_SetObject( ksJson * node )
{
	if ( node != NULL )
	{
		ksJson_FreeNode( node, false );
		node->type = JSON_OBJECT;
	}
	return node;
}

static inline ksJson * ksJson_SetArray( ksJson * node )
{
	if ( node != NULL )
	{
		ksJson_FreeNode( node, false );
		node->type = JSON_ARRAY;
	}
	return node;
}

static inline ksJson * ksJson_SetNull( ksJson * node )
{
	if ( node != NULL )
	{
		ksJson_FreeNode( node, false );
		node->type = JSON_NULL;
		node->valueString = (char *)"null";
	}
	return node;
}

static inline ksJson * ksJson_SetBoolean( ksJson * node, const bool value )
{
	if ( node != NULL )
	{
		ksJson_FreeNode( node, false );
		node->type = JSON_BOOLEAN;
		node->valueString = value ? (char *)"true" : (char *)"false";
	}
	return node;
}

static inline ksJson * ksJson_SetInt8( ksJson * node, const int8_t value )
{
	if ( node != NULL )
	{
		ksJson_FreeNode( node, false );
		node->type = JSON_INT;
		node->valueInt64 = value;
	}
	return node;
}

static inline ksJson * ksJson_SetUint8( ksJson * node, const uint8_t value )
{
	if ( node != NULL )
	{
		ksJson_FreeNode( node, false );
		node->type = JSON_UINT;
		node->valueUint64 = value;
	}
	return node;
}

static inline ksJson * ksJson_SetInt16( ksJson * node, const int16_t value )
{
	if ( node != NULL )
	{
		ksJson_FreeNode( node, false );
		node->type = JSON_INT;
		node->valueInt64 = value;
	}
	return node;
}

static inline ksJson * ksJson_SetUint16( ksJson * node, const uint16_t value )
{
	if ( node != NULL )
	{
		ksJson_FreeNode( node, false );
		node->type = JSON_UINT;
		node->valueUint64 = value;
	}
	return node;
}

static inline ksJson * ksJson_SetInt32( ksJson * node, const int32_t value )
{
	if ( node != NULL )
	{
		ksJson_FreeNode( node, false );
		node->type = JSON_INT;
		node->valueInt64 = value;
	}
	return node;
}

static inline ksJson * ksJson_SetUint32( ksJson * node, const uint32_t value )
{
	if ( node != NULL )
	{
		ksJson_FreeNode( node, false );
		node->type = JSON_UINT;
		node->valueUint64 = value;
	}
	return node;
}

static inline ksJson * ksJson_SetInt64( ksJson * node, const int64_t value )
{
	if ( node != NULL )
	{
		ksJson_FreeNode( node, false );
		node->type = JSON_INT;
		node->valueInt64 = value;
	}
	return node;
}

static inline ksJson * ksJson_SetUint64( ksJson * node, const uint64_t value )
{
	if ( node != NULL )
	{
		ksJson_FreeNode( node, false );
		node->type = JSON_UINT;
		node->valueUint64 = value;
	}
	return node;
}

static inline ksJson * ksJson_SetFloat( ksJson * node, const float value )
{
	if ( node != NULL )
	{
		ksJson_FreeNode( node, false );
		node->type = JSON_FLOAT;
		node->valueDouble = value;
	}
	return node;
}

static inline ksJson * ksJson_SetDouble( ksJson * node, const double value )
{
	if ( node != NULL )
	{
		ksJson_FreeNode( node, false );
		node->type = JSON_FLOAT;
		node->valueDouble = value;
	}
	return node;
}

static inline ksJson * ksJson_SetString( ksJson * node, const char * value )
{
	if ( node != NULL )
	{
		assert( value != NULL );
		ksJson_FreeNode( node, false );
		node->type = JSON_STRING;
		const int length = (int)strlen( value );
		node->valueString = (char *) malloc( length + 1 );
		strcpy( node->valueString, value );
	}
	return node;
}

#endif // !KSJSON_H
