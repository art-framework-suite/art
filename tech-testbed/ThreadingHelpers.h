#ifndef TBBProcessingDemo_ThreadingHelpers_h
#define TBBProcessingDemo_ThreadingHelpers_h

//NOTE: Taken from libdispatch shims/atomics.h
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)
#define hardware_pause()      asm("")
#endif
#if defined(__x86_64__) || defined(__i386__)
#undef hardware_pause
#define hardware_pause() asm("pause")
#endif

#endif
