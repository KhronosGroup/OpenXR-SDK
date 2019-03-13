/*
================================================================================================

Description	:	Threading primitives.
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

#if !defined( KSTHREADING_H )
#define KSTHREADING_H

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
	#include <pthread.h>						// for pthread_create() etc.
#elif defined( OS_APPLE )
	#include <sys/time.h>
	#include <pthread.h>
#elif defined( OS_ANDROID )
	#include <time.h>
	#include <unistd.h>
	#include <pthread.h>
	#include <sys/prctl.h>						// for prctl( PR_SET_NAME )
	#include <sys/stat.h>						// for gettid
	#include <sys/syscall.h>					// for syscall
#elif defined( OS_HEXAGON )
	#include "qurt.h"
	#include "qurt_atomic_ops.h"
#endif

#include <stdbool.h>
#include "nanoseconds.h"

#if !defined( UNUSED_PARM )
#define UNUSED_PARM( x )				{ (void)(x); }
#endif

/*
================================================================================================================================

Atomic 32-bit unsigned integer

================================================================================================================================
*/

typedef unsigned int ksAtomicUint32;

static ksAtomicUint32 ksAtomicUint32_Increment( ksAtomicUint32 * atomicUint32 )
{
#if defined( OS_WINDOWS )
	return (ksAtomicUint32) InterlockedIncrement( (LONG *)atomicUint32 );
#elif defined( OS_HEXAGON )
	return qurt_atomic_inc_return( atomicUint32 );
#else
	return __sync_fetch_and_add( atomicUint32, 1 );
#endif
}

static ksAtomicUint32 ksAtomicUint32_Decrement( ksAtomicUint32 * atomicUint32 )
{
#if defined( OS_WINDOWS )
	return (ksAtomicUint32) InterlockedDecrement( (LONG *)atomicUint32 );
#elif defined( OS_HEXAGON )
	return qurt_atomic_dec_return( atomicUint32 );
#else
	return __sync_fetch_and_add( atomicUint32, -1 );
#endif
}

/*
================================================================================================================================

Mutex for mutual exclusion on shared resources within a single process.

Equivalent to a Windows Critical Section Object which allows recursive access. This mutex cannot be
used for mutual-exclusion synchronization between threads from different processes.

ksMutex

static void ksMutex_Create( ksMutex * mutex );
static void ksMutex_Destroy( ksMutex * mutex );
static bool ksMutex_Lock( ksMutex * mutex, const bool blocking );
static void ksMutex_Unlock( ksMutex * mutex );

================================================================================================================================
*/

typedef struct
{
#if defined( OS_WINDOWS )
	CRITICAL_SECTION	handle;
#elif defined( OS_HEXAGON )
	qurt_mutex_t		mutex;
#else
	pthread_mutex_t		mutex;
#endif
} ksMutex;

static void ksMutex_Create( ksMutex * mutex )
{
#if defined( OS_WINDOWS )
	InitializeCriticalSection( &mutex->handle );
#elif defined( OS_HEXAGON )
	qurt_rmutex_init( &mutex->mutex );
#else
	pthread_mutexattr_t attr;
	pthread_mutexattr_init( &attr );
	pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );
	pthread_mutex_init( &mutex->mutex, &attr );
#endif
}

static void ksMutex_Destroy( ksMutex * mutex )
{
#if defined( OS_WINDOWS )
	DeleteCriticalSection( &mutex->handle );
#elif defined( OS_HEXAGON )
	qurt_rmutex_destroy( &mutex->mutex );
#else
	pthread_mutex_destroy( &mutex->mutex );
#endif
}

static bool ksMutex_Lock( ksMutex * mutex, const bool blocking )
{
#if defined( OS_WINDOWS )
	if ( TryEnterCriticalSection( &mutex->handle ) == 0 )
	{
		if ( !blocking )
		{
			return false;
		}
		EnterCriticalSection( &mutex->handle );
	}
	return true;
#elif defined( OS_HEXAGON )
	if ( qurt_rmutex_try_lock( &mutex->mutex ) != 0 )
	{
		if ( !blocking )
		{
			return false;
		}
		qurt_rmutex_lock( &mutex->mutex );
	}
	return true;
#else
	if ( pthread_mutex_trylock( &mutex->mutex ) == EBUSY )
	{
		if ( !blocking )
		{
			return false;
		}
		pthread_mutex_lock( &mutex->mutex );
	}
	return true;
#endif
}

static void ksMutex_Unlock( ksMutex * mutex )
{
#if defined( OS_WINDOWS )
	LeaveCriticalSection( &mutex->handle );
#elif defined( OS_HEXAGON )
	qurt_rmutex_unlock( &mutex->mutex );
#else
	pthread_mutex_unlock( &mutex->mutex );
#endif
}

/*
================================================================================================================================

Signal for thread synchronization, similar to a Windows event object which only supports SetEvent.

Windows event objects come in two types: auto-reset events and manual-reset events. A Windows event object
can be signalled by calling either SetEvent or PulseEvent.

When a manual-reset event is signaled by calling SetEvent, it sets the event into the signaled state and
wakes up all threads waiting on the event. The manual-reset event remains in the signalled state until
the event is manually reset. When an auto-reset event is signaled by calling SetEvent and there are any
threads waiting, it wakes up only one thread and resets the event to the non-signaled state. If there are
no threads waiting for an auto-reset event, then the event remains signaled until a single waiting thread
waits on it and is released.

When a manual-reset event is signaled by calling PulseEvent, it wakes up all waiting threads and atomically
resets the event. When an auto-reset event is signaled by calling PulseEvent, and there are any threads
waiting, it wakes up only one thread and resets the event to the non-signaled state. If there are no threads
waiting, then no threads are released and the event is set to the non-signaled state.

A Windows event object has limited functionality compared to a POSIX condition variable. Unlike a
Windows event object, the expression waited upon by a POSIX condition variable can be arbitrarily complex.
Furthermore, there is no way to release just one waiting thread with a manual-reset Windows event object.
Similarly there is no way to release all waiting threads with an auto-reset Windows event object.
These limitations make it difficult to simulate a POSIX condition variable using Windows event objects.

Windows Vista and later implement PCONDITION_VARIABLE, but as Douglas C. Schmidt and Irfan Pyarali point
out, it is complicated to simulate a POSIX condition variable on prior versions of Windows without causing
unfair or even incorrect behavior:

	1. "Strategies for Implementing POSIX Condition Variables on Win32"
	   http://www.cs.wustl.edu/~schmidt/win32-cv-1.html
	2. "Patterns for POSIX Condition Variables on Win32"
	   http://www.cs.wustl.edu/~schmidt/win32-cv-2.html

Even using SignalObjectAndWait is not safe because as per the Microsoft documentation: "Note that the 'signal'
and 'wait' are not guaranteed to be performed as an atomic operation. Threads executing on other processors
can observe the signaled state of the first object before the thread calling SignalObjectAndWait begins its
wait on the second object."

Simulating a Windows event object using a POSIX condition variable is fairly straight forward, which
is done here. However, this implementation does not support the equivalent of PulseEvent, because
PulseEvent is unreliable. On Windows, a thread waiting on an event object can be momentarily removed
from the wait state by a kernel-mode Asynchronous Procedure Call (APC), and then returned to the wait
state after the APC is complete. If a call to PulseEvent occurs during the time when the thread has
been temporarily removed from the wait state, then the thread will not be released, because PulseEvent
releases only those threads that are in the wait state at the moment PulseEvent is called.

ksSignal

static void ksSignal_Create( ksSignal * signal, const bool autoReset );
static void ksSignal_Destroy( ksSignal * signal );
static bool ksSignal_Wait( ksSignal * signal, const ksNanoseconds timeOutNanoseconds );
static void ksSignal_Raise( ksSignal * signal );
static void ksSignal_Clear( ksSignal * signal );

================================================================================================================================
*/

#define SIGNAL_TIMEOUT_INFINITE		0xFFFFFFFFFFFFFFFFULL

typedef struct
{
#if defined( OS_WINDOWS )
	HANDLE			handle;
#elif defined( OS_HEXAGON )
	qurt_mutex_t	mutex;
	qurt_cond_t		cond;
	int				waitCount;		// number of threads waiting on the signal
	bool			autoReset;		// automatically clear the signalled state when a single thread is released
	bool			signaled;		// in the signalled state if true
#else
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
	int				waitCount;		// number of threads waiting on the signal
	bool			autoReset;		// automatically clear the signalled state when a single thread is released
	bool			signaled;		// in the signalled state if true
#endif
} ksSignal;

static void ksSignal_Create( ksSignal * signal, const bool autoReset )
{
#if defined( OS_WINDOWS )
	signal->handle = CreateEventA( NULL, !autoReset, FALSE, NULL );
#elif defined( OS_HEXAGON )
	qurt_mutex_init( &signal->mutex );
	qurt_cond_init( &signal->cond );
	signal->waitCount = 0;
	signal->autoReset = autoReset;
	signal->signaled = false;
#else
	pthread_mutex_init( &signal->mutex, NULL );
	pthread_cond_init( &signal->cond, NULL );
	signal->waitCount = 0;
	signal->autoReset = autoReset;
	signal->signaled = false;
#endif
}

static void ksSignal_Destroy( ksSignal * signal )
{
#if defined( OS_WINDOWS )
	CloseHandle( signal->handle );
#elif defined( OS_HEXAGON )
	qurt_cond_destroy( &signal->cond );
	qurt_mutex_destroy( &signal->mutex );
#else
	pthread_cond_destroy( &signal->cond );
	pthread_mutex_destroy( &signal->mutex );
#endif
}

// Waits for the object to enter the signalled state and returns true if this state is reached within the time-out period.
// If 'autoReset' is true then the first thread that reaches the signalled state within the time-out period will clear the signalled state.
// If 'timeOutNanoseconds' is SIGNAL_TIMEOUT_INFINITE then this will wait indefinitely until the signalled state is reached.
// Returns true if the thread was released because the object entered the signalled state, returns false if the time-out is reached first.
static bool ksSignal_Wait( ksSignal * signal, const ksNanoseconds timeOutNanoseconds )
{
#if defined( OS_WINDOWS )
	DWORD result = WaitForSingleObject( signal->handle, ( timeOutNanoseconds == SIGNAL_TIMEOUT_INFINITE ) ? INFINITE : (DWORD)( timeOutNanoseconds / ( 1000 * 1000 ) ) );
	assert( result == WAIT_OBJECT_0 || ( timeOutNanoseconds != SIGNAL_TIMEOUT_INFINITE && result == WAIT_TIMEOUT ) );
	return ( result == WAIT_OBJECT_0 );
#elif defined( OS_HEXAGON )
	bool released = false;
	qurt_mutex_lock( &signal->mutex );
	if ( signal->signaled )
	{
		released = true;
	}
	else
	{
		signal->waitCount++;
		if ( timeOutMicroseconds == SIGNAL_TIMEOUT_INFINITE )
		{
			do
			{
				qurt_cond_wait( &signal->cond, &signal->mutex );
				// Re-check condition in case qurt_cond_wait spuriously woke up.
			} while ( signal->signaled == false );
		}
		else if ( timeOutMicroseconds > 0 )
		{
			// No support for a time-out other than zero.
			//assert( false );
		}
		released = signal->signaled;
		signal->waitCount--;
	}
	if ( released && signal->autoReset )
	{
		signal->signaled = false;
	}
	qurt_mutex_unlock( &signal->mutex );
	return released;
#else
	bool released = false;
	pthread_mutex_lock( &signal->mutex );
	if ( signal->signaled )
	{
		released = true;
	}
	else
	{
		signal->waitCount++;
		if ( timeOutNanoseconds == SIGNAL_TIMEOUT_INFINITE )
		{
			do
			{
				pthread_cond_wait( &signal->cond, &signal->mutex );
				// Must re-check condition because pthread_cond_wait may spuriously wake up.
			} while ( signal->signaled == false );
		}
		else if ( timeOutNanoseconds > 0 )
		{
			struct timeval tp;
			gettimeofday( &tp, NULL );
			struct timespec ts;
			ts.tv_sec = (time_t)( tp.tv_sec + timeOutNanoseconds / ( 1000 * 1000 * 1000 ) );
			ts.tv_nsec = (long)( tp.tv_usec + ( timeOutNanoseconds % ( 1000 * 1000 * 1000 ) ) );
			do
			{
				if ( pthread_cond_timedwait( &signal->cond, &signal->mutex, &ts ) == ETIMEDOUT )
				{
					break;
				}
				// Must re-check condition because pthread_cond_timedwait may spuriously wake up.
			} while ( signal->signaled == false );
		}
		released = signal->signaled;
		signal->waitCount--;
	}
	if ( released && signal->autoReset )
	{
		signal->signaled = false;
	}
	pthread_mutex_unlock( &signal->mutex );
	return released;
#endif
}

// Enter the signalled state.
// Note that if 'autoReset' is true then this will only release a single thread.
static void ksSignal_Raise( ksSignal * signal )
{
#if defined( OS_WINDOWS )
	SetEvent( signal->handle );
#elif defined( OS_HEXAGON )
	qurt_mutex_lock( &signal->mutex );
	signal->signaled = true;
	if ( signal->waitCount > 0 )
	{
		qurt_cond_broadcast( &signal->cond );
	}
	qurt_mutex_unlock( &signal->mutex );
#else
	pthread_mutex_lock( &signal->mutex );
	signal->signaled = true;
	if ( signal->waitCount > 0 )
	{
		pthread_cond_broadcast( &signal->cond );
	}
	pthread_mutex_unlock( &signal->mutex );
#endif
}

// Clear the signalled state.
// Should not be needed for auto-reset signals (autoReset == true).
static void ksSignal_Clear( ksSignal * signal )
{
#if defined( OS_WINDOWS )
	ResetEvent( signal->handle );
#elif defined( OS_HEXAGON )
	qurt_mutex_lock( &signal->mutex );
	signal->signaled = false;
	qurt_mutex_unlock( &signal->mutex );
#else
	pthread_mutex_lock( &signal->mutex );
	signal->signaled = false;
	pthread_mutex_unlock( &signal->mutex );
#endif
}

/*
================================================================================================================================

Worker thread.

When the thread is first created, it will be in a suspended state. The thread function will be
called as soon as the thread is signalled. If the thread is not signalled again, then the thread
will return to a suspended state as soon as the thread function returns. The thread function will
be called again by signalling the thread again. The thread function will be called again right
away, when the thread is signalled during the execution of the thread function. Signalling the
thread more than once during the execution of the thread function does not cause the thread
function to be called multiple times. The thread can be joined to wait for the thread function
to return.

This worker thread will function as a normal thread by immediately signalling the thread after creation.
Once the thread function returns, the thread can be destroyed. Destroying the thread always waits
for the thread function to return first.

ksThread

static bool ksThread_Create( ksThread * thread, const char * threadName, ksThreadFunction threadFunction, void * threadData );
static void ksThread_Destroy( ksThread * thread );
static void ksThread_Signal( ksThread * thread );
static void ksThread_Join( ksThread * thread );
static void ksThread_Submit( ksThread * thread, ksThreadFunction threadFunction, void * threadData );

// These must be called from the thread itself.
static void ksThread_SetName( const char * name );
static void ksThread_SetAffinity( int mask );
static void ksThread_SetRealTimePriority( int priority );

================================================================================================================================
*/

typedef void (*ksThreadFunction)( void * data );

#if defined( OS_WINDOWS )
#define THREAD_HANDLE			HANDLE
#define THREAD_RETURN_TYPE		int
#define THREAD_RETURN_VALUE		0
#elif defined( OS_HEXAGON )
#define THREAD_HANDLE			qurt_thread_t
#define THREAD_RETURN_TYPE		void
#define THREAD_RETURN_VALUE
#else
#define THREAD_HANDLE			pthread_t
#define THREAD_RETURN_TYPE		void *
#define THREAD_RETURN_VALUE		0
#endif

#define THREAD_AFFINITY_BIG_CORES		-1

typedef struct
{
	char				threadName[128];
	ksThreadFunction	threadFunction;
	void *				threadData;

	void *				stack;
	THREAD_HANDLE		handle;
	ksSignal			workIsDone;
	ksSignal			workIsAvailable;
	ksMutex				workMutex;
	volatile bool		terminate;
} ksThread;

// Note that on Android AttachCurrentThread will reset the thread name.
static void ksThread_SetName( const char * name )
{
#if defined( OS_WINDOWS )
	static const unsigned int MS_VC_EXCEPTION = 0x406D1388;

#pragma pack( push, 8 )
	typedef struct
	{
		DWORD dwType;		// Must be 0x1000.
		LPCSTR szName;		// Pointer to name (in user address space).
		DWORD dwThreadID;	// Thread ID (-1 = caller thread).
		DWORD dwFlags;		// Reserved for future use, must be zero.
	} THREADNAME_INFO;
#pragma pack( pop )

	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = name;
	info.dwThreadID = (DWORD)( -1 );
	info.dwFlags = 0;
	__try
	{
		RaiseException( MS_VC_EXCEPTION, 0, sizeof( info ) / sizeof( ULONG_PTR ), (const ULONG_PTR *)&info );
	}
	__except( GetExceptionCode() == MS_VC_EXCEPTION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		info.dwFlags = 0;
	}
#elif defined( OS_LINUX )
	pthread_setname_np( pthread_self(), name );
#elif defined( OS_APPLE )
	pthread_setname_np( name );
#elif defined( OS_ANDROID )
	prctl( PR_SET_NAME, (long)name, 0, 0, 0 );
#endif
}

static void ksThread_SetAffinity( int mask )
{
#if defined( OS_WINDOWS )
	if ( mask == THREAD_AFFINITY_BIG_CORES )
	{
		return;
	}
	HANDLE thread = GetCurrentThread();
	if ( !SetThreadAffinityMask( thread, mask ) )
	{
		char buffer[1024];
		DWORD error = GetLastError();
		FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), buffer, sizeof( buffer ), NULL );
		printf( "Failed to set thread %p affinity: %s(%lu)\n", thread, buffer, error );
	}
	else
	{
		printf( "Thread %p affinity set to 0x%02X\n", thread, mask );
	}
#elif defined( OS_LINUX )
	if ( mask == THREAD_AFFINITY_BIG_CORES )
	{
		return;
	}
	cpu_set_t set;
	memset( &set, 0, sizeof( cpu_set_t ) );
	for ( int bit = 0; bit < 32; bit++ )
	{
		if ( ( mask & ( 1 << bit ) ) != 0 )
		{
			set.__bits[bit / sizeof( set.__bits[0] )] |= 1 << ( bit & ( sizeof( set.__bits[0] ) - 1 ) );
		}
	}
	const int result = pthread_setaffinity_np( pthread_self(), sizeof( cpu_set_t ), &set );
	if ( result != 0 )
	{
		printf( "Failed to set thread %d affinity.\n", (unsigned int)pthread_self() );
	}
	else
	{
		printf( "Thread %d affinity set to 0x%02X\n", (unsigned int)pthread_self(), mask );
	}
#elif defined( OS_APPLE )
	// macOS and iOS do not export interfaces that identify processors or control thread placement.
	UNUSED_PARM( mask );
#elif defined( OS_ANDROID )
	// Optionally use the faster cores of a heterogeneous CPU.
	if ( mask == THREAD_AFFINITY_BIG_CORES )
	{
		mask = 0;
		unsigned int bestFrequency = 0;
		for ( int i = 0; i < 16; i++ )
		{
			int maxFrequency = 0;
			const char * files[] =
			{
				"scaling_available_frequencies",	// not available on all devices
				"scaling_max_freq",					// no user read permission on all devices
				"cpuinfo_max_freq",					// could be set lower than the actual max, but better than nothing
			};
			for ( int j = 0; j < ( sizeof(files) / sizeof(files[0]) ); j++ )
			{
				char fileName[1024];
				sprintf( fileName, "/sys/devices/system/cpu/cpu%d/cpufreq/%s", i, files[j] );
				FILE * fp = fopen( fileName, "r" );
				if ( fp == NULL )
				{
					continue;
				}
				char buffer[1024];
				if ( fgets( buffer, sizeof( buffer ), fp ) == NULL )
				{
					fclose( fp );
					continue;
				}
				for ( int index = 0; buffer[index] != '\0'; )
				{
					const unsigned int frequency = atoi( buffer + index );
					maxFrequency = ( frequency > maxFrequency ) ? frequency : maxFrequency;
					while ( isspace( buffer[index] ) ) { index++; }
					while ( isdigit( buffer[index] ) ) { index++; }
				}
				fclose( fp );
				break;
			}
			if ( maxFrequency == 0 )
			{
				break;
			}

			if ( maxFrequency == bestFrequency )
			{
				mask |= ( 1 << i );
			}
			else if ( maxFrequency > bestFrequency )
			{
				mask = ( 1 << i );
				bestFrequency = maxFrequency;
			}
		}

		if ( mask == 0 )
		{
			return;
		}
	}

	// Set the thread affinity.
	pid_t pid = gettid();
	int syscallres = syscall( __NR_sched_setaffinity, pid, sizeof( mask ), &mask );
	if ( syscallres )
	{
		int err = errno;
		printf( "    Error sched_setaffinity(%d): thread=(%d) mask=0x%X err=%s(%d)\n", __NR_sched_setaffinity, pid, mask, strerror( err ), err );
	}
	else
	{
		printf( "    Thread %d affinity 0x%02X\n", pid, mask );
	}
#else
	UNUSED_PARM( mask );
#endif
}

static void ksThread_SetRealTimePriority( int priority )
{
#if defined( OS_WINDOWS )
	UNUSED_PARM( priority );
	HANDLE process = GetCurrentProcess();
	if( !SetPriorityClass( process, REALTIME_PRIORITY_CLASS ) )
	{
		char buffer[1024];
		DWORD error = GetLastError();
		FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), buffer, sizeof( buffer ), NULL );
		printf( "Failed to set process %p priority class: %s(%lu)\n", process, buffer, error );
	}
	else
	{
		printf( "Process %p priority class set to real-time.\n", process );
	}
	HANDLE thread = GetCurrentThread();
	if ( !SetThreadPriority( thread, THREAD_PRIORITY_TIME_CRITICAL ) )
	{
		char buffer[1024];
		DWORD error = GetLastError();
		FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), buffer, sizeof( buffer ), NULL );
		printf( "Failed to set thread %p priority: %s(%lu)\n", thread, buffer, error );
	}
	else
	{
		printf( "Thread %p priority set to critical.\n", thread );
	}
#elif defined( OS_LINUX ) || defined( OS_APPLE )
	struct sched_param sp;
	memset( &sp, 0, sizeof( struct sched_param ) );
	sp.sched_priority = priority;
	if ( pthread_setschedparam( pthread_self(), SCHED_FIFO, &sp ) == -1 )
	{
		printf( "Failed to change thread %llu priority.\n", (unsigned long long)pthread_self() );
	}
	else
	{
		printf( "Thread %llu set to SCHED_FIFO, priority=%d\n", (unsigned long long)pthread_self(), priority );
	}
#elif defined( OS_ANDROID )
	struct sched_attr
	{
		uint32_t size;
		uint32_t sched_policy;
		uint64_t sched_flags;
		int32_t  sched_nice;
		uint32_t sched_priority;
		uint64_t sched_runtime;
		uint64_t sched_deadline;
		uint64_t sched_period;
	} attr;

	memset( &attr, 0, sizeof( attr ) );
	attr.size = sizeof( attr );
	attr.sched_policy = SCHED_FIFO;
	attr.sched_flags = SCHED_FLAG_RESET_ON_FORK;
	attr.sched_nice = 0;				// (SCHED_OTHER, SCHED_BATCH)
	attr.sched_priority = priority;		// (SCHED_FIFO, SCHED_RR)
	attr.sched_runtime = 0;				// (SCHED_DEADLINE)
	attr.sched_deadline = 0;			// (SCHED_DEADLINE)
	attr.sched_period = 0;				// (SCHED_DEADLINE)

	unsigned int flags = 0;

	pid_t pid = gettid();
	int syscallres = syscall( __NR_sched_setattr, pid, &attr, flags );
	if ( syscallres )
	{
		int err = errno;
		printf( "    Error sched_setattr(%d): thread=%d err=%s(%d)\n", __NR_sched_setattr, pid, strerror( err ), err );
	}
	else
	{
		printf( "    Thread %d set to SCHED_FIFO, priority=%d\n", pid, priority );
	}
#else
	UNUSED_PARM( priority );
#endif
}

static THREAD_RETURN_TYPE ThreadFunctionInternal( void * data )
{
	ksThread * thread = (ksThread *)data;

	ksThread_SetName( thread->threadName );

	for ( ; ; )
	{
		ksMutex_Lock( &thread->workMutex, true );
		if ( ksSignal_Wait( &thread->workIsAvailable, 0 ) )
		{
			ksMutex_Unlock( &thread->workMutex );
		}
		else
		{
			ksSignal_Raise( &thread->workIsDone );
			ksMutex_Unlock( &thread->workMutex );
			ksSignal_Wait( &thread->workIsAvailable, SIGNAL_TIMEOUT_INFINITE );
		}
		if ( thread->terminate )
		{
			ksSignal_Raise( &thread->workIsDone );
			break;
		}
		thread->threadFunction( thread->threadData );
	}
	return THREAD_RETURN_VALUE;
}

static bool ksThread_Create( ksThread * thread, const char * threadName, ksThreadFunction threadFunction, void * threadData )
{
#if defined( OS_WINDOWS )
	strncpy_s( thread->threadName, sizeof( thread->threadName ), threadName, sizeof( thread->threadName ) );
#else
	strncpy( thread->threadName, threadName, sizeof( thread->threadName ) );
#endif
	thread->threadName[sizeof( thread->threadName ) - 1] = '\0';
	thread->threadFunction = threadFunction;
	thread->threadData = threadData;
	thread->stack = NULL;
	ksSignal_Create( &thread->workIsDone, false );
	ksSignal_Create( &thread->workIsAvailable, true );
	ksMutex_Create( &thread->workMutex );
	thread->terminate = false;

#if defined( OS_WINDOWS )
	const int stackSize = 512 * 1024;
	DWORD threadID;
	thread->handle = CreateThread( NULL, stackSize, (LPTHREAD_START_ROUTINE)ThreadFunctionInternal, thread, STACK_SIZE_PARAM_IS_A_RESERVATION, &threadID );
	if ( thread->handle == 0 )
	{
		return false;
	}
#elif defined( OS_HEXAGON )
	const int stackSize = 16 * 1024;
	thread->stack = malloc( stackSize + 128 );
	void * aligned = (void *)( ( (size_t)thread->stack + 127 ) & ~127 );
	qurt_thread_attr_t attr;
	qurt_thread_attr_init( &attr );
	qurt_thread_attr_set_name( &attr, (char *)threadName );
	qurt_thread_attr_set_stack_addr( &attr, aligned );
	qurt_thread_attr_set_stack_size( &attr, stackSize );
	qurt_thread_attr_set_priority( &attr, qurt_thread_get_priority( qurt_thread_get_id() ) );
	int ret = qurt_thread_create( &thread->handle, &attr, ThreadFunctionInternal, (void *)thread );
	if ( ret != 0 )
	{
		return false;
	}
#else
	const int stackSize = 512 * 1024;
	pthread_attr_t attr;
	pthread_attr_init( &attr );
	pthread_attr_setstacksize( &attr, stackSize );
	int ret = pthread_create( &thread->handle, &attr, ThreadFunctionInternal, thread );
	if ( ret != 0 )
	{
		return false;
	}
	pthread_attr_destroy( &attr );
#endif

	ksSignal_Wait( &thread->workIsDone, SIGNAL_TIMEOUT_INFINITE );
	return true;
}

static void ksThread_Destroy( ksThread * thread )
{
	ksMutex_Lock( &thread->workMutex, true );
	ksSignal_Clear( &thread->workIsDone );
	thread->terminate = true;
	ksSignal_Raise( &thread->workIsAvailable );
	ksMutex_Unlock( &thread->workMutex );
	ksSignal_Wait( &thread->workIsDone, SIGNAL_TIMEOUT_INFINITE );
	ksMutex_Destroy( &thread->workMutex );
	ksSignal_Destroy( &thread->workIsDone );
	ksSignal_Destroy( &thread->workIsAvailable );
#if defined( OS_WINDOWS )
	WaitForSingleObject( thread->handle, INFINITE );
	CloseHandle( thread->handle );
#elif defined( OS_HEXAGON )
	int status = 0;
	qurt_thread_join( thread->handle, &status );
	free( thread->stack );
#else
	pthread_join( thread->handle, NULL );
#endif
}

static void ksThread_Signal( ksThread * thread )
{
	ksMutex_Lock( &thread->workMutex, true );
	ksSignal_Clear( &thread->workIsDone );
	ksSignal_Raise( &thread->workIsAvailable );
	ksMutex_Unlock( &thread->workMutex );
}

static void ksThread_Join( ksThread * thread )
{
	ksSignal_Wait( &thread->workIsDone, SIGNAL_TIMEOUT_INFINITE );
}

static void ksThread_Submit( ksThread * thread, ksThreadFunction threadFunction, void * threadData )
{
	ksThread_Join( thread );
	thread->threadFunction = threadFunction;
	thread->threadData = threadData;
	ksThread_Signal( thread );
}

/*
================================================================================================================================

Worker thread pool.

ksThreadPool

static bool ksThreadPool_Create( ksThreadPool * pool, const int numWorkers );
static void ksThreadPool_Destroy( ksThreadPool * pool );
static void ksThreadPool_Submit( ksThreadPool * pool, ksThreadFunction threadFunction, void * threadData );
static void ksThreadPool_Join( ksThreadPool * pool );

================================================================================================================================
*/

#define MAX_WORKERS		8

typedef struct
{
	ksThread	threads[MAX_WORKERS];
	int			threadCount;
} ksThreadPool;

static void PoolThreadStartFuntion( void * data )
{
	UNUSED_PARM( data );

	ksThread_SetAffinity( THREAD_AFFINITY_BIG_CORES );
	ksThread_SetRealTimePriority( 1 );
}

static void ksThreadPool_Create( ksThreadPool * pool, const int numWorkers )
{
	pool->threadCount = ( numWorkers <= MAX_WORKERS ) ? numWorkers : MAX_WORKERS;
#if defined( OS_HEXAGON )
	qurt_sysenv_max_hthreads_t num_threads;
	if ( qurt_sysenv_get_max_hw_threads( &num_threads ) == QURT_EOK )
	{
		pool->threadCount = num_threads.max_hthreads;
	}
#endif

	for ( int i = 0; i < pool->threadCount; i++ )
	{
		ksThread_Create( &pool->threads[i], "worker", PoolThreadStartFuntion, NULL );
		ksThread_Signal( &pool->threads[i] );
		ksThread_Join( &pool->threads[i] );
	}
}

static void ksThreadPool_Destroy( ksThreadPool * pool )
{
	for ( int i = 0; i < pool->threadCount; i++ )
	{
		ksThread_Destroy( &pool->threads[i] );
	}
}

static void ksThreadPool_Submit( ksThreadPool * pool, ksThreadFunction threadFunction, void * threadData )
{
	for ( int i = 0; i < pool->threadCount; i++ )
	{
		ksThread_Submit( &pool->threads[i], threadFunction, threadData );
	}
}

static void ksThreadPool_Join( ksThreadPool * pool )
{
	for ( int i = 0; i < pool->threadCount; i++ )
	{
		ksThread_Join( &pool->threads[i] );
	}
}

#endif // !KSTHREADING_H
