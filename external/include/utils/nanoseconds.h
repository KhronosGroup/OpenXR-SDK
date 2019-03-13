/*
================================================================================================

Description	:	Time in nanoseconds.
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

================================================================================================
*/

#if !defined( KSNANOSECONDS_H )
#define KSNANOSECONDS_H

#if defined( WIN32 ) || defined( _WIN32 ) || defined( WIN64 ) || defined( _WIN64 )
	#if !defined( OS_WINDOWS )
		#define OS_WINDOWS
	#endif
#elif defined( __ANDROID__ )
	#if !defined( OS_ANDROID )
		#define OS_ANDROID
	#endif
#elif defined( __hexagon__ ) || defined( __qdsp6__ )
	#if !defined( OS_HEXAGON )
		#define OS_HEXAGON
	#endif
#elif defined( __APPLE__ )
	#if !defined( OS_APPLE )
		#define OS_APPLE
	#endif
	#include <Availability.h>
	#if __IPHONE_OS_VERSION_MAX_ALLOWED && !defined( OS_APPLE_IOS )
		#define OS_APPLE_IOS
	#elif __MAC_OS_X_VERSION_MAX_ALLOWED && !defined( OS_APPLE_MACOS )
		#define OS_APPLE_MACOS
	#endif
#elif defined( __linux__ )
	#if !defined( OS_LINUX )
		#define OS_LINUX
	#endif
#else
	#error "unknown platform"
#endif

#if defined( OS_WINDOWS )
	#include <windows.h>
#elif defined( OS_LINUX )
	#include <time.h>							// for timespec
	#include <sys/time.h>						// for gettimeofday()
#elif defined( OS_APPLE )
	#include <sys/time.h>
#elif defined( OS_ANDROID )
	#include <time.h>
#elif defined( OS_HEXAGON )
	#include <qurt_timer.h>
#endif

#include <stdint.h>

typedef uint64_t ksNanoseconds;

static ksNanoseconds GetTimeNanoseconds()
{
#if defined( OS_WINDOWS )
	static ksNanoseconds ticksPerSecond = 0;
	static ksNanoseconds timeBase = 0;

	if ( ticksPerSecond == 0 )
	{
		LARGE_INTEGER li;
		QueryPerformanceFrequency( &li );
		ticksPerSecond = (ksNanoseconds) li.QuadPart;
		QueryPerformanceCounter( &li );
		timeBase = (ksNanoseconds) li.LowPart + 0xFFFFFFFFULL * li.HighPart;
	}

	LARGE_INTEGER li;
	QueryPerformanceCounter( &li );
	ksNanoseconds counter = (ksNanoseconds) li.LowPart + 0xFFFFFFFFULL * li.HighPart;
	return ( counter - timeBase ) * 1000ULL * 1000ULL * 1000ULL / ticksPerSecond;
#elif defined( OS_ANDROID )
	static ksNanoseconds timeBase = 0;

	struct timespec ts;
	clock_gettime( CLOCK_MONOTONIC, &ts );

	if ( timeBase == 0 )
	{
		timeBase = (ksNanoseconds) ts.tv_sec * 1000ULL * 1000ULL * 1000ULL + ts.tv_nsec;
	}

	return (ksNanoseconds) ts.tv_sec * 1000ULL * 1000ULL * 1000ULL + ts.tv_nsec - timeBase;
#elif defined( OS_HEXAGON )
	return QURT_TIMER_TIMETICK_TO_US( qurt_timer_get_ticks() ) * 1000;
#else
	static ksNanoseconds timeBase = 0;

	struct timeval tv;
	gettimeofday( &tv, 0 );

	if ( timeBase == 0 )
	{
		timeBase = (ksNanoseconds) tv.tv_sec * 1000ULL * 1000ULL * 1000ULL + tv.tv_usec * 1000ULL;
	}

	return (ksNanoseconds) tv.tv_sec * 1000ULL * 1000ULL * 1000ULL + tv.tv_usec * 1000ULL - timeBase;
#endif
}

#endif // !KSNANOSECONDS_H
