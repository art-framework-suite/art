/*#include "sqliteInt.h"*/

// Ignore warnings in order to preserve the relationship between this
// file and the VFS template from which it was taken.
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"

extern "C" {
#include <sqlite3.h>
}
#define SQLITE_OPEN_WAL              0x00080000  /* VFS only */
#define SQLITE_FCNTL_SIZE_HINT        5
#define SQLITE_FCNTL_CHUNK_SIZE       6
#define SQLITE_FCNTL_SYNC_OMITTED     8

#define SQLITE_FCNTL_DB_UNCHANGED 0xca093fa0
#define SQLITE_DEFAULT_SECTOR_SIZE 512

#define UNUSED_PARAMETER(x) (void)(x)
#define UNUSED_PARAMETER2(x,y) UNUSED_PARAMETER(x),UNUSED_PARAMETER(y)

#define ArraySize(X) ((int)(sizeof(X)/sizeof(X[0])))

typedef sqlite_int64 i64;

#define _LARGE_FILE       1
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE 1

/*
 * Save db to root file on close.
*/

#ifndef TKEYVFS_NO_ROOT
#include "TFile.h"
#include "TKey.h"
#endif // TKEYVFS_NO_ROOT

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
extern "C" {
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
}

/*
 *  Debug tracing.
*/

#define TKEYVFS_TRACE 0

/*
 * Externally provided ROOT file, must be open.
*/

#ifndef TKEYVFS_NO_ROOT
static TFile * gRootFile;
#endif // TKEYVFS_NO_ROOT

/*
 * Memory Page size.
*/

#define MEMPAGE 2048

/*
** Maximum supported path-length.
*/
#define MAX_PATHNAME 512

#define SQLITE_TEMP_FILE_PREFIX "etilqs_"

/*
** The unixFile structure is subclass of sqlite3_file specific to the unix
** VFS implementations.
*/
struct unixFile {
  sqlite3_io_methods const * pMethod; /* Always the first entry */
#ifndef TKEYVFS_NO_ROOT
  TFile * rootFile;                   /* The ROOT file the db is stored in */
  int saveToRootFile;                 /* On close, save db to root file */
#endif // TKEYVFS_NO_ROOT
  char * pBuf;                        /* File contents */
  i64 bufAllocated;                   /* File buffer size in bytes */
  i64 fileSize;                       /* Current file size in bytes */
  int eFileLock;                      /* The type of lock held on this fd */
  int lastErrno;                      /* The unix errno from last I/O error */
  const char * zPath;                 /* Name of the file */
  int szChunk;                        /* Configured by FCNTL_CHUNK_SIZE */
  /* The next group of variables are used to track whether or not the
  ** transaction counter in bytes 24-27 of database files are updated
  ** whenever any part of the database changes.  An assertion fault will
  ** occur if a file is updated without also updating the transaction
  ** counter.  This test is made to avoid new problems similar to the
  ** one described by ticket #3584.
  */
  unsigned char transCntrChng;   /* True if the transaction counter changed */
  unsigned char dbUpdate;        /* True if any part of database file changed */
  unsigned char inNormalWrite;   /* True if in a normal write operation */
};
typedef struct unixFile unixFile;

/*
** Define various macros that are missing from some systems.
** Must come after the include of the system headers.
*/
#ifndef O_LARGEFILE
# define O_LARGEFILE 0
#endif
#ifdef SQLITE_DISABLE_LFS
# undef O_LARGEFILE
# define O_LARGEFILE 0
#endif
#ifndef O_NOFOLLOW
# define O_NOFOLLOW 0
#endif
#ifndef O_BINARY
# define O_BINARY 0
#endif

#if 0 // F13
typedef void (*sqlite3_syscall_ptr)(void);
#endif // 0

/* Function Directory */
static int sqlite3CantopenError(int lineno);
static int sqlite3Strlen30(const char * z);
static const sqlite3_io_methods * nolockIoFinderImpl(const char * z, unixFile * p);
static int unixLogErrorAtLine(int errcode, const char * zFunc, const char * zPath, int iLine);
static int robust_open(const char * z, int f, int m);
static void robust_close(unixFile * pFile, int h, int lineno);
static int unixGetTempname(int nBuf, char * zBuf);
static int fcntlSizeHint(unixFile * pFile, i64 nByte);
static int seekAndRead(unixFile * id, sqlite3_int64 offset, void * pBuf, int cnt);
static int seekAndWrite(unixFile * id, i64 offset, const void * pBuf, int cnt);
/* IoMethods calls */
static int nolockClose(sqlite3_file * id);
static int unixRead(sqlite3_file * id, void * pBuf, int amt, sqlite3_int64 offset);
static int unixWrite(sqlite3_file * id, const void * pBuf, int amt, sqlite3_int64 offset);
static int unixTruncate(sqlite3_file * id, i64 nByte);
static int unixSync(sqlite3_file * id, int flags);
static int unixFileSize(sqlite3_file * id, i64 * pSize);
static int nolockLock(sqlite3_file * NotUsed, int NotUsed2);
static int nolockUnlock(sqlite3_file * NotUsed, int NotUsed2);
static int nolockCheckReservedLock(sqlite3_file * NotUsed, int * pResOut);
static int unixFileControl(sqlite3_file * id, int op, void * pArg);
static int unixSectorSize(sqlite3_file * NotUsed);
static int unixDeviceCharacteristics(sqlite3_file * NotUsed);
/* VFS calls */
static int unixOpen(sqlite3_vfs * pVfs, const char * zPath, sqlite3_file * pFile, int flags, int * pOutFlags);
static int unixDelete(sqlite3_vfs * NotUsed, const char * zPath, int dirSync);
static int unixAccess(sqlite3_vfs * NotUsed, const char * zPath, int flags, int * pResOut);
static int unixFullPathname(sqlite3_vfs * pVfs, const char * zPath, int nOut, char * zOut);
static void * unixDlOpen(sqlite3_vfs * NotUsed, const char * zFilename);
static void unixDlError(sqlite3_vfs * NotUsed, int nBuf, char * zBufOut);
static void (*unixDlSym(sqlite3_vfs * NotUsed, void * p, const char * zSym))(void);
static void unixDlClose(sqlite3_vfs * NotUsed, void * pHandle);
static int unixRandomness(sqlite3_vfs * NotUsed, int nBuf, char * zBuf);
static int unixSleep(sqlite3_vfs * NotUsed, int microseconds);
static int unixCurrentTime(sqlite3_vfs * NotUsed, double * prNow);
static int unixGetLastError(sqlite3_vfs * NotUsed, int NotUsed2, char * NotUsed3);
static int unixCurrentTimeInt64(sqlite3_vfs * NotUsed, sqlite3_int64 * piNow);
static int unixSetSystemCall(sqlite3_vfs * pNotUsed, const char * zName, sqlite3_syscall_ptr pNewFunc);
static sqlite3_syscall_ptr unixGetSystemCall(sqlite3_vfs * pNotUsed, const char * zName);
static const char * unixNextSystemCall(sqlite3_vfs * p, const char * zName);
/**/
static int sqlite3CantopenError(int lineno)
{
  fprintf(stderr,
          "tkeyvfs.c: cannot open file at line %d of [%.10s]",
          lineno, 20 + sqlite3_sourceid());
  return SQLITE_CANTOPEN;
}
#define SQLITE_CANTOPEN_BKPT sqlite3CantopenError(__LINE__)

/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
**
** The value returned will never be negative.  Nor will it ever be greater
** than the actual length of the string.  For very long strings (greater
** than 1GiB) the value returned might be less than the true string length.
*/
static int sqlite3Strlen30(const char * z)
{
  const char * z2 = z;
  if (z == 0) {
    return 0;
  }
  while (*z2) {
    z2++;
  }
  return 0x3fffffff & (int)(z2 - z);
}

/*
** Many system calls are accessed through pointer-to-functions so that
** they may be overridden at runtime to facilitate fault injection during
** testing and sandboxing.  The following array holds the names and pointers
** to all overrideable system calls.
*/
static struct unix_syscall {
  const char * zName;           /* Name of the sytem call */
  sqlite3_syscall_ptr pCurrent; /* Current value of the system call */
  sqlite3_syscall_ptr pDefault; /* Default value */
} aSyscall[] = {
  { "open", (sqlite3_syscall_ptr)open,       0  },
#define osOpen      ((int(*)(const char*,int,...))aSyscall[0].pCurrent)

  { "close", (sqlite3_syscall_ptr)close,      0  },
#define osClose     ((int(*)(int))aSyscall[1].pCurrent)

  { "access", (sqlite3_syscall_ptr)access,     0  },
#define osAccess    ((int(*)(const char*,int))aSyscall[2].pCurrent)

  { "getcwd", (sqlite3_syscall_ptr)getcwd,     0  },
#define osGetcwd    ((char*(*)(char*,size_t))aSyscall[3].pCurrent)

  { "stat", (sqlite3_syscall_ptr)stat,       0  },
#define osStat      ((int(*)(const char*,struct stat*))aSyscall[4].pCurrent)

  /*
  ** The DJGPP compiler environment looks mostly like Unix, but it
  ** lacks the fcntl() system call.  So redefine fcntl() to be something
  ** that always succeeds.  This means that locking does not occur under
  ** DJGPP.  But it is DOS - what did you expect?
  */
#ifdef __DJGPP__
  { "fstat",        0,                 0  },
#define osFstat(a,b,c)    0
#else
  { "fstat", (sqlite3_syscall_ptr)fstat,      0  },
#define osFstat     ((int(*)(int,struct stat*))aSyscall[5].pCurrent)
#endif

  { "ftruncate", (sqlite3_syscall_ptr)ftruncate,  0  },
#define osFtruncate ((int(*)(int,off_t))aSyscall[6].pCurrent)

  { "fcntl", (sqlite3_syscall_ptr)fcntl,      0  },
#define osFcntl     ((int(*)(int,int,...))aSyscall[7].pCurrent)

  { "read", (sqlite3_syscall_ptr)read,       0  },
#define osRead      ((ssize_t(*)(int,void*,size_t))aSyscall[8].pCurrent)

#if defined(USE_PREAD)
  { "pread", (sqlite3_syscall_ptr)pread,      0  },
#else
  { "pread", (sqlite3_syscall_ptr)0,          0  },
#endif
#define osPread     ((ssize_t(*)(int,void*,size_t,off_t))aSyscall[9].pCurrent)

#if defined(USE_PREAD64)
  { "pread64", (sqlite3_syscall_ptr)pread64,    0  },
#else
  { "pread64", (sqlite3_syscall_ptr)0,          0  },
#endif
#define osPread64   ((ssize_t(*)(int,void*,size_t,off_t))aSyscall[10].pCurrent)

  { "write", (sqlite3_syscall_ptr)write,      0  },
#define osWrite     ((ssize_t(*)(int,const void*,size_t))aSyscall[11].pCurrent)

#if defined(USE_PREAD)
  { "pwrite", (sqlite3_syscall_ptr)pwrite,     0  },
#else
  { "pwrite", (sqlite3_syscall_ptr)0,          0  },
#endif
#define osPwrite    ((ssize_t(*)(int,const void*,size_t,off_t))\
  aSyscall[12].pCurrent)

#if defined(USE_PREAD64)
  { "pwrite64", (sqlite3_syscall_ptr)pwrite64,   0  },
#else
  { "pwrite64", (sqlite3_syscall_ptr)0,          0  },
#endif
#define osPwrite64  ((ssize_t(*)(int,const void*,size_t,off_t))\
  aSyscall[13].pCurrent)

  { "fchmod", (sqlite3_syscall_ptr)0,          0  },
#define osFchmod    ((int(*)(int,mode_t))aSyscall[14].pCurrent)

#if defined(HAVE_POSIX_FALLOCATE) && HAVE_POSIX_FALLOCATE
  { "fallocate", (sqlite3_syscall_ptr)posix_fallocate,  0 },
#else
  { "fallocate", (sqlite3_syscall_ptr)0,                0 },
#endif
#define osFallocate ((int(*)(int,off_t,off_t))aSyscall[15].pCurrent)

}; /* End of the overrideable system calls */

static const sqlite3_io_methods nolockIoMethods = {
  1,                          /* iVersion */
  nolockClose,                /* xClose */
  unixRead,                   /* xRead */
  unixWrite,                  /* xWrite */
  unixTruncate,               /* xTruncate */
  unixSync,                   /* xSync */
  unixFileSize,               /* xFileSize */
  nolockLock,                 /* xLock */
  nolockUnlock,               /* xUnlock */
  nolockCheckReservedLock,    /* xCheckReservedLock */
  unixFileControl,            /* xFileControl */
  unixSectorSize,             /* xSectorSize */
  unixDeviceCharacteristics,  /* xDeviceCapabilities */
#if 0
  0,                          /* xShmMap */
  0,                          /* xShmLock */
  0,                          /* xShmBarrier */
  0                           /* xShmUnmap */
#endif // 0
};

static const sqlite3_io_methods * nolockIoFinderImpl(const char * z, unixFile * p)
{
  UNUSED_PARAMETER(z);
  UNUSED_PARAMETER(p);
  return &nolockIoMethods;
}

static const sqlite3_io_methods * (*const nolockIoFinder)(const char *, unixFile * p)
  = nolockIoFinderImpl;

typedef const sqlite3_io_methods * (*finder_type)(const char *, unixFile *);

/*
**
** This function - unixLogError_x(), is only ever called via the macro
** unixLogError().
**
** It is invoked after an error occurs in an OS function and errno has been
** set. It logs a message using sqlite3_log() containing the current value of
** errno and, if possible, the human-readable equivalent from strerror() or
** strerror_r().
**
** The first argument passed to the macro should be the error code that
** will be returned to SQLite (e.g. SQLITE_IOERR_DELETE, SQLITE_CANTOPEN).
** The two subsequent arguments should be the name of the OS function that
** failed (e.g. "unlink", "open") and the the associated file-system path,
** if any.
*/
#define unixLogError(a,b,c) unixLogErrorAtLine(a,b,c,__LINE__)
static int unixLogErrorAtLine(
  int errcode,       /* SQLite error code */
  const char * zFunc, /* Name of OS function that failed */
  const char * zPath, /* File path associated with error */
  int iLine          /* Source line number where error occurred */
)
{
  char * zErr; /* Message from strerror() or equivalent */
  int iErrno = errno; /* Saved syscall error number */
  zErr = strerror(iErrno);
  if (zPath == 0) {
    zPath = "";
  }
  fprintf(stderr,
          "tkeyvfs.c:%d: (%d) %s(%s) - %s",
          iLine, iErrno, zFunc, zPath, zErr
         );
  return errcode;
}

/*
** Retry open() calls that fail due to EINTR
*/
static int robust_open(const char * z, int f, int m)
{
  int rc;
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin robust_open ...\n");
#endif /* TKEYVFS_TRACE */
  do {
    rc = osOpen(z, f, m);
  }
  while (rc < 0 && errno == EINTR);
#if TKEYVFS_TRACE
  fprintf(stderr, "End   robust_open ...\n");
#endif /* TKEYVFS_TRACE */
  return rc;
}

/*
** Close a file descriptor.
**
** We assume that close() almost always works, since it is only in a
** very sick application or on a very sick platform that it might fail.
** If it does fail, simply leak the file descriptor, but do log the
** error.
**
** Note that it is not safe to retry close() after EINTR since the
** file descriptor might have already been reused by another thread.
** So we don't even try to recover from an EINTR.  Just log the error
** and move on.
*/
static void robust_close(unixFile * pFile, int h, int lineno)
{
  /**/
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin robust_close ...\n");
#endif /* TKEYVFS_TRACE */
  if (osClose(h)) {
    if (pFile) {
      unixLogErrorAtLine(SQLITE_IOERR_CLOSE, "close", pFile->zPath, lineno);
    }
    else {
      unixLogErrorAtLine(SQLITE_IOERR_CLOSE, "close", 0, lineno);
    }
  }
#if TKEYVFS_TRACE
  fprintf(stderr, "End   robust_close ...\n");
#endif /* TKEYVFS_TRACE */
  /**/
}

/*
 * ** This function performs the parts of the "close file" operation
 * ** common to all locking schemes. It closes the directory and file
 * ** handles, if they are valid, and sets all fields of the unixFile
 * ** structure to 0.
 * **
 * ** It is *not* necessary to hold the mutex when this routine is called,
 * ** even on VxWorks.  A mutex will be acquired on VxWorks by the
 * ** vxworksReleaseFileId() routine.
 * */
static int closeUnixFile(sqlite3_file * id)
{
  unixFile * pFile = (unixFile *)id;
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin closeUnixFile ...\n");
  if (((unixFile *)id)->zPath) {
    fprintf(stderr, "filename: %s\n", ((unixFile *)id)->zPath);
  }
#ifndef TKEYVFS_NO_ROOT
  fprintf(stderr, "saveToRootFile: %d\n", ((unixFile *)id)->saveToRootFile);
#endif // TKEYVFS_NO_ROOT
#endif /* TKEYVFS_TRACE */
#ifndef TKEYVFS_NO_ROOT
  if (pFile->saveToRootFile) {
    /**/
#if TKEYVFS_TRACE
    fprintf(stderr, "fileSize: 0x%016lx\n", (unsigned long long)pFile->fileSize);
#endif /* TKEYVFS_TRACE */
    /* Create a tkey which will contain the contents
    ** of the database in the root file */
    TKey * k = new TKey(pFile->zPath, "sqlite3 database file", TKey::Class(),
                        pFile->fileSize /*nbytes*/, pFile->rootFile /*dir*/);
#if TKEYVFS_TRACE
    /* Ask the key for the size of the database file it contains. */
    Int_t objlen = k->GetObjlen();
    fprintf(stderr, "objlen: %d\n", objlen);
#endif /* TKEYVFS_TRACE */
    /* Add the new key to the root file toplevel directory. */
    /* Note: The tkey is now owned by the root file. */
    Int_t cycle = pFile->rootFile->AppendKey(k);
    /* Get a pointer to the i/o buffer inside the tkey. */
    char * p = k->GetBuffer();
    /* Copy the entire in-memory database file into the tkey i/o buffer. */
    (void *)memcpy((void *)p, (void *)pFile->pBuf, (size_t)pFile->fileSize);
    /* Write the tkey contents to the root file. */
    /* Note: This has not yet written the top-level directory entry for the key. */
    Int_t cnt = k->WriteFile(cycle, 0 /*file*/);
    if (cnt == -1) {
      /* bad */
      fprintf(stderr, "tkeyvfs: failed to write root tkey containing database to root file!\n");
    }
    /* Force the root file to flush the top-level directory entry for our tkey to disk. */
    cnt = pFile->rootFile->Write();
    if (cnt < 0) {
      /* bad */
      fprintf(stderr, "tkeyvfs: failed to write root file to disk!\n");
    }
  }
#endif // TKEYVFS_NO_ROOT
  if (pFile->pBuf != NULL) {
    free(pFile->pBuf);
  }
  if (pFile->zPath != NULL) {
    free((void *)pFile->zPath);
  }
  memset(pFile, 0, sizeof(unixFile));
#if TKEYVFS_TRACE
  fprintf(stderr, "End   closeUnixFile ...\n");
#endif /* TKEYVFS_TRACE */
  return SQLITE_OK;
}

/*
** Create a temporary file name in zBuf.  zBuf must be allocated
** by the calling process and must be big enough to hold at least
** pVfs->mxPathname bytes.
*/
static int unixGetTempname(int nBuf, char * zBuf)
{
  static const unsigned char zChars[] =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789";
  unsigned int i, j;
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixGetTempname ...\n");
#endif /* TKEYVFS_TRACE */
  /* Check that the output buffer is large enough for the temporary file
  ** name. If it is not, return SQLITE_ERROR.
  */
  if ((strlen(SQLITE_TEMP_FILE_PREFIX) + 17) >= (size_t)nBuf) {
    /**/
#if TKEYVFS_TRACE
    fprintf(stderr, "End   unixGetTempname ...\n");
#endif /* TKEYVFS_TRACE */
    return SQLITE_ERROR;
  }
  sqlite3_snprintf(nBuf - 17, zBuf, SQLITE_TEMP_FILE_PREFIX);
  j = (int)strlen(zBuf);
  sqlite3_randomness(15, &zBuf[j]);
  for (i = 0; i < 15; i++, j++) {
    zBuf[j] = (char)zChars[((unsigned char)zBuf[j]) % (sizeof(zChars) - 1)];
  }
  zBuf[j] = 0;
#if TKEYVFS_TRACE
  fprintf(stderr, "End   unixGetTempname ...\n");
#endif /* TKEYVFS_TRACE */
  return SQLITE_OK;
}

/*
** This function is called to handle the SQLITE_FCNTL_SIZE_HINT
** file-control operation.
**
** If the user has configured a chunk-size for this file, it could be
** that the file needs to be extended at this point. Otherwise, the
** SQLITE_FCNTL_SIZE_HINT operation is a no-op for Unix.
*/
static int fcntlSizeHint(unixFile * pFile, i64 nByte)
{
  /**/
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin fcntlSizeHint ...\n");
#endif /* TKEYVFS_TRACE */
  if (pFile->szChunk) {
    i64 nSize; /* Required file size */
    i64 nAlloc;
    nSize = ((nByte + (pFile->szChunk - 1)) / pFile->szChunk) * pFile->szChunk;
    nAlloc = ((nSize + ((i64)(MEMPAGE - 1))) / ((i64)MEMPAGE)) * ((i64)MEMPAGE);
    if ((nSize > pFile->fileSize) && (nAlloc > pFile->bufAllocated)) {
      if (nAlloc > pFile->bufAllocated) {
        char * pNewBuf = (char *)realloc((void *)pFile->pBuf, (size_t)nAlloc);
        if (pNewBuf == NULL) {
          /**/
#if TKEYVFS_TRACE
          fprintf(stderr, "End   fcntlSizeHint ...\n");
#endif /* TKEYVFS_TRACE */
          return SQLITE_IOERR_WRITE;
        }
        (void)memset(pNewBuf + pFile->fileSize, 0, (size_t)(nAlloc - pFile->fileSize));
        pFile->pBuf = pNewBuf;
        pFile->bufAllocated = nAlloc;
      }
      else {
        (void)memset(pFile->pBuf + pFile->fileSize, 0, (size_t)(nSize - pFile->fileSize));
      }
      pFile->fileSize = nSize;
    }
  }
#if TKEYVFS_TRACE
  fprintf(stderr, "End   fcntlSizeHint ...\n");
#endif /* TKEYVFS_TRACE */
  return SQLITE_OK;
}

/*
** Seek to the offset passed as the second argument, then read cnt
** bytes into pBuf. Return the number of bytes actually read.
**
** NB:  If you define USE_PREAD or USE_PREAD64, then it might also
** be necessary to define _XOPEN_SOURCE to be 500.  This varies from
** one system to another.  Since SQLite does not define USE_PREAD
** any any form by default, we will not attempt to define _XOPEN_SOURCE.
** See tickets #2741 and #2681.
**
** To avoid stomping the errno value on a failed read the lastErrno value
** is set before returning.
*/
static int seekAndRead(unixFile * id, sqlite3_int64 offset, void * pBuf, int cnt)
{
  /**/
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin seekAndRead ...\n");
#endif /* TKEYVFS_TRACE */
  if (offset >= id->fileSize) {
    id->lastErrno = 0;
#if TKEYVFS_TRACE
    fprintf(stderr, "End   seekAndRead ...\n");
#endif /* TKEYVFS_TRACE */
    return 0;
  }
  if ((offset + cnt) > id->fileSize) {
    cnt = (offset + cnt) - id->fileSize;
  }
  (void *)memcpy(pBuf, (const void *)(id->pBuf + offset), (size_t)cnt);
#if TKEYVFS_TRACE
  fprintf(stderr, "End   seekAndRead ...\n");
#endif /* TKEYVFS_TRACE */
  return cnt;
}

/*
** Seek to the offset in id->offset then read cnt bytes into pBuf.
** Return the number of bytes actually read.  Update the offset.
**
** To avoid stomping the errno value on a failed write the lastErrno value
** is set before returning.
*/
static int seekAndWrite(unixFile * id, i64 offset, const void * pBuf, int cnt)
{
  unixFile * pFile = (unixFile *)id;
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin seekAndWrite ...\n");
#endif /* TKEYVFS_TRACE */
  if ((offset + (i64)cnt) > id->bufAllocated) {
    i64 nByte;
    i64 newBufSize;
    nByte = offset + ((i64)cnt);
    if (pFile->szChunk) {
      nByte = ((nByte + (pFile->szChunk - 1)) / pFile->szChunk) * pFile->szChunk;
    }
    newBufSize = ((nByte + (i64)(MEMPAGE - 1)) / ((i64)MEMPAGE)) * ((i64)MEMPAGE);
    char * pNewBuf = (char *)realloc((void *)id->pBuf, (size_t)(newBufSize));
    if (pNewBuf == NULL) {
      id->lastErrno = errno;
#if TKEYVFS_TRACE
      fprintf(stderr, "End   seekAndWrite ...\n");
#endif /* TKEYVFS_TRACE */
      return 0;
    }
    if ((offset + (i64)cnt) < newBufSize) {
      i64 zeroCnt = newBufSize - (offset + (i64)cnt);
      (void *)memset((void *)(pNewBuf + offset + (i64)cnt), 0, (size_t)zeroCnt);
    }
    id->pBuf = pNewBuf;
    id->bufAllocated = newBufSize;
  }
  (void *)memcpy((void *)(id->pBuf + offset), pBuf, (size_t)cnt);
  if ((offset + (i64)cnt) > id->fileSize) {
    id->fileSize = offset + (i64)cnt;
  }
#if TKEYVFS_TRACE
  fprintf(stderr, "End   seekAndWrite ...\n");
#endif /* TKEYVFS_TRACE */
  return cnt;
}

/*--------------------------------------------------------------------------*/
/* IoMethods calls */

static int nolockClose(sqlite3_file * id)
{
  /**/
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin nolockClose ...\n");
#endif /* TKEYVFS_TRACE */
  int val = closeUnixFile(id);
#if TKEYVFS_TRACE
  fprintf(stderr, "End   nolockClose ...\n");
#endif /* TKEYVFS_TRACE */
  return val;
}

/*
** Read data from a file into a buffer.  Return SQLITE_OK if all
** bytes were read successfully and SQLITE_IOERR if anything goes
** wrong.
*/
static int unixRead(sqlite3_file * id, void * pBuf, int amt, sqlite3_int64 offset)
{
  unixFile * pFile = (unixFile *)id;
  int got;
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixRead ...\n");
  if (((unixFile *)id)->zPath) {
    fprintf(stderr, "filename: %s\n", ((unixFile *)id)->zPath);
  }
  fprintf(stderr, "offset: 0x%016lx  amt: 0x%08x\n", (unsigned long long)offset, amt);
#endif /* TKEYVFS_TRACE */
  got = seekAndRead(pFile, offset, pBuf, amt);
  if (got == amt) {
    /**/
#if TKEYVFS_TRACE
    fprintf(stderr, "End   unixRead ...\n");
#endif /* TKEYVFS_TRACE */
    return SQLITE_OK;
  }
  else if (got < 0) {
    /* lastErrno set by seekAndRead */
#if TKEYVFS_TRACE
    fprintf(stderr, "End   unixRead ...\n");
#endif /* TKEYVFS_TRACE */
    return SQLITE_IOERR_READ;
  }
  else {
    pFile->lastErrno = 0; /* not a system error */
    /* Unread parts of the buffer must be zero-filled */
    memset(&((char *)pBuf)[got], 0, amt - got);
#if TKEYVFS_TRACE
    fprintf(stderr, "End   unixRead ...\n");
#endif /* TKEYVFS_TRACE */
    return SQLITE_IOERR_SHORT_READ;
  }
}

/*
** Write data from a buffer into a file.  Return SQLITE_OK on success
** or some other error code on failure.
*/
static int unixWrite(sqlite3_file * id, const void * pBuf, int amt, sqlite3_int64 offset)
{
  unixFile * pFile = (unixFile *)id;
  int wrote = 0;
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixWrite ...\n");
  if (((unixFile *)id)->zPath) {
    fprintf(stderr, "filename: %s\n", ((unixFile *)id)->zPath);
  }
  fprintf(stderr, "offset: 0x%016lx  amt: 0x%08x\n", (unsigned long long)offset, amt);
#endif /* TKEYVFS_TRACE */
  /* If we are doing a normal write to a database file (as opposed to
  ** doing a hot-journal rollback or a write to some file other than a
  ** normal database file) then record the fact that the database
  ** has changed.  If the transaction counter is modified, record that
  ** fact too.
  */
  if (pFile->inNormalWrite) {
    pFile->dbUpdate = 1;  /* The database has been modified */
    if ((offset <= 24) && (offset + amt >= 27)) {
      int rc;
      char oldCntr[4];
      rc = seekAndRead(pFile, 24, oldCntr, 4);
      if (rc != 4 || memcmp(oldCntr, &((char *)pBuf)[24 - offset], 4) != 0) {
        pFile->transCntrChng = 1;  /* The transaction counter has changed */
      }
    }
  }
  while ((amt > 0) && ((wrote = seekAndWrite(pFile, offset, pBuf, amt)) > 0)) {
    amt -= wrote;
    offset += wrote;
    pBuf = &((char *)pBuf)[wrote];
  }
  if (amt > 0) {
    if (wrote < 0) {
      /* lastErrno set by seekAndWrite */
#if TKEYVFS_TRACE
      fprintf(stderr, "End   unixWrite ...\n");
#endif /* TKEYVFS_TRACE */
      return SQLITE_IOERR_WRITE;
    }
    else {
      pFile->lastErrno = 0; /* not a system error */
#if TKEYVFS_TRACE
      fprintf(stderr, "End   unixWrite ...\n");
#endif /* TKEYVFS_TRACE */
      return SQLITE_FULL;
    }
  }
#if TKEYVFS_TRACE
  fprintf(stderr, "End   unixWrite ...\n");
#endif /* TKEYVFS_TRACE */
  return SQLITE_OK;
}

/*
** Truncate an open file to a specified size
*/
static int unixTruncate(sqlite3_file * id, i64 nByte)
{
  unixFile * pFile = (unixFile *)id;
  int rc;
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixTruncate ...\n");
  if (((unixFile *)id)->zPath) {
    fprintf(stderr, "filename: %s\n", ((unixFile *)id)->zPath);
  }
  fprintf(stderr, "nByte: 0x%016lx\n", (unsigned long long)nByte);
#endif /* TKEYVFS_TRACE */
  /* If the user has configured a chunk-size for this file, truncate the
  ** file so that it consists of an integer number of chunks (i.e. the
  ** actual file size after the operation may be larger than the requested
  ** size).
  */
  if (pFile->szChunk) {
    nByte = ((nByte + pFile->szChunk - 1) / pFile->szChunk) * pFile->szChunk;
  }
  if (nByte == 0) {
    free(pFile->pBuf);
    pFile->pBuf = (char *)calloc(1, MEMPAGE);
    if (pFile->pBuf == NULL) {
      pFile->bufAllocated = 0;
      pFile->fileSize = 0;
      pFile->lastErrno = errno;
#if TKEYVFS_TRACE
      fprintf(stderr, "End   unixTruncate ...\n");
#endif /* TKEYVFS_TRACE */
      return unixLogError(SQLITE_IOERR_TRUNCATE, "ftruncate", pFile->zPath);
    }
    pFile->bufAllocated = MEMPAGE;
    pFile->fileSize = 0;
  }
  else {
    i64 newBufSize = ((nByte + (i64)(MEMPAGE - 1)) / ((i64)MEMPAGE)) * ((i64)MEMPAGE);
    i64 zeroCnt = newBufSize - nByte;
    char * pNewBuf = (char *)realloc((void *)pFile->pBuf, (size_t)newBufSize);
    if (pNewBuf == NULL) {
      pFile->lastErrno = errno;
#if TKEYVFS_TRACE
      fprintf(stderr, "End   unixTruncate ...\n");
#endif /* TKEYVFS_TRACE */
      return unixLogError(SQLITE_IOERR_TRUNCATE, "ftruncate", pFile->zPath);
    }
    (void *)memset((void *)(pNewBuf + nByte), 0, (size_t)zeroCnt);
    pFile->pBuf = pNewBuf;
    pFile->bufAllocated = newBufSize;
    pFile->fileSize = nByte;
  }
  /* If we are doing a normal write to a database file (as opposed to
  ** doing a hot-journal rollback or a write to some file other than a
  ** normal database file) and we truncate the file to zero length,
  ** that effectively updates the change counter.  This might happen
  ** when restoring a database using the backup API from a zero-length
  ** source.
  */
  if (pFile->inNormalWrite && (nByte == 0)) {
    pFile->transCntrChng = 1;
  }
#if TKEYVFS_TRACE
  fprintf(stderr, "End   unixTruncate ...\n");
#endif /* TKEYVFS_TRACE */
  return SQLITE_OK;
}

/*
** Make sure all writes to a particular file are committed to disk.
**
** If dataOnly==0 then both the file itself and its metadata (file
** size, access time, etc) are synced.  If dataOnly!=0 then only the
** file data is synced.
**
** Under Unix, also make sure that the directory entry for the file
** has been created by fsync-ing the directory that contains the file.
** If we do not do this and we encounter a power failure, the directory
** entry for the journal might not exist after we reboot.  The next
** SQLite to access the file will not know that the journal exists (because
** the directory entry for the journal was never created) and the transaction
** will not roll back - possibly leading to database corruption.
*/
static int unixSync(sqlite3_file * id, int flags)
{
  UNUSED_PARAMETER2(id, flags);
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixSync ...\n");
  if (((unixFile *)id)->zPath) {
    fprintf(stderr, "filename: %s\n", ((unixFile *)id)->zPath);
  }
#endif /* TKEYVFS_TRACE */
#if TKEYVFS_TRACE
  fprintf(stderr, "End   unixSync ...\n");
#endif /* TKEYVFS_TRACE */
  return SQLITE_OK;
}

/*
** Determine the current size of a file in bytes
*/
static int unixFileSize(sqlite3_file * id, i64 * pSize)
{
  unixFile * p = (unixFile *)id;
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixFileSize ...\n");
  if (((unixFile *)id)->zPath) {
    fprintf(stderr, "filename: %s\n", ((unixFile *)id)->zPath);
  }
#endif /* TKEYVFS_TRACE */
  *pSize = p->fileSize;
  /* When opening a zero-size database, the findInodeInfo() procedure
  ** writes a single byte into that file in order to work around a bug
  ** in the OS-X msdos filesystem.  In order to avoid problems with upper
  ** layers, we need to report this file size as zero even though it is
  ** really 1.   Ticket #3260.
  */
  if (*pSize == 1) {
    *pSize = 0;
  }
#if TKEYVFS_TRACE
  fprintf(stderr, "End   unixFileSize ...\n");
#endif /* TKEYVFS_TRACE */
  return SQLITE_OK;
}

static int nolockLock(sqlite3_file * id /*NotUsed*/, int NotUsed2)
{
  /*UNUSED_PARAMETER2(NotUsed, NotUsed2);*/
  UNUSED_PARAMETER(NotUsed2);
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin nolockLock ...\n");
  if (((unixFile *)id)->zPath) {
    fprintf(stderr, "filename: %s\n", ((unixFile *)id)->zPath);
  }
#endif /* TKEYVFS_TRACE */
#if TKEYVFS_TRACE
  fprintf(stderr, "End   nolockLock ...\n");
#endif /* TKEYVFS_TRACE */
  return SQLITE_OK;
}

static int nolockUnlock(sqlite3_file * id /*NotUsed*/, int NotUsed2)
{
  /*UNUSED_PARAMETER2(NotUsed, NotUsed2);*/
  UNUSED_PARAMETER(NotUsed2);
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin nolockUnlock ...\n");
  if (((unixFile *)id)->zPath) {
    fprintf(stderr, "filename: %s\n", ((unixFile *)id)->zPath);
  }
#endif /* TKEYVFS_TRACE */
#if TKEYVFS_TRACE
  fprintf(stderr, "End   nolockUnlock ...\n");
#endif /* TKEYVFS_TRACE */
  return SQLITE_OK;
}

static int nolockCheckReservedLock(sqlite3_file * id /*NotUsed*/, int * pResOut)
{
  /*UNUSED_PARAMETER(NotUsed);*/
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin nolockCheckReservedLock ...\n");
  if (((unixFile *)id)->zPath) {
    fprintf(stderr, "filename: %s\n", ((unixFile *)id)->zPath);
  }
#endif /* TKEYVFS_TRACE */
  *pResOut = 0;
#if TKEYVFS_TRACE
  fprintf(stderr, "End   nolockCheckReservedLock ...\n");
#endif /* TKEYVFS_TRACE */
  return SQLITE_OK;
}

/*
** Information and control of an open file handle.
*/
static int unixFileControl(sqlite3_file * id, int op, void * pArg)
{
  /**/
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixFileControl ...\n");
  if (((unixFile *)id)->zPath) {
    fprintf(stderr, "filename: %s\n", ((unixFile *)id)->zPath);
  }
#endif /* TKEYVFS_TRACE */
  switch (op) {
    case SQLITE_FCNTL_LOCKSTATE: {
      /**/
#if TKEYVFS_TRACE
      fprintf(stderr, "op: LOCKSTATE\n");
#endif /* TKEYVFS_TRACE */
      *(int *)pArg = ((unixFile *)id)->eFileLock;
      /*SQLITE_LOCK_NONE*/
#if TKEYVFS_TRACE
      fprintf(stderr, "End   unixFileControl ...\n");
#endif /* TKEYVFS_TRACE */
      return SQLITE_OK;
    }
    case SQLITE_LAST_ERRNO: {
      /**/
#if TKEYVFS_TRACE
      fprintf(stderr, "op: LAST_ERRNO\n");
#endif /* TKEYVFS_TRACE */
      *(int *)pArg = ((unixFile *)id)->lastErrno;
#if TKEYVFS_TRACE
      fprintf(stderr, "End   unixFileControl ...\n");
#endif /* TKEYVFS_TRACE */
      return SQLITE_OK;
    }
    case SQLITE_FCNTL_CHUNK_SIZE: {
      /**/
#if TKEYVFS_TRACE
      fprintf(stderr, "op: CHUNK_SIZE\n");
      fprintf(stderr, "szChunk: %d\n", *(int *)pArg);
#endif /* TKEYVFS_TRACE */
      ((unixFile *)id)->szChunk = *(int *)pArg;
#if TKEYVFS_TRACE
      fprintf(stderr, "End   unixFileControl ...\n");
#endif /* TKEYVFS_TRACE */
      return SQLITE_OK;
    }
    case SQLITE_FCNTL_SIZE_HINT: {
      /**/
#if TKEYVFS_TRACE
      fprintf(stderr, "op: SIZE_HINT\n");
      fprintf(stderr, "hint: 0x%016lx\n", *(i64 *)pArg);
#endif /* TKEYVFS_TRACE */
      int val = fcntlSizeHint((unixFile *)id, *(i64 *)pArg);
#if TKEYVFS_TRACE
      fprintf(stderr, "End   unixFileControl ...\n");
#endif /* TKEYVFS_TRACE */
      return val;
    }
    /* The pager calls this method to signal that it has done
    ** a rollback and that the database is therefore unchanged and
    ** it hence it is OK for the transaction change counter to be
    ** unchanged.
    */
    case SQLITE_FCNTL_DB_UNCHANGED: {
      /**/
#if TKEYVFS_TRACE
      fprintf(stderr, "op: DB_UNCHANGED\n");
#endif /* TKEYVFS_TRACE */
      ((unixFile *)id)->dbUpdate = 0;
#if TKEYVFS_TRACE
      fprintf(stderr, "End   unixFileControl ...\n");
#endif /* TKEYVFS_TRACE */
      return SQLITE_OK;
    }
    case SQLITE_FCNTL_SYNC_OMITTED: {
      /**/
#if TKEYVFS_TRACE
      fprintf(stderr, "op: SYNC_OMITTED\n");
      fprintf(stderr, "End   unixFileControl ...\n");
#endif /* TKEYVFS_TRACE */
      return SQLITE_OK;  /* A no-op */
    }
  }
#if TKEYVFS_TRACE
  fprintf(stderr, "End   unixFileControl ...\n");
#endif /* TKEYVFS_TRACE */
  return SQLITE_NOTFOUND;
}

/*
** Return the sector size in bytes of the underlying block device for
** the specified file. This is almost always 512 bytes, but may be
** larger for some devices.
**
** SQLite code assumes this function cannot fail. It also assumes that
** if two files are created in the same file-system directory (i.e.
** a database and its journal file) that the sector size will be the
** same for both.
*/
static int unixSectorSize(sqlite3_file * NotUsed)
{
  UNUSED_PARAMETER(NotUsed);
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixSectorSize ...\n");
#endif /* TKEYVFS_TRACE */
#if TKEYVFS_TRACE
  fprintf(stderr, "End   unixSectorSize ...\n");
#endif /* TKEYVFS_TRACE */
  return SQLITE_DEFAULT_SECTOR_SIZE;
}

/*
** Return the device characteristics for the file. This is always 0 for unix.
*/
static int unixDeviceCharacteristics(sqlite3_file * NotUsed)
{
  UNUSED_PARAMETER(NotUsed);
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixDeviceCharacteristics ...\n");
#endif /* TKEYVFS_TRACE */
#if TKEYVFS_TRACE
  fprintf(stderr, "End   unixDeviceCharacteristics ...\n");
#endif /* TKEYVFS_TRACE */
  return 0;
}

/*--------------------------------------------------------------------------*/
/* VFS calls */

/*
** Open the file zPath.
**
** Previously, the SQLite OS layer used three functions in place of this
** one:
**
**     sqlite3OsOpenReadWrite();
**     sqlite3OsOpenReadOnly();
**     sqlite3OsOpenExclusive();
**
** These calls correspond to the following combinations of flags:
**
**     ReadWrite() ->     (READWRITE | CREATE)
**     ReadOnly()  ->     (READONLY)
**     OpenExclusive() -> (READWRITE | CREATE | EXCLUSIVE)
**
** The old OpenExclusive() accepted a boolean argument - "delFlag". If
** true, the file was configured to be automatically deleted when the
** file handle closed. To achieve the same effect using this new
** interface, add the DELETEONCLOSE flag to those specified above for
** OpenExclusive().
*/
static int unixOpen(
  sqlite3_vfs * /*pVfs*/,      /* The VFS for which this is the xOpen method */
  const char * zPath,          /* Pathname of file to be opened */
  sqlite3_file * pFile,        /* The file descriptor to be filled in */
  int flags,                   /* Input flags to control the opening */
  int * pOutFlags              /* Output flags returned to SQLite core */
)
{
  unixFile * p = (unixFile *)pFile;
  int eType = flags & 0xFFFFFF00; /* Type of file to open */
  int rc = SQLITE_OK;
  // int isExclusive  = (flags & SQLITE_OPEN_EXCLUSIVE); // Not used.
  int isDelete     = (flags & SQLITE_OPEN_DELETEONCLOSE);
  int isCreate     = (flags & SQLITE_OPEN_CREATE);
  int isReadonly   = (flags & SQLITE_OPEN_READONLY);
  int isReadWrite  = (flags & SQLITE_OPEN_READWRITE);
  char zTmpname[MAX_PATHNAME + 1];
  const char * zName = zPath;
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixOpen ...\n");
  if (zPath != NULL) {
    fprintf(stderr, "filename: %s\n", zPath);
  }
#endif /* TKEYVFS_TRACE */
  memset(p, 0, sizeof(unixFile));
  if (pOutFlags) {
    *pOutFlags = flags;
  }
  if (!zName) {
    rc = unixGetTempname(MAX_PATHNAME + 1, zTmpname);
    if (rc != SQLITE_OK) {
      return rc;
    }
    zName = zTmpname;
  }
  if (zName != NULL) {
    p->zPath = (char *)malloc(strlen(zName) + 1);
    if (p->zPath != NULL) {
      (void *)strcpy((char *)p->zPath, zName);
    }
  }
  p->lastErrno = 0;
  p->pMethod = &nolockIoMethods;
#ifndef TKEYVFS_NO_ROOT
  p->rootFile = (TFile *)NULL;
  if (eType & SQLITE_OPEN_MAIN_DB) {
    p->rootFile = gRootFile;
  }
  p->saveToRootFile = (p->rootFile &&
                       p->rootFile->IsWritable() &&
                       (eType & SQLITE_OPEN_MAIN_DB) &&
                       (isCreate || isReadWrite) &&
                       !isDelete);
#endif // TKEYVFS_NO_ROOT
  if ((eType & SQLITE_OPEN_MAIN_DB) && !isCreate) {
    /**/
    i64 nBytes = 0;
    i64 nAlloc = 0;
#ifndef TKEYVFS_NO_ROOT
    Bool_t status = kFALSE;
    TKey * k = 0;
    char * pKeyBuf = 0;
    /* Read the highest numbered cycle of the tkey which contains
    ** the database from the root file. */
    k = p->rootFile->GetKey(p->zPath, 9999 /*cycle*/);
    /* Force the tkey to allocate an i/o buffer for its contents. */
    k->SetBuffer();
    /* Read the contents of the tkey from the root file. */
    status = k->ReadFile();
    if (!status) {
      /**/
#if TKEYVFS_TRACE
      fprintf(stderr, "End   unixOpen ...\n");
#endif /* TKEYVFS_TRACE */
      rc = unixLogError(SQLITE_CANTOPEN_BKPT, "open", zName);
      return rc;
    }
    /* Get a pointer to the tkey i/o buffer. */
    pKeyBuf = k->GetBuffer();
    /* Get the size of the contained database file from the tkey. */
    nBytes = k->GetObjlen();
    /* Allocate enough memory pages to contain the database file. */
    nAlloc = ((nBytes + ((i64)(MEMPAGE - 1))) / ((i64)MEMPAGE)) * ((i64)MEMPAGE);
    p->pBuf = (char *)malloc((size_t)nAlloc);
#else // TKEYVFS_NO_ROOT
    /* If not using root, a database file read is a noop. */
    nBytes = 0;
    nAlloc = MEMPAGE;
    p->pBuf = (char *)calloc(1, MEMPAGE);
#endif // TKEYVFS_NO_ROOT
    if (p->pBuf == NULL) {
      /**/
#if TKEYVFS_TRACE
      fprintf(stderr, "End   unixOpen ...\n");
#endif /* TKEYVFS_TRACE */
      rc = unixLogError(SQLITE_CANTOPEN_BKPT, "open", zName);
      return rc;
    }
#ifndef TKEYVFS_NO_ROOT
    /* Copy the entire database file from the tkey i/o buffer
    ** into our in-memory database. */
    (void *)memcpy(p->pBuf, pKeyBuf, (size_t)nBytes);
#endif // TKEYVFS_NO_ROOT
    p->bufAllocated = nAlloc;
    p->fileSize = nBytes;
    /**/
  }
  else {
    p->pBuf = (char *)calloc(1, MEMPAGE);
    if (p->pBuf == NULL) {
      /**/
#if TKEYVFS_TRACE
      fprintf(stderr, "End   unixOpen ...\n");
#endif /* TKEYVFS_TRACE */
      rc = unixLogError(SQLITE_CANTOPEN_BKPT, "open", zName);
      return rc;
    }
    p->bufAllocated = MEMPAGE;
    p->fileSize = 0;
  }
  rc = SQLITE_OK;
#if TKEYVFS_TRACE
  fprintf(stderr, "End   unixOpen ...\n");
#endif /* TKEYVFS_TRACE */
  return rc;
}

/*
** Delete the file at zPath. If the dirSync argument is true, fsync()
** the directory after deleting the file.
*/
static int unixDelete(
  sqlite3_vfs * NotUsed,    /* VFS containing this as the xDelete method */
  const char * /*zPath*/,   /* Name of file to be deleted */
  int /*dirSync*/           /* If true, fsync() directory after deleting file */
)
{
  int rc = SQLITE_OK;
  UNUSED_PARAMETER(NotUsed);
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixDelete ...\n");
  if (zPath != NULL) {
    fprintf(stderr, "filename: %s\n", zPath);
  }
#endif /* TKEYVFS_TRACE */
#if TKEYVFS_TRACE
  fprintf(stderr, "End   unixDelete ...\n");
#endif /* TKEYVFS_TRACE */
  return rc;
}

/*
** Test the existance of or access permissions of file zPath. The
** test performed depends on the value of flags:
**
**     SQLITE_ACCESS_EXISTS: Return 1 if the file exists
**     SQLITE_ACCESS_READWRITE: Return 1 if the file is read and writable.
**     SQLITE_ACCESS_READONLY: Return 1 if the file is readable.
**
** Otherwise return 0.
*/
static int unixAccess(
  sqlite3_vfs * NotUsed,  /* The VFS containing this xAccess method */
  const char * /*zPath*/, /* Path of the file to examine */
  int flags,              /* What do we want to learn about the zPath file? */
  int * pResOut           /* Write result boolean here */
)
{
  int amode = 0;
  UNUSED_PARAMETER(NotUsed);
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixAccess ...\n");
  if (zPath != NULL) {
    fprintf(stderr, "filename: %s\n", zPath);
  }
#endif /* TKEYVFS_TRACE */
  switch (flags) {
    case SQLITE_ACCESS_EXISTS:
      /**/
#if TKEYVFS_TRACE
      fprintf(stderr, "op: SQLITE_ACCESS_EXISTS\n");
#endif /* TKEYVFS_TRACE */
      amode = F_OK;
      break;
    case SQLITE_ACCESS_READWRITE:
      /**/
#if TKEYVFS_TRACE
      fprintf(stderr, "op: SQLITE_ACCESS_READWRITE\n");
#endif /* TKEYVFS_TRACE */
      amode = W_OK | R_OK;
      break;
    case SQLITE_ACCESS_READ:
      /**/
#if TKEYVFS_TRACE
      fprintf(stderr, "op: SQLITE_ACCESS_READ\n");
#endif /* TKEYVFS_TRACE */
      amode = R_OK;
      break;
    default:
      assert(!"Invalid flags argument");
  }
  *pResOut = 0;
#if TKEYVFS_TRACE
  fprintf(stderr, "End   unixAccess ...\n");
#endif /* TKEYVFS_TRACE */
  return SQLITE_OK;
}

/*
** Turn a relative pathname into a full pathname. The relative path
** is stored as a nul-terminated string in the buffer pointed to by
** zPath.
**
** zOut points to a buffer of at least sqlite3_vfs.mxPathname bytes
** (in this case, MAX_PATHNAME bytes). The full-path is written to
** this buffer before returning.
*/
static int unixFullPathname(
  sqlite3_vfs * pVfs,           /* Pointer to vfs object */
  const char * zPath,           /* Possibly relative input path */
  int nOut,                     /* Size of output buffer in bytes */
  char * zOut                   /* Output buffer */
)
{
  /**/
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixFullPathName ...\n");
  if (zPath != NULL) {
    fprintf(stderr, "filename: %s\n", zPath);
  }
#endif /* TKEYVFS_TRACE */
  assert(pVfs->mxPathname == MAX_PATHNAME);
  UNUSED_PARAMETER(pVfs);
  zOut[nOut - 1] = '\0';
  sqlite3_snprintf(nOut, zOut, "%s", zPath);
#if TKEYVFS_TRACE
  fprintf(stderr, "End   unixFullPathName ...\n");
#endif /* TKEYVFS_TRACE */
  return SQLITE_OK;
}

/*
** Interfaces for opening a shared library, finding entry points
** within the shared library, and closing the shared library.
*/
static void * unixDlOpen(sqlite3_vfs * NotUsed, const char * zFilename)
{
  UNUSED_PARAMETER(NotUsed);
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixFullPathName ...\n");
#endif /* TKEYVFS_TRACE */
  void * p =  dlopen(zFilename, RTLD_NOW | RTLD_GLOBAL);
#if TKEYVFS_TRACE
  fprintf(stderr, "End   unixFullPathName ...\n");
#endif /* TKEYVFS_TRACE */
  return p;
}

/*
** SQLite calls this function immediately after a call to unixDlSym() or
** unixDlOpen() fails (returns a null pointer). If a more detailed error
** message is available, it is written to zBufOut. If no error message
** is available, zBufOut is left unmodified and SQLite uses a default
** error message.
*/
static void unixDlError(sqlite3_vfs * NotUsed, int nBuf, char * zBufOut)
{
  const char * zErr;
  UNUSED_PARAMETER(NotUsed);
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixDlError ...\n");
#endif /* TKEYVFS_TRACE */
  zErr = dlerror();
  if (zErr) {
    sqlite3_snprintf(nBuf, zBufOut, "%s", zErr);
  }
#if TKEYVFS_TRACE
  fprintf(stderr, "End   unixDlError ...\n");
#endif /* TKEYVFS_TRACE */
  /**/
}

static void (*unixDlSym(sqlite3_vfs * NotUsed, void * p, const char * zSym))(void)
{
  /*
  ** GCC with -pedantic-errors says that C90 does not allow a void* to be
  ** cast into a pointer to a function.  And yet the library dlsym() routine
  ** returns a void* which is really a pointer to a function.  So how do we
  ** use dlsym() with -pedantic-errors?
  **
  ** Variable x below is defined to be a pointer to a function taking
  ** parameters void* and const char* and returning a pointer to a function.
  ** We initialize x by assigning it a pointer to the dlsym() function.
  ** (That assignment requires a cast.)  Then we call the function that
  ** x points to.
  **
  ** This work-around is unlikely to work correctly on any system where
  ** you really cannot cast a function pointer into void*.  But then, on the
  ** other hand, dlsym() will not work on such a system either, so we have
  ** not really lost anything.
  */
  void (*(*x)(void *, const char *))(void);
  UNUSED_PARAMETER(NotUsed);
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixDlSym ...\n");
#endif /* TKEYVFS_TRACE */
  x = (void(*( *)(void *, const char *))(void))dlsym;
  return (*x)(p, zSym);
}

static void unixDlClose(sqlite3_vfs * NotUsed, void * pHandle)
{
  UNUSED_PARAMETER(NotUsed);
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixDlClose ...\n");
#endif /* TKEYVFS_TRACE */
  dlclose(pHandle);
#if TKEYVFS_TRACE
  fprintf(stderr, "End   unixDlClose ...\n");
#endif /* TKEYVFS_TRACE */
  /**/
}

/*
** Write nBuf bytes of random data to the supplied buffer zBuf.
*/
static int unixRandomness(sqlite3_vfs * NotUsed, int nBuf, char * zBuf)
{
  UNUSED_PARAMETER(NotUsed);
  assert((size_t)nBuf >= (sizeof(time_t) + sizeof(int)));
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixRandomness ...\n");
#endif /* TKEYVFS_TRACE */
  /* We have to initialize zBuf to prevent valgrind from reporting
  ** errors.  The reports issued by valgrind are incorrect - we would
  ** prefer that the randomness be increased by making use of the
  ** uninitialized space in zBuf - but valgrind errors tend to worry
  ** some users.  Rather than argue, it seems easier just to initialize
  ** the whole array and silence valgrind, even if that means less randomness
  ** in the random seed.
  **
  ** When testing, initializing zBuf[] to zero is all we do.  That means
  ** that we always use the same random number sequence.  This makes the
  ** tests repeatable.
  */
  memset(zBuf, 0, nBuf);
  {
    int pid, fd;
    fd = robust_open("/dev/urandom", O_RDONLY, 0);
    if (fd < 0) {
      time_t t;
      time(&t);
      memcpy(zBuf, &t, sizeof(t));
      pid = getpid();
      memcpy(&zBuf[sizeof(t)], &pid, sizeof(pid));
      assert(sizeof(t) + sizeof(pid) <= (size_t)nBuf);
      nBuf = sizeof(t) + sizeof(pid);
    }
    else {
      do {
        nBuf = osRead(fd, zBuf, nBuf);
      }
      while (nBuf < 0 && errno == EINTR);
      robust_close(0, fd, __LINE__);
    }
  }
#if TKEYVFS_TRACE
  fprintf(stderr, "End   unixRandomness ...\n");
#endif /* TKEYVFS_TRACE */
  return nBuf;
}


/*
** Sleep for a little while.  Return the amount of time slept.
** The argument is the number of microseconds we want to sleep.
** The return value is the number of microseconds of sleep actually
** requested from the underlying operating system, a number which
** might be greater than or equal to the argument, but not less
** than the argument.
*/
static int unixSleep(sqlite3_vfs * NotUsed, int microseconds)
{
  UNUSED_PARAMETER(NotUsed);
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixSleep ...\n");
#endif /* TKEYVFS_TRACE */
  usleep(microseconds);
#if TKEYVFS_TRACE
  fprintf(stderr, "End   unixSleep ...\n");
#endif /* TKEYVFS_TRACE */
  return microseconds;
}

/*
** Find the current time (in Universal Coordinated Time).  Write the
** current time and date as a Julian Day number into *prNow and
** return 0.  Return 1 if the time and date cannot be found.
*/
static int unixCurrentTime(sqlite3_vfs * NotUsed, double * prNow)
{
  sqlite3_int64 i;
  UNUSED_PARAMETER(NotUsed);
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixCurrentTime ...\n");
#endif /* TKEYVFS_TRACE */
  unixCurrentTimeInt64(0, &i);
  *prNow = i / 86400000.0;
#if TKEYVFS_TRACE
  fprintf(stderr, "End   unixCurrentTime ...\n");
#endif /* TKEYVFS_TRACE */
  return 0;
}

/*
** We added the xGetLastError() method with the intention of providing
** better low-level error messages when operating-system problems come up
** during SQLite operation.  But so far, none of that has been implemented
** in the core.  So this routine is never called.  For now, it is merely
** a place-holder.
*/
static int unixGetLastError(sqlite3_vfs * NotUsed, int NotUsed2, char * NotUsed3)
{
  UNUSED_PARAMETER(NotUsed);
  UNUSED_PARAMETER(NotUsed2);
  UNUSED_PARAMETER(NotUsed3);
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixGetLastError ...\n");
  fprintf(stderr, "End   unixGetLastError ...\n");
#endif /* TKEYVFS_TRACE */
  return 0;
}

/*
** Find the current time (in Universal Coordinated Time).  Write into *piNow
** the current time and date as a Julian Day number times 86_400_000.  In
** other words, write into *piNow the number of milliseconds since the Julian
** epoch of noon in Greenwich on November 24, 4714 B.C according to the
** proleptic Gregorian calendar.
**
** On success, return 0.  Return 1 if the time and date cannot be found.
*/
static int unixCurrentTimeInt64(sqlite3_vfs * NotUsed, sqlite3_int64 * piNow)
{
  static const sqlite3_int64 unixEpoch = 24405875 * (sqlite3_int64)8640000;
  struct timeval sNow;
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixCurrentTimeInt64 ...\n");
#endif /* TKEYVFS_TRACE */
  gettimeofday(&sNow, 0);
  *piNow = unixEpoch + 1000 * (sqlite3_int64)sNow.tv_sec + sNow.tv_usec / 1000;
  UNUSED_PARAMETER(NotUsed);
#if TKEYVFS_TRACE
  fprintf(stderr, "End   unixCurrentTimeInt64 ...\n");
#endif /* TKEYVFS_TRACE */
  return 0;
}

/*
** This is the xSetSystemCall() method of sqlite3_vfs for all of the
** "unix" VFSes.  Return SQLITE_OK opon successfully updating the
** system call pointer, or SQLITE_NOTFOUND if there is no configurable
** system call named zName.
*/
static int unixSetSystemCall(
  sqlite3_vfs * pNotUsed,       /* The VFS pointer.  Not used */
  const char * zName,           /* Name of system call to override */
  sqlite3_syscall_ptr pNewFunc  /* Pointer to new system call value */
)
{
  unsigned int i;
  int rc = SQLITE_NOTFOUND;
  UNUSED_PARAMETER(pNotUsed);
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixSetSystemCall ...\n");
#endif /* TKEYVFS_TRACE */
  if (zName == 0) {
    /* If no zName is given, restore all system calls to their default
    ** settings and return NULL
    */
    rc = SQLITE_OK;
    for (i = 0; i < sizeof(aSyscall) / sizeof(aSyscall[0]); i++) {
      if (aSyscall[i].pDefault) {
        aSyscall[i].pCurrent = aSyscall[i].pDefault;
      }
    }
  }
  else {
    /* If zName is specified, operate on only the one system call
    ** specified.
    */
    for (i = 0; i < sizeof(aSyscall) / sizeof(aSyscall[0]); i++) {
      if (strcmp(zName, aSyscall[i].zName) == 0) {
        if (aSyscall[i].pDefault == 0) {
          aSyscall[i].pDefault = aSyscall[i].pCurrent;
        }
        rc = SQLITE_OK;
        if (pNewFunc == 0) {
          pNewFunc = aSyscall[i].pDefault;
        }
        aSyscall[i].pCurrent = pNewFunc;
        break;
      }
    }
  }
#if TKEYVFS_TRACE
  fprintf(stderr, "End   unixSetSystemCall ...\n");
#endif /* TKEYVFS_TRACE */
  return rc;
}

/*
** Return the value of a system call.  Return NULL if zName is not a
** recognized system call name.  NULL is also returned if the system call
** is currently undefined.
*/
static sqlite3_syscall_ptr unixGetSystemCall(sqlite3_vfs * pNotUsed, const char * zName)
{
  unsigned int i;
  UNUSED_PARAMETER(pNotUsed);
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixGetSystemCall ...\n");
#endif /* TKEYVFS_TRACE */
  for (i = 0; i < sizeof(aSyscall) / sizeof(aSyscall[0]); i++) {
    if (strcmp(zName, aSyscall[i].zName) == 0) {
      /**/
#if TKEYVFS_TRACE
      fprintf(stderr, "End   unixGetSystemCall ...\n");
#endif /* TKEYVFS_TRACE */
      return aSyscall[i].pCurrent;
    }
  }
#if TKEYVFS_TRACE
  fprintf(stderr, "End   unixGetSystemCall ...\n");
#endif /* TKEYVFS_TRACE */
  return 0;
}

/*
** Return the name of the first system call after zName.  If zName==NULL
** then return the name of the first system call.  Return NULL if zName
** is the last system call or if zName is not the name of a valid
** system call.
*/
static const char * unixNextSystemCall(sqlite3_vfs * p, const char * zName)
{
  int i = -1;
  UNUSED_PARAMETER(p);
#if TKEYVFS_TRACE
  fprintf(stderr, "Begin unixNextSystemCall ...\n");
#endif /* TKEYVFS_TRACE */
  if (zName) {
    for (i = 0; i < ArraySize(aSyscall) - 1; i++) {
      if (strcmp(zName, aSyscall[i].zName) == 0) {
        break;
      }
    }
  }
  for (i++; i < ArraySize(aSyscall); i++) {
    if (aSyscall[i].pCurrent != 0) {
      /**/
#if TKEYVFS_TRACE
      fprintf(stderr, "End   unixNextSystemCall ...\n");
#endif /* TKEYVFS_TRACE */
      return aSyscall[i].zName;
    }
  }
#if TKEYVFS_TRACE
  fprintf(stderr, "End   unixNextSystemCall ...\n");
#endif /* TKEYVFS_TRACE */
  return 0;
}

#ifndef TKEYVFS_NO_ROOT
class RootFileSentry {
public:
  RootFileSentry(TFile * fPtr);
  ~RootFileSentry();
};

RootFileSentry::RootFileSentry(TFile * fPtr)
{
  gRootFile = fPtr;
}

RootFileSentry::~RootFileSentry()
{
  gRootFile = 0;
}
#endif

extern "C" {
  int tkeyvfs_init(void)
  {
    /*
    ** The following macro defines an initializer for an sqlite3_vfs object.
    ** The name of the VFS is NAME.  The pAppData is a pointer to a pointer
    ** to the "finder" function.  (pAppData is a pointer to a pointer because
    ** silly C90 rules prohibit a void* from being cast to a function pointer
    ** and so we have to go through the intermediate pointer to avoid problems
    ** when compiling with -pedantic-errors on GCC.)
    **
    ** The FINDER parameter to this macro is the name of the pointer to the
    ** finder-function.  The finder-function returns a pointer to the
    ** sqlite_io_methods object that implements the desired locking
    ** behaviors.  See the division above that contains the IOMETHODS
    ** macro for addition information on finder-functions.
    **
    ** Most finders simply return a pointer to a fixed sqlite3_io_methods
    ** object.  But the "autolockIoFinder" available on MacOSX does a little
    ** more than that; it looks at the filesystem type that hosts the
    ** database file and tries to choose an locking method appropriate for
    ** that filesystem time.
    */
#define UNIXVFS(VFSNAME, FINDER) {                        \
    1,                    /* iVersion */                \
    sizeof(unixFile),     /* szOsFile */              \
    MAX_PATHNAME,         /* mxPathname */            \
    0,                    /* pNext */                 \
    VFSNAME,              /* zName */                 \
    (void*)&FINDER,       /* pAppData */              \
    unixOpen,             /* xOpen */                 \
    unixDelete,           /* xDelete */               \
    unixAccess,           /* xAccess */               \
    unixFullPathname,     /* xFullPathname */         \
    unixDlOpen,           /* xDlOpen */               \
    unixDlError,          /* xDlError */              \
    unixDlSym,            /* xDlSym */                \
    unixDlClose,          /* xDlClose */              \
    unixRandomness,       /* xRandomness */           \
    unixSleep,            /* xSleep */                \
    unixCurrentTime,      /* xCurrentTime */          \
    unixGetLastError,     /* xGetLastError */         \
    /* unixCurrentTimeInt64, v2, xCurrentTimeInt64 */ \
    /* unixSetSystemCall,    v3, xSetSystemCall */    \
    /* unixGetSystemCall,    v3, xGetSystemCall */    \
    /* unixNextSystemCall,   v3, xNextSystemCall */   \
  }
    /*
    ** All default VFSes for unix are contained in the following array.
    **
    ** Note that the sqlite3_vfs.pNext field of the VFS object is modified
    ** by the SQLite core when the VFS is registered.  So the following
    ** array cannot be const.
    */
    static sqlite3_vfs aVfs[] = {
      UNIXVFS("tkeyvfs",     nolockIoFinder),
    };
    unsigned int i;          /* Loop counter */
    /* Double-check that the aSyscall[] array has been constructed
    ** correctly.  See ticket [bb3a86e890c8e96ab] */
    assert(ArraySize(aSyscall) == 16);
    /* Register all VFSes defined in the aVfs[] array */
    for (i = 0; i < (sizeof(aVfs) / sizeof(sqlite3_vfs)); i++) {
      sqlite3_vfs_register(&aVfs[i], 0);
    }
    return SQLITE_OK;
  }

  int tkeyvfs_open_v2(
    const char * filename,  /* Database filename (UTF-8) */
    sqlite3 ** ppDb,        /* OUT: SQLite db handle */
    int flags              /* Flags */
#ifndef TKEYVFS_NO_ROOT
    , TFile * rootFile     /* IN-OUT: Root file, must be already open. */
#endif // TKEYVFS_NO_ROOT
  )
  {
#ifndef TKEYVFS_NO_ROOT
    RootFileSentry rfs(rootFile);
    // Note that the sentry *is* the correct thing to do, here:
    // gRootFile is required in unixOpen(), which is called as part of
    // the chain of functions of which sqlite3_open_v2() is the first
    // call. By the time we return from sqlite3_open_v2() then, we no
    // longer require gRootFile and the sentry can do the job of
    // cleaning up when it goes out of scope.
#endif // TKEYVFS_NO_ROOT
    return sqlite3_open_v2(filename, ppDb, flags, "tkeyvfs");
  }
}
