/*
 * Copyright (c) 2007 Apple Inc.  All Rights Reserved.
 * 
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/***********************************************************************
* objc-os.h
* OS portability layer.
**********************************************************************/

#ifndef _OBJC_OS_H
#define _OBJC_OS_H

#include <TargetConditionals.h>
#include "objc-config.h"

#ifdef __LP64__
#   define WORD_SHIFT 3UL
#   define WORD_MASK 7UL
#   define WORD_BITS 64
#else
#   define WORD_SHIFT 2UL
#   define WORD_MASK 3UL
#   define WORD_BITS 32
#endif

static inline uint32_t word_align(uint32_t x) {
    return (x + WORD_MASK) & ~WORD_MASK;
}
static inline size_t word_align(size_t x) {
    return (x + WORD_MASK) & ~WORD_MASK;
}


// Mix-in for classes that must not be copied.
class nocopy_t {
  private:
    nocopy_t(const nocopy_t&) = delete;
    const nocopy_t& operator=(const nocopy_t&) = delete;
  protected:
    constexpr nocopy_t() = default;
    ~nocopy_t() = default;
};


#if TARGET_OS_MAC

#   define OS_UNFAIR_LOCK_INLINE 1

#   ifndef __STDC_LIMIT_MACROS
#       define __STDC_LIMIT_MACROS
#   endif

#   include <stdio.h>
#   include <stdlib.h>
#   include <stdint.h>
#   include <stdarg.h>
#   include <string.h>
#   include <ctype.h>
#   include <errno.h>
#   include <dlfcn.h>
#   include <fcntl.h>
#   include <assert.h>
#   include <limits.h>
#   include <syslog.h>
#   include <unistd.h>
#   include <pthread.h>
#   include <crt_externs.h>
#   undef check
#   include <Availability.h>
#   include <TargetConditionals.h>
#   include <sys/mman.h>
#   include <sys/time.h>
#   include <sys/stat.h>
#   include <sys/param.h>
#   include <sys/reason.h>
#   include <mach/mach.h>
#   include <mach/vm_param.h>
#   include <mach/mach_time.h>
#   include <mach-o/dyld.h>
#   include <mach-o/ldsyms.h>
#   include <mach-o/loader.h>
#   include <mach-o/getsect.h>
#   include <mach-o/dyld_priv.h>
#   include <malloc/malloc.h>
#   include <os/lock_private.h>
#   include <libkern/OSAtomic.h>
#   include <libkern/OSCacheControl.h>
#   include <System/pthread_machdep.h>
#   include "objc-probes.h"  // generated dtrace probe definitions.

// Some libc functions call objc_msgSend() 
// so we can't use them without deadlocks.
void syslog(int, const char *, ...) UNAVAILABLE_ATTRIBUTE;
void vsyslog(int, const char *, va_list) UNAVAILABLE_ATTRIBUTE;


#define ALWAYS_INLINE inline __attribute__((always_inline))
#define NEVER_INLINE inline __attribute__((noinline))

#define fastpath(x) (__builtin_expect(bool(x), 1))
#define slowpath(x) (__builtin_expect(bool(x), 0))


static ALWAYS_INLINE uintptr_t 
addc(uintptr_t lhs, uintptr_t rhs, uintptr_t carryin, uintptr_t *carryout)
{
    return __builtin_addcl(lhs, rhs, carryin, carryout);
}

static ALWAYS_INLINE uintptr_t 
subc(uintptr_t lhs, uintptr_t rhs, uintptr_t carryin, uintptr_t *carryout)
{
    return __builtin_subcl(lhs, rhs, carryin, carryout);
}

#if __arm64__ && !__arm64e__

static ALWAYS_INLINE
uintptr_t
LoadExclusive(uintptr_t *src)
{
    return __builtin_arm_ldrex(src);
}

static ALWAYS_INLINE
bool
StoreExclusive(uintptr_t *dst, uintptr_t oldvalue __unused, uintptr_t value)
{
    return !__builtin_arm_strex(value, dst);
}


static ALWAYS_INLINE
bool
StoreReleaseExclusive(uintptr_t *dst, uintptr_t oldvalue __unused, uintptr_t value)
{
    return !__builtin_arm_stlex(value, dst);
}

static ALWAYS_INLINE
void
ClearExclusive(uintptr_t *dst __unused)
{
    __builtin_arm_clrex();
}

#else

static ALWAYS_INLINE
uintptr_t
LoadExclusive(uintptr_t *src)
{
    return __c11_atomic_load((_Atomic(uintptr_t) *)src, __ATOMIC_RELAXED);
}

static ALWAYS_INLINE
bool
StoreExclusive(uintptr_t *dst, uintptr_t oldvalue, uintptr_t value)
{
    return __c11_atomic_compare_exchange_weak((_Atomic(uintptr_t) *)dst, &oldvalue, value, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
}


static ALWAYS_INLINE
bool
StoreReleaseExclusive(uintptr_t *dst, uintptr_t oldvalue, uintptr_t value)
{
    return __c11_atomic_compare_exchange_weak((_Atomic(uintptr_t) *)dst, &oldvalue, value, __ATOMIC_RELEASE, __ATOMIC_RELAXED);
}

static ALWAYS_INLINE
void
ClearExclusive(uintptr_t *dst __unused)
{
}

#endif


#if !TARGET_OS_IPHONE
#   include <CrashReporterClient.h>
#else
    // CrashReporterClient not yet available on iOS
    __BEGIN_DECLS
    extern const char *CRSetCrashLogMessage(const char *msg);
    extern const char *CRGetCrashLogMessage(void);
    __END_DECLS
#endif

#   if __cplusplus
#       include <vector>
#       include <algorithm>
#       include <functional>
        using namespace std;
#   endif

#   define PRIVATE_EXTERN __attribute__((visibility("hidden")))
#   undef __private_extern__
#   define __private_extern__ use_PRIVATE_EXTERN_instead
#   undef private_extern
#   define private_extern use_PRIVATE_EXTERN_instead

/* Use this for functions that are intended to be breakpoint hooks.
   If you do not, the compiler may optimize them away.
   BREAKPOINT_FUNCTION( void stop_on_error(void) ); */
#   define BREAKPOINT_FUNCTION(prototype)                             \
    OBJC_EXTERN __attribute__((noinline, used, visibility("hidden"))) \
    prototype { asm(""); }
#else
#   error unknown OS
#endif


#include <objc/objc.h>
#include <objc/objc-api.h>

extern void _objc_fatal(const char *fmt, ...) 
    __attribute__((noreturn, format (printf, 1, 2)));
extern void _objc_fatal_with_reason(uint64_t reason, uint64_t flags, 
                                    const char *fmt, ...) 
    __attribute__((noreturn, format (printf, 3, 4)));

#define INIT_ONCE_PTR(var, create, delete)                              \
    do {                                                                \
        if (var) break;                                                 \
        typeof(var) v = create;                                         \
        while (!var) {                                                  \
            if (OSAtomicCompareAndSwapPtrBarrier(0, (void*)v, (void**)&var)){ \
                goto done;                                              \
            }                                                           \
        }                                                               \
        delete;                                                         \
    done:;                                                              \
    } while (0)

#define INIT_ONCE_32(var, create, delete)                               \
    do {                                                                \
        if (var) break;                                                 \
        typeof(var) v = create;                                         \
        while (!var) {                                                  \
            if (OSAtomicCompareAndSwap32Barrier(0, v, (volatile int32_t *)&var)) { \
                goto done;                                              \
            }                                                           \
        }                                                               \
        delete;                                                         \
    done:;                                                              \
    } while (0)


// Thread keys reserved by libc for our use.
#if defined(__PTK_FRAMEWORK_OBJC_KEY0)
#   define SUPPORT_DIRECT_THREAD_KEYS 1
#   define TLS_DIRECT_KEY        ((tls_key_t)__PTK_FRAMEWORK_OBJC_KEY0)
#   define SYNC_DATA_DIRECT_KEY  ((tls_key_t)__PTK_FRAMEWORK_OBJC_KEY1)
#   define SYNC_COUNT_DIRECT_KEY ((tls_key_t)__PTK_FRAMEWORK_OBJC_KEY2)
#   define AUTORELEASE_POOL_KEY  ((tls_key_t)__PTK_FRAMEWORK_OBJC_KEY3)
# if SUPPORT_RETURN_AUTORELEASE
#   define RETURN_DISPOSITION_KEY ((tls_key_t)__PTK_FRAMEWORK_OBJC_KEY4)
# endif
#else
#   define SUPPORT_DIRECT_THREAD_KEYS 0
#endif


#if TARGET_OS_MAC


// OS headers
#include <mach-o/loader.h>
#ifndef __LP64__
#   define SEGMENT_CMD LC_SEGMENT
#else
#   define SEGMENT_CMD LC_SEGMENT_64
#endif

#ifndef VM_MEMORY_OBJC_DISPATCHERS
#   define VM_MEMORY_OBJC_DISPATCHERS 0
#endif


// Compiler compatibility

// OS compatibility

static inline uint64_t nanoseconds() {
    return clock_gettime_nsec_np(CLOCK_MONOTONIC_RAW);
}

// Internal data types

typedef pthread_t objc_thread_t;

static __inline int thread_equal(objc_thread_t t1, objc_thread_t t2) { 
    return pthread_equal(t1, t2); 
}
static __inline objc_thread_t thread_self(void) { 
    return pthread_self(); 
}


typedef pthread_key_t tls_key_t;

static inline tls_key_t tls_create(void (*dtor)(void*)) { 
    tls_key_t k;
    pthread_key_create(&k, dtor); 
    return k;
}
static inline void *tls_get(tls_key_t k) { 
    return pthread_getspecific(k); 
}
static inline void tls_set(tls_key_t k, void *value) { 
    pthread_setspecific(k, value); 
}

#if SUPPORT_DIRECT_THREAD_KEYS

#if DEBUG
static bool is_valid_direct_key(tls_key_t k) {
    return (   k == SYNC_DATA_DIRECT_KEY
            || k == SYNC_COUNT_DIRECT_KEY
            || k == AUTORELEASE_POOL_KEY
#   if SUPPORT_RETURN_AUTORELEASE
            || k == RETURN_DISPOSITION_KEY
#   endif
               );
}
#endif

static inline void *tls_get_direct(tls_key_t k) 
{ 
    assert(is_valid_direct_key(k));

    if (_pthread_has_direct_tsd()) {
        return _pthread_getspecific_direct(k);
    } else {
        return pthread_getspecific(k);
    }
}
static inline void tls_set_direct(tls_key_t k, void *value) 
{ 
    assert(is_valid_direct_key(k));

    if (_pthread_has_direct_tsd()) {
        _pthread_setspecific_direct(k, value);
    } else {
        pthread_setspecific(k, value);
    }
}

// SUPPORT_DIRECT_THREAD_KEYS
#endif


static inline pthread_t pthread_self_direct()
{
    return (pthread_t)
        _pthread_getspecific_direct(_PTHREAD_TSD_SLOT_PTHREAD_SELF);
}

static inline mach_port_t mach_thread_self_direct() 
{
    return (mach_port_t)(uintptr_t)
        _pthread_getspecific_direct(_PTHREAD_TSD_SLOT_MACH_THREAD_SELF);
}


template <bool Debug> class mutex_tt;
template <bool Debug> class monitor_tt;
template <bool Debug> class recursive_mutex_tt;

#if DEBUG
#   define LOCKDEBUG 1
#else
#   define LOCKDEBUG 0
#endif

using spinlock_t = mutex_tt<LOCKDEBUG>;
using mutex_t = mutex_tt<LOCKDEBUG>;
using monitor_t = monitor_tt<LOCKDEBUG>;
using recursive_mutex_t = recursive_mutex_tt<LOCKDEBUG>;

// Use fork_unsafe_lock to get a lock that isn't 
// acquired and released around fork().
// All fork-safe locks are checked in debug builds.
struct fork_unsafe_lock_t {
    constexpr fork_unsafe_lock_t() = default;
};
extern const fork_unsafe_lock_t fork_unsafe_lock;

#include "objc-lockdebug.h"

template <bool Debug>
class mutex_tt : nocopy_t {
    os_unfair_lock mLock;
 public:
    constexpr mutex_tt() : mLock(OS_UNFAIR_LOCK_INIT) {
        lockdebug_remember_mutex(this);
    }

    constexpr mutex_tt(const fork_unsafe_lock_t unsafe) : mLock(OS_UNFAIR_LOCK_INIT) { }

    void lock() {
        lockdebug_mutex_lock(this);

        os_unfair_lock_lock_with_options_inline
            (&mLock, OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION);
    }

    void unlock() {
        lockdebug_mutex_unlock(this);

        os_unfair_lock_unlock_inline(&mLock);
    }

    void forceReset() {
        lockdebug_mutex_unlock(this);

        bzero(&mLock, sizeof(mLock));
        mLock = os_unfair_lock OS_UNFAIR_LOCK_INIT;
    }

    void assertLocked() {
        lockdebug_mutex_assert_locked(this);
    }

    void assertUnlocked() {
        lockdebug_mutex_assert_unlocked(this);
    }


    // Address-ordered lock discipline for a pair of locks.

    static void lockTwo(mutex_tt *lock1, mutex_tt *lock2) {
        if (lock1 < lock2) {
            lock1->lock();
            lock2->lock();
        } else {
            lock2->lock();
            if (lock2 != lock1) lock1->lock(); 
        }
    }

    static void unlockTwo(mutex_tt *lock1, mutex_tt *lock2) {
        lock1->unlock();
        if (lock2 != lock1) lock2->unlock();
    }

    // Scoped lock and unlock
    class locker : nocopy_t {
        mutex_tt& lock;
    public:
        locker(mutex_tt& newLock) 
            : lock(newLock) { lock.lock(); }
        ~locker() { lock.unlock(); }
    };

    // Either scoped lock and unlock, or NOP.
    class conditional_locker : nocopy_t {
        mutex_tt& lock;
        bool didLock;
    public:
        conditional_locker(mutex_tt& newLock, bool shouldLock)
            : lock(newLock), didLock(shouldLock)
        {
            if (shouldLock) lock.lock();
        }
        ~conditional_locker() { if (didLock) lock.unlock(); }
    };
};

using mutex_locker_t = mutex_tt<LOCKDEBUG>::locker;
using conditional_mutex_locker_t = mutex_tt<LOCKDEBUG>::conditional_locker;


template <bool Debug>
class recursive_mutex_tt : nocopy_t {
    os_unfair_recursive_lock mLock;

  public:
    constexpr recursive_mutex_tt() : mLock(OS_UNFAIR_RECURSIVE_LOCK_INIT) {
        lockdebug_remember_recursive_mutex(this);
    }

    constexpr recursive_mutex_tt(const fork_unsafe_lock_t unsafe)
        : mLock(OS_UNFAIR_RECURSIVE_LOCK_INIT)
    { }

    void lock()
    {
        lockdebug_recursive_mutex_lock(this);
        os_unfair_recursive_lock_lock(&mLock);
    }

    void unlock()
    {
        lockdebug_recursive_mutex_unlock(this);

        os_unfair_recursive_lock_unlock(&mLock);
    }

    void forceReset()
    {
        lockdebug_recursive_mutex_unlock(this);

        bzero(&mLock, sizeof(mLock));
        mLock = os_unfair_recursive_lock OS_UNFAIR_RECURSIVE_LOCK_INIT;
    }

    bool tryUnlock()
    {
        if (os_unfair_recursive_lock_tryunlock4objc(&mLock)) {
            lockdebug_recursive_mutex_unlock(this);
            return true;
        }
        return false;
    }

    void assertLocked() {
        lockdebug_recursive_mutex_assert_locked(this);
    }

    void assertUnlocked() {
        lockdebug_recursive_mutex_assert_unlocked(this);
    }
};


template <bool Debug>
class monitor_tt {
    pthread_mutex_t mutex;
    pthread_cond_t cond;

  public:
    constexpr monitor_tt()
        : mutex(PTHREAD_MUTEX_INITIALIZER), cond(PTHREAD_COND_INITIALIZER)
    {
        lockdebug_remember_monitor(this);
    }

    monitor_tt(const fork_unsafe_lock_t unsafe) 
        : mutex(PTHREAD_MUTEX_INITIALIZER), cond(PTHREAD_COND_INITIALIZER)
    { }

    void enter() 
    {
        lockdebug_monitor_enter(this);

        int err = pthread_mutex_lock(&mutex);
        if (err) _objc_fatal("pthread_mutex_lock failed (%d)", err);
    }

    void leave() 
    {
        lockdebug_monitor_leave(this);

        int err = pthread_mutex_unlock(&mutex);
        if (err) _objc_fatal("pthread_mutex_unlock failed (%d)", err);
    }

    void wait() 
    {
        lockdebug_monitor_wait(this);

        int err = pthread_cond_wait(&cond, &mutex);
        if (err) _objc_fatal("pthread_cond_wait failed (%d)", err);
    }

    void notify() 
    {
        int err = pthread_cond_signal(&cond);
        if (err) _objc_fatal("pthread_cond_signal failed (%d)", err);        
    }

    void notifyAll() 
    {
        int err = pthread_cond_broadcast(&cond);
        if (err) _objc_fatal("pthread_cond_broadcast failed (%d)", err);        
    }

    void forceReset()
    {
        lockdebug_monitor_leave(this);
        
        bzero(&mutex, sizeof(mutex));
        bzero(&cond, sizeof(cond));
        mutex = pthread_mutex_t PTHREAD_MUTEX_INITIALIZER;
        cond = pthread_cond_t PTHREAD_COND_INITIALIZER;
    }

    void assertLocked()
    {
        lockdebug_monitor_assert_locked(this);
    }

    void assertUnlocked()
    {
        lockdebug_monitor_assert_unlocked(this);
    }
};


// semaphore_create formatted for INIT_ONCE use
static inline semaphore_t create_semaphore(void)
{
    semaphore_t sem;
    kern_return_t k;
    k = semaphore_create(mach_task_self(), &sem, SYNC_POLICY_FIFO, 0);
    if (k) _objc_fatal("semaphore_create failed (0x%x)", k);
    return sem;
}


#ifndef __LP64__
typedef struct mach_header headerType;
typedef struct segment_command segmentType;
typedef struct section sectionType;
#else
typedef struct mach_header_64 headerType;
typedef struct segment_command_64 segmentType;
typedef struct section_64 sectionType;
#endif
#define headerIsBundle(hi) (hi->mhdr()->filetype == MH_BUNDLE)
#define libobjc_header ((headerType *)&_mh_dylib_header)

// Prototypes

/* Secure /tmp usage */
extern int secure_open(const char *filename, int flags, uid_t euid);


#else


#error unknown OS


#endif


static inline void *
memdup(const void *mem, size_t len)
{
    void *dup = malloc(len);
    memcpy(dup, mem, len);
    return dup;
}

// strdup that doesn't copy read-only memory
static inline char *
strdupIfMutable(const char *str)
{
    size_t size = strlen(str) + 1;
    if (_dyld_is_memory_immutable(str, size)) {
        return (char *)str;
    } else {
        return (char *)memdup(str, size);
    }
}

// free strdupIfMutable() result
static inline void
freeIfMutable(char *str)
{
    size_t size = strlen(str) + 1;
    if (_dyld_is_memory_immutable(str, size)) {
        // nothing
    } else {
        free(str);
    }
}

// nil-checking unsigned strdup
static inline uint8_t *
ustrdupMaybeNil(const uint8_t *str)
{
    if (!str) return nil;
    return (uint8_t *)strdupIfMutable((char *)str);
}

// OS version checking:
//
// sdkVersion()
// DYLD_OS_VERSION(mac, ios, tv, watch, bridge)
// sdkIsOlderThan(mac, ios, tv, watch, bridge)
// sdkIsAtLeast(mac, ios, tv, watch, bridge)
// 
// This version order matches OBJC_AVAILABLE.

#if TARGET_OS_OSX
#   define DYLD_OS_VERSION(x, i, t, w, b) DYLD_MACOSX_VERSION_##x
#   define sdkVersion() dyld_get_program_sdk_version()

#elif TARGET_OS_IOS
#   define DYLD_OS_VERSION(x, i, t, w, b) DYLD_IOS_VERSION_##i
#   define sdkVersion() dyld_get_program_sdk_version()

#elif TARGET_OS_TV
    // dyld does not currently have distinct constants for tvOS
#   define DYLD_OS_VERSION(x, i, t, w, b) DYLD_IOS_VERSION_##t
#   define sdkVersion() dyld_get_program_sdk_version()

#elif TARGET_OS_BRIDGE
#   if TARGET_OS_WATCH
#       error bridgeOS 1.0 not supported
#   endif
    // fixme don't need bridgeOS versioning yet
#   define DYLD_OS_VERSION(x, i, t, w, b) DYLD_IOS_VERSION_##t
#   define sdkVersion() dyld_get_program_sdk_bridge_os_version()

#elif TARGET_OS_WATCH
#   define DYLD_OS_VERSION(x, i, t, w, b) DYLD_WATCHOS_VERSION_##w
    // watchOS has its own API for compatibility reasons
#   define sdkVersion() dyld_get_program_sdk_watch_os_version()

#else
#   error unknown OS
#endif


#define sdkIsOlderThan(x, i, t, w, b)                           \
            (sdkVersion() < DYLD_OS_VERSION(x, i, t, w, b))
#define sdkIsAtLeast(x, i, t, w, b)                             \
            (sdkVersion() >= DYLD_OS_VERSION(x, i, t, w, b))

// Allow bare 0 to be used in DYLD_OS_VERSION() and sdkIsOlderThan()
#define DYLD_MACOSX_VERSION_0 0
#define DYLD_IOS_VERSION_0 0
#define DYLD_TVOS_VERSION_0 0
#define DYLD_WATCHOS_VERSION_0 0
#define DYLD_BRIDGEOS_VERSION_0 0

// Pretty-print a DYLD_*_VERSION_* constant.
#define SDK_FORMAT "%hu.%hhu.%hhu"
#define FORMAT_SDK(v) \
    (unsigned short)(((uint32_t)(v))>>16),  \
    (unsigned  char)(((uint32_t)(v))>>8),   \
    (unsigned  char)(((uint32_t)(v))>>0)

// fork() safety requires careful tracking of all locks.
// Our custom lock types check this in debug builds.
// Disallow direct use of all other lock types.
typedef __darwin_pthread_mutex_t pthread_mutex_t UNAVAILABLE_ATTRIBUTE;
typedef __darwin_pthread_rwlock_t pthread_rwlock_t UNAVAILABLE_ATTRIBUTE;
typedef int32_t OSSpinLock UNAVAILABLE_ATTRIBUTE;
typedef struct os_unfair_lock_s os_unfair_lock UNAVAILABLE_ATTRIBUTE;


#endif
