#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-parameter"

/*#include "sqliteInt.h"*/
#include <sqlite3.h>
#define SQLITE_OPEN_WAL 0x00080000 /* VFS only */
#define SQLITE_FCNTL_SIZE_HINT 5
#define SQLITE_FCNTL_CHUNK_SIZE 6
#define SQLITE_FCNTL_SYNC_OMITTED 8

#define SQLITE_FCNTL_DB_UNCHANGED 0xca093fa0
#define SQLITE_DEFAULT_SECTOR_SIZE 512

#define UNUSED_PARAMETER(x) (void)(x)
#define UNUSED_PARAMETER2(x, y) UNUSED_PARAMETER(x), UNUSED_PARAMETER(y)

#define ArraySize(X) ((int)(sizeof(X) / sizeof(X[0])))

typedef sqlite_int64 i64;

#define _LARGE_FILE 1
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE 1

#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

/*
** Default permissions when creating a new file
*/
#ifndef SQLITE_DEFAULT_FILE_PERMISSIONS
#define SQLITE_DEFAULT_FILE_PERMISSIONS 0644
#endif

/*
** Maximum supported path-length.
*/
#define MAX_PATHNAME 512

#define SQLITE_TEMP_FILE_PREFIX "etilqs_"

typedef struct unixShmNode unixShmNode; /* Shared memory instance */

/*
** An instance of the following structure serves as the key used
** to locate a particular unixInodeInfo object.
*/
struct unixFileId {
  dev_t dev; /* Device number */
  ino_t ino; /* Inode number */
};

/*
** Sometimes, after a file handle is closed by SQLite, the file descriptor
** cannot be closed immediately. In these cases, instances of the following
** structure are used to store the file descriptor while waiting for an
** opportunity to either close or reuse it.
*/
struct UnixUnusedFd {
  int fd;                     /* File descriptor to close */
  int flags;                  /* Flags this file descriptor was opened with */
  struct UnixUnusedFd* pNext; /* Next unused file descriptor on same file */
};

/*
** An instance of the following structure is allocated for each open
** inode.  Or, on LinuxThreads, there is one of these structures for
** each inode opened by each thread.
**
** A single inode can have multiple file descriptors, so each unixFile
** structure contains a pointer to an instance of this object and this
** object keeps a count of the number of unixFile pointing to it.
*/
struct unixInodeInfo {
  struct unixFileId fileId;     /* The lookup key */
  int nShared;                  /* Number of SHARED locks held */
  unsigned char eFileLock;      /* One of SHARED_LOCK, RESERVED_LOCK etc. */
  unsigned char bProcessLock;   /* An exclusive process lock is held */
  int nRef;                     /* Number of pointers to this structure */
  unixShmNode* pShmNode;        /* Shared memory associated with this inode */
  int nLock;                    /* Number of outstanding file locks */
  struct UnixUnusedFd* pUnused; /* Unused file descriptors to close */
  struct unixInodeInfo* pNext;  /* List of all unixInodeInfo objects */
  struct unixInodeInfo* pPrev;  /*    .... doubly linked */
};

/*
** A lists of all unixInodeInfo objects.
*/
static struct unixInodeInfo* inodeList = 0;

/*
** The unixFile structure is subclass of sqlite3_file specific to the unix
** VFS implementations.
*/
typedef struct unixFile unixFile;
struct unixFile {
  sqlite3_io_methods const* pMethod; /* Always the first entry */
  struct unixInodeInfo* pInode;      /* Info about locks on this inode */
  int h;                             /* The file descriptor */
  int dirfd;                         /* File descriptor for the directory */
  unsigned char eFileLock;           /* The type of lock held on this fd */
  unsigned char ctrlFlags;           /* Behavioral bits.  UNIXFILE_* flags */
  int lastErrno;                     /* The unix errno from last I/O error */
  void* lockingContext;              /* Locking style specific state */
  struct UnixUnusedFd* pUnused;      /* Pre-allocated UnixUnusedFd */
  const char* zPath;                 /* Name of the file */
  int szChunk;                       /* Configured by FCNTL_CHUNK_SIZE */
  /* The next group of variables are used to track whether or not the
  ** transaction counter in bytes 24-27 of database files are updated
  ** whenever any part of the database changes.  An assertion fault will
  ** occur if a file is updated without also updating the transaction
  ** counter.  This test is made to avoid new problems similar to the
  ** one described by ticket #3584.
  */
  unsigned char transCntrChng; /* True if the transaction counter changed */
  unsigned char dbUpdate;      /* True if any part of database file changed */
  unsigned char inNormalWrite; /* True if in a normal write operation */
};

/*
** Allowed values for the unixFile.ctrlFlags bitmask:
*/
#define UNIXFILE_EXCL 0x01   /* Connections from one process only */
#define UNIXFILE_RDONLY 0x02 /* Connection is read only */

/*
** Define various macros that are missing from some systems.
*/
#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif
#ifdef SQLITE_DISABLE_LFS
#undef O_LARGEFILE
#define O_LARGEFILE 0
#endif
#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif
#ifndef O_BINARY
#define O_BINARY 0
#endif

#if 0  /* F13 */
typedef void (*sqlite3_syscall_ptr)(void);
#endif /* 0 */

/* Function Directory */
static int sqlite3CantopenError(int lineno);
static int sqlite3Strlen30(const char* z);
static const sqlite3_io_methods* nolockIoFinderImpl(const char* z, unixFile* p);
static int unixLogErrorAtLine(int errcode,
                              const char* zFunc,
                              const char* zPath,
                              int iLine);
static int robust_open(const char* z, int f, int m);
static void robust_close(unixFile* pFile, int h, int lineno);
static int robust_ftruncate(int h, sqlite3_int64 sz);
static int fillInUnixFile(sqlite3_vfs* pVfs,
                          int h,
                          int dirfd,
                          sqlite3_file* pId,
                          const char* zFilename,
                          int noLock,
                          int isDelete,
                          int isReadOnly);
static int openDirectory(const char* zFilename, int* pFd);
static const char* unixTempFileDir(void);
static int unixGetTempname(int nBuf, char* zBuf);
static struct UnixUnusedFd* findReusableFd(const char* zPath, int flags);
static int findCreateFileMode(const char* zPath, int flags, mode_t* pMode);
static int fcntlSizeHint(unixFile* pFile, i64 nByte);
static int full_fsync(int fd, int fullSync, int dataOnly);
static int seekAndRead(unixFile* id, sqlite3_int64 offset, void* pBuf, int cnt);
static int seekAndWrite(unixFile* id, i64 offset, const void* pBuf, int cnt);
/* IoMethods calls */
static int nolockClose(sqlite3_file* id);
static int unixRead(sqlite3_file* id,
                    void* pBuf,
                    int amt,
                    sqlite3_int64 offset);
static int unixWrite(sqlite3_file* id,
                     const void* pBuf,
                     int amt,
                     sqlite3_int64 offset);
static int unixTruncate(sqlite3_file* id, i64 nByte);
static int unixSync(sqlite3_file* id, int flags);
static int unixFileSize(sqlite3_file* id, i64* pSize);
static int nolockLock(sqlite3_file* NotUsed, int NotUsed2);
static int nolockUnlock(sqlite3_file* NotUsed, int NotUsed2);
static int nolockCheckReservedLock(sqlite3_file* NotUsed, int* pResOut);
static int unixFileControl(sqlite3_file* id, int op, void* pArg);
static int unixSectorSize(sqlite3_file* NotUsed);
static int unixDeviceCharacteristics(sqlite3_file* NotUsed);
/* VFS calls */
static int unixOpen(sqlite3_vfs* pVfs,
                    const char* zPath,
                    sqlite3_file* pFile,
                    int flags,
                    int* pOutFlags);
static int unixDelete(sqlite3_vfs* NotUsed, const char* zPath, int dirSync);
static int unixAccess(sqlite3_vfs* NotUsed,
                      const char* zPath,
                      int flags,
                      int* pResOut);
static int unixFullPathname(sqlite3_vfs* pVfs,
                            const char* zPath,
                            int nOut,
                            char* zOut);
static void* unixDlOpen(sqlite3_vfs* NotUsed, const char* zFilename);
static void unixDlError(sqlite3_vfs* NotUsed, int nBuf, char* zBufOut);
static void (*unixDlSym(sqlite3_vfs* NotUsed, void* p, const char* zSym))(void);
static void unixDlClose(sqlite3_vfs* NotUsed, void* pHandle);
static int unixRandomness(sqlite3_vfs* NotUsed, int nBuf, char* zBuf);
static int unixSleep(sqlite3_vfs* NotUsed, int microseconds);
static int unixCurrentTime(sqlite3_vfs* NotUsed, double* prNow);
static int unixGetLastError(sqlite3_vfs* NotUsed, int NotUsed2, char* NotUsed3);
static int unixCurrentTimeInt64(sqlite3_vfs* NotUsed, sqlite3_int64* piNow);
static int unixSetSystemCall(sqlite3_vfs* pNotUsed,
                             const char* zName,
                             sqlite3_syscall_ptr pNewFunc);
static sqlite3_syscall_ptr unixGetSystemCall(sqlite3_vfs* pNotUsed,
                                             const char* zName);
static const char* unixNextSystemCall(sqlite3_vfs* p, const char* zName);
/**/
int sqlite3_os_init(void);

static int
sqlite3CantopenError(int lineno)
{
#if 0
  sqlite3_log(SQLITE_CANTOPEN,
              "cannot open file at line %d of [%.10s]",
              lineno, 20 + sqlite3_sourceid());
#endif /* 0 */
  fprintf(stderr,
          "myvfs.c: cannot open file at line %d of [%.10s]",
          lineno,
          20 + sqlite3_sourceid());
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
static int
sqlite3Strlen30(const char* z)
{
  const char* z2 = z;
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
  const char* zName;            /* Name of the sytem call */
  sqlite3_syscall_ptr pCurrent; /* Current value of the system call */
  sqlite3_syscall_ptr pDefault; /* Default value */
} aSyscall[] = {
  {"open", (sqlite3_syscall_ptr)open, 0},
#define osOpen ((int (*)(const char*, int, ...))aSyscall[0].pCurrent)

  {"close", (sqlite3_syscall_ptr)close, 0},
#define osClose ((int (*)(int))aSyscall[1].pCurrent)

  {"access", (sqlite3_syscall_ptr)access, 0},
#define osAccess ((int (*)(const char*, int))aSyscall[2].pCurrent)

  {"getcwd", (sqlite3_syscall_ptr)getcwd, 0},
#define osGetcwd ((char* (*)(char*, size_t))aSyscall[3].pCurrent)

  {"stat", (sqlite3_syscall_ptr)stat, 0},
#define osStat ((int (*)(const char*, struct stat*))aSyscall[4].pCurrent)

/*
** The DJGPP compiler environment looks mostly like Unix, but it
** lacks the fcntl() system call.  So redefine fcntl() to be something
** that always succeeds.  This means that locking does not occur under
** DJGPP.  But it is DOS - what did you expect?
*/
#ifdef __DJGPP__
  {"fstat", 0, 0},
#define osFstat(a, b, c) 0
#else
  {"fstat", (sqlite3_syscall_ptr)fstat, 0},
#define osFstat ((int (*)(int, struct stat*))aSyscall[5].pCurrent)
#endif

  {"ftruncate", (sqlite3_syscall_ptr)ftruncate, 0},
#define osFtruncate ((int (*)(int, off_t))aSyscall[6].pCurrent)

  {"fcntl", (sqlite3_syscall_ptr)fcntl, 0},
#define osFcntl ((int (*)(int, int, ...))aSyscall[7].pCurrent)

  {"read", (sqlite3_syscall_ptr)read, 0},
#define osRead ((ssize_t(*)(int, void*, size_t))aSyscall[8].pCurrent)

#if defined(USE_PREAD)
  {"pread", (sqlite3_syscall_ptr)pread, 0},
#else
  {"pread", (sqlite3_syscall_ptr)0, 0},
#endif
#define osPread ((ssize_t(*)(int, void*, size_t, off_t))aSyscall[9].pCurrent)

#if defined(USE_PREAD64)
  {"pread64", (sqlite3_syscall_ptr)pread64, 0},
#else
  {"pread64", (sqlite3_syscall_ptr)0, 0},
#endif
#define osPread64 ((ssize_t(*)(int, void*, size_t, off_t))aSyscall[10].pCurrent)

  {"write", (sqlite3_syscall_ptr)write, 0},
#define osWrite ((ssize_t(*)(int, const void*, size_t))aSyscall[11].pCurrent)

#if defined(USE_PREAD)
  {"pwrite", (sqlite3_syscall_ptr)pwrite, 0},
#else
  {"pwrite", (sqlite3_syscall_ptr)0, 0},
#endif
#define osPwrite                                                               \
  ((ssize_t(*)(int, const void*, size_t, off_t))aSyscall[12].pCurrent)

#if defined(USE_PREAD64)
  {"pwrite64", (sqlite3_syscall_ptr)pwrite64, 0},
#else
  {"pwrite64", (sqlite3_syscall_ptr)0, 0},
#endif
#define osPwrite64                                                             \
  ((ssize_t(*)(int, const void*, size_t, off_t))aSyscall[13].pCurrent)

  {"fchmod", (sqlite3_syscall_ptr)0, 0},
#define osFchmod ((int (*)(int, mode_t))aSyscall[14].pCurrent)

#if defined(HAVE_POSIX_FALLOCATE) && HAVE_POSIX_FALLOCATE
  {"fallocate", (sqlite3_syscall_ptr)posix_fallocate, 0},
#else
  {"fallocate", (sqlite3_syscall_ptr)0, 0},
#endif
#define osFallocate ((int (*)(int, off_t, off_t))aSyscall[15].pCurrent)

}; /* End of the overrideable system calls */

static const sqlite3_io_methods nolockIoMethods = {
  1,                         /* iVersion */
  nolockClose,               /* xClose */
  unixRead,                  /* xRead */
  unixWrite,                 /* xWrite */
  unixTruncate,              /* xTruncate */
  unixSync,                  /* xSync */
  unixFileSize,              /* xFileSize */
  nolockLock,                /* xLock */
  nolockUnlock,              /* xUnlock */
  nolockCheckReservedLock,   /* xCheckReservedLock */
  unixFileControl,           /* xFileControl */
  unixSectorSize,            /* xSectorSize */
  unixDeviceCharacteristics, /* xDeviceCapabilities */
#if 0
  0,                          /* xShmMap */
  0,                          /* xShmLock */
  0,                          /* xShmBarrier */
  0                           /* xShmUnmap */
#endif /* 0 */
};

static const sqlite3_io_methods*
nolockIoFinderImpl(const char* z, unixFile* p)
{
  UNUSED_PARAMETER(z);
  UNUSED_PARAMETER(p);
  return &nolockIoMethods;
}

static const sqlite3_io_methods* (*const nolockIoFinder)(const char*,
                                                         unixFile* p) =
  nolockIoFinderImpl;

typedef const sqlite3_io_methods* (*finder_type)(const char*, unixFile*);

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
#define unixLogError(a, b, c) unixLogErrorAtLine(a, b, c, __LINE__)
static int
unixLogErrorAtLine(int errcode,       /* SQLite error code */
                   const char* zFunc, /* Name of OS function that failed */
                   const char* zPath, /* File path associated with error */
                   int iLine /* Source line number where error occurred */
)
{
  char* zErr;         /* Message from strerror() or equivalent */
  int iErrno = errno; /* Saved syscall error number */
  zErr = strerror(iErrno);
  assert(errcode != SQLITE_OK);
  if (zPath == 0) {
    zPath = "";
  }
#if 0
  sqlite3_log(errcode,
              "os_unix.c:%d: (%d) %s(%s) - %s",
              iLine, iErrno, zFunc, zPath, zErr
             );
#endif /* 0 */
  fprintf(
    stderr, "myvfs.c:%d: (%d) %s(%s) - %s", iLine, iErrno, zFunc, zPath, zErr);
  return errcode;
}

/*
** Retry open() calls that fail due to EINTR
*/
static int
robust_open(const char* z, int f, int m)
{
  int rc;
  do {
    rc = osOpen(z, f, m);
  } while (rc < 0 && errno == EINTR);
  return rc;
}

/*
 * ** Close a file descriptor.
 * **
 * ** We assume that close() almost always works, since it is only in a
 * ** very sick application or on a very sick platform that it might fail.
 * ** If it does fail, simply leak the file descriptor, but do log the
 * ** error.
 * **
 * ** Note that it is not safe to retry close() after EINTR since the
 * ** file descriptor might have already been reused by another thread.
 * ** So we don't even try to recover from an EINTR.  Just log the error
 * ** and move on.
 * */
static void
robust_close(unixFile* pFile, int h, int lineno)
{
  if (osClose(h)) {
    unixLogErrorAtLine(
      SQLITE_IOERR_CLOSE, "close", pFile ? pFile->zPath : 0, lineno);
  }
}

/*
** Retry ftruncate() calls that fail due to EINTR
*/
static int
robust_ftruncate(int h, sqlite3_int64 sz)
{
  int rc;
  do {
    rc = osFtruncate(h, sz);
  } while (rc < 0 && errno == EINTR);
  return rc;
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
static int
closeUnixFile(sqlite3_file* id)
{
  unixFile* pFile = (unixFile*)id;
  if (pFile->dirfd >= 0) {
    robust_close(pFile, pFile->dirfd, __LINE__);
    pFile->dirfd = -1;
  }
  if (pFile->h >= 0) {
    robust_close(pFile, pFile->h, __LINE__);
    pFile->h = -1;
  }
  sqlite3_free(pFile->pUnused);
  memset(pFile, 0, sizeof(unixFile));
  return SQLITE_OK;
}

/*
** Initialize the contents of the unixFile structure pointed to by pId.
*/
static int
fillInUnixFile(sqlite3_vfs* pVfs, /* Pointer to vfs object */
               int h,     /* Open file descriptor of file being opened */
               int dirfd, /* Directory file descriptor */
               sqlite3_file* pId,     /* Write to the unixFile structure here */
               const char* zFilename, /* Name of the file being opened */
               int noLock,            /* Omit locking if true */
               int isDelete,          /* Delete on close if true */
               int isReadOnly         /* True if the file is opened read-only */
)
{
  const sqlite3_io_methods* pLockingStyle;
  unixFile* pNew = (unixFile*)pId;
  int rc = SQLITE_OK;
  assert(pNew->pInode == NULL);
  /* Parameter isDelete is only used on vxworks. Express this explicitly
  ** here to prevent compiler warnings about unused parameters.
  */
  UNUSED_PARAMETER(isDelete);
  /* Usually the path zFilename should not be a relative pathname. The
  ** exception is when opening the proxy "conch" file in builds that
  ** include the special Apple locking styles.
  */
  assert(zFilename == 0 || zFilename[0] == '/');
  pNew->h = h;
  pNew->dirfd = dirfd;
  pNew->zPath = zFilename;
  if (strcmp(pVfs->zName, "unix-excl") == 0) {
    pNew->ctrlFlags = UNIXFILE_EXCL;
  } else {
    pNew->ctrlFlags = 0;
  }
  if (isReadOnly) {
    pNew->ctrlFlags |= UNIXFILE_RDONLY;
  }
  pLockingStyle = &nolockIoMethods;
  pNew->lastErrno = 0;
  if (rc != SQLITE_OK) {
    if (dirfd >= 0) {
      robust_close(pNew, dirfd, __LINE__);
    }
    if (h >= 0) {
      robust_close(pNew, h, __LINE__);
    }
  } else {
    pNew->pMethod = pLockingStyle;
  }
  return rc;
}

/*
** Open a file descriptor to the directory containing file zFilename.
** If successful, *pFd is set to the opened file descriptor and
** SQLITE_OK is returned. If an error occurs, either SQLITE_NOMEM
** or SQLITE_CANTOPEN is returned and *pFd is set to an undefined
** value.
**
** If SQLITE_OK is returned, the caller is responsible for closing
** the file descriptor *pFd using close().
*/
static int
openDirectory(const char* zFilename, int* pFd)
{
  int ii;
  int fd = -1;
  char zDirname[MAX_PATHNAME + 1];
  sqlite3_snprintf(MAX_PATHNAME, zDirname, "%s", zFilename);
  for (ii = (int)strlen(zDirname); ii > 1 && zDirname[ii] != '/'; ii--) {
    ;
  }
  if (ii > 0) {
    zDirname[ii] = '\0';
    fd = robust_open(zDirname, O_RDONLY | O_BINARY, 0);
  }
  *pFd = fd;
  return (fd >= 0 ? SQLITE_OK :
                    unixLogError(SQLITE_CANTOPEN_BKPT, "open", zDirname));
}

/*
** Return the name of a directory in which to put temporary files.
** If no suitable temporary file directory can be found, return NULL.
*/
static const char*
unixTempFileDir(void)
{
  static const char* azDirs[] = {
    0, 0, "/var/tmp", "/usr/tmp", "/tmp", 0 /* List terminator */
  };
  unsigned int i;
  struct stat buf;
  const char* zDir = 0;
  azDirs[0] = sqlite3_temp_directory;
  if (!azDirs[1]) {
    azDirs[1] = getenv("TMPDIR");
  }
  for (i = 0; i < sizeof(azDirs) / sizeof(azDirs[0]); zDir = azDirs[i++]) {
    if (zDir == 0) {
      continue;
    }
    if (osStat(zDir, &buf)) {
      continue;
    }
    if (!S_ISDIR(buf.st_mode)) {
      continue;
    }
    if (osAccess(zDir, 07)) {
      continue;
    }
    break;
  }
  return zDir;
}

/*
** Create a temporary file name in zBuf.  zBuf must be allocated
** by the calling process and must be big enough to hold at least
** pVfs->mxPathname bytes.
*/
static int
unixGetTempname(int nBuf, char* zBuf)
{
  static const unsigned char zChars[] = "abcdefghijklmnopqrstuvwxyz"
                                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "0123456789";
  unsigned int i, j;
  const char* zDir;
  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing.
  */
  zDir = unixTempFileDir();
  if (zDir == 0) {
    zDir = ".";
  }
  /* Check that the output buffer is large enough for the temporary file
  ** name. If it is not, return SQLITE_ERROR.
  */
  if ((strlen(zDir) + strlen(SQLITE_TEMP_FILE_PREFIX) + 17) >= (size_t)nBuf) {
    return SQLITE_ERROR;
  }
  do {
    sqlite3_snprintf(nBuf - 17, zBuf, "%s/" SQLITE_TEMP_FILE_PREFIX, zDir);
    j = (int)strlen(zBuf);
    sqlite3_randomness(15, &zBuf[j]);
    for (i = 0; i < 15; i++, j++) {
      zBuf[j] = (char)zChars[((unsigned char)zBuf[j]) % (sizeof(zChars) - 1)];
    }
    zBuf[j] = 0;
  } while (osAccess(zBuf, 0) == 0);
  return SQLITE_OK;
}

/*
** Search for an unused file descriptor that was opened on the database
** file (not a journal or master-journal file) identified by pathname
** zPath with SQLITE_OPEN_XXX flags matching those passed as the second
** argument to this function.
**
** Such a file descriptor may exist if a database connection was closed
** but the associated file descriptor could not be closed because some
** other file descriptor open on the same file is holding a file-lock.
** Refer to comments in the unixClose() function and the lengthy comment
** describing "Posix Advisory Locking" at the start of this file for
** further details. Also, ticket #4018.
**
** If a suitable file descriptor is found, then it is returned. If no
** such file descriptor is located, -1 is returned.
*/
static struct UnixUnusedFd*
findReusableFd(const char* zPath, int flags)
{
  struct UnixUnusedFd* pUnused = 0;
  /* Do not search for an unused file descriptor on vxworks. Not because
  ** vxworks would not benefit from the change (it might, we're not sure),
  ** but because no way to test it is currently available. It is better
  ** not to risk breaking vxworks support for the sake of such an obscure
  ** feature.  */
  struct stat sStat; /* Results of stat() call */
  /* A stat() call may fail for various reasons. If this happens, it is
  ** almost certain that an open() call on the same path will also fail.
  ** For this reason, if an error occurs in the stat() call here, it is
  ** ignored and -1 is returned. The caller will try to open a new file
  ** descriptor on the same path, fail, and return an error to SQLite.
  **
  ** Even if a subsequent open() call does succeed, the consequences of
  ** not searching for a resusable file descriptor are not dire.  */
  if (0 == stat(zPath, &sStat)) {
    struct unixInodeInfo* pInode;
    pInode = inodeList;
    while (pInode && (pInode->fileId.dev != sStat.st_dev ||
                      pInode->fileId.ino != sStat.st_ino)) {
      pInode = pInode->pNext;
    }
    if (pInode) {
      struct UnixUnusedFd** pp;
      for (pp = &pInode->pUnused; *pp && (*pp)->flags != flags;
           pp = &((*pp)->pNext)) {
        ;
      }
      pUnused = *pp;
      if (pUnused) {
        *pp = pUnused->pNext;
      }
    }
  }
  return pUnused;
}

/*
** This function is called by unixOpen() to determine the unix permissions
** to create new files with. If no error occurs, then SQLITE_OK is returned
** and a value suitable for passing as the third argument to open(2) is
** written to *pMode. If an IO error occurs, an SQLite error code is
** returned and the value of *pMode is not modified.
**
** If the file being opened is a temporary file, it is always created with
** the octal permissions 0600 (read/writable by owner only). If the file
** is a database or master journal file, it is created with the permissions
** mask SQLITE_DEFAULT_FILE_PERMISSIONS.
**
** Finally, if the file being opened is a WAL or regular journal file, then
** this function queries the file-system for the permissions on the
** corresponding database file and sets *pMode to this value. Whenever
** possible, WAL and journal files are created using the same permissions
** as the associated database file.
*/
static int
findCreateFileMode(
  const char* zPath, /* Path of file (possibly) being created */
  int flags,         /* Flags passed as 4th argument to xOpen() */
  mode_t* pMode      /* OUT: Permissions to open file with */
)
{
  int rc = SQLITE_OK; /* Return Code */
  if (flags & (SQLITE_OPEN_WAL | SQLITE_OPEN_MAIN_JOURNAL)) {
    char zDb[MAX_PATHNAME + 1]; /* Database file path */
    int nDb;                    /* Number of valid bytes in zDb */
    struct stat sStat;          /* Output of stat() on database file */
    /* zPath is a path to a WAL or journal file. The following block derives
    ** the path to the associated database file from zPath. This block handles
    ** the following naming conventions:
    **
    **   "<path to db>-journal"
    **   "<path to db>-wal"
    **   "<path to db>-journal-NNNN"
    **   "<path to db>-wal-NNNN"
    **
    ** where NNNN is a 4 digit decimal number. The NNNN naming schemes are
    ** used by the test_multiplex.c module.
    */
    nDb = sqlite3Strlen30(zPath) - 1;
    while (nDb > 0 && zPath[nDb] != 'l') {
      nDb--;
    }
    nDb -= ((flags & SQLITE_OPEN_WAL) ? 3 : 7);
    memcpy(zDb, zPath, nDb);
    zDb[nDb] = '\0';
    if (0 == stat(zDb, &sStat)) {
      *pMode = sStat.st_mode & 0777;
    } else {
      rc = SQLITE_IOERR_FSTAT;
    }
  } else if (flags & SQLITE_OPEN_DELETEONCLOSE) {
    *pMode = 0600;
  } else {
    *pMode = SQLITE_DEFAULT_FILE_PERMISSIONS;
  }
  return rc;
}

/*
** This function is called to handle the SQLITE_FCNTL_SIZE_HINT
** file-control operation.
**
** If the user has configured a chunk-size for this file, it could be
** that the file needs to be extended at this point. Otherwise, the
** SQLITE_FCNTL_SIZE_HINT operation is a no-op for Unix.
*/
static int
fcntlSizeHint(unixFile* pFile, i64 nByte)
{
  if (pFile->szChunk) {
    i64 nSize;       /* Required file size */
    struct stat buf; /* Used to hold return values of fstat() */
    if (osFstat(pFile->h, &buf)) {
      return SQLITE_IOERR_FSTAT;
    }
    nSize = ((nByte + pFile->szChunk - 1) / pFile->szChunk) * pFile->szChunk;
    if (nSize > (i64)buf.st_size) {
      /* The code below is handling the return value of osFallocate()
      ** correctly. posix_fallocate() is defined to "returns zero on success,
      ** or an error number on  failure". See the manpage for details. */
      int err;
      do {
        err = osFallocate(pFile->h, buf.st_size, nSize - buf.st_size);
      } while (err == EINTR);
      if (err) {
        return SQLITE_IOERR_WRITE;
      }
    }
  }
  return SQLITE_OK;
}

/*
** The fsync() system call does not work as advertised on many
** unix systems.  The following procedure is an attempt to make
** it work better.
**
** The SQLITE_NO_SYNC macro disables all fsync()s.  This is useful
** for testing when we want to run through the test suite quickly.
** You are strongly advised *not* to deploy with SQLITE_NO_SYNC
** enabled, however, since with SQLITE_NO_SYNC enabled, an OS crash
** or power failure will likely corrupt the database file.
**
** SQLite sets the dataOnly flag if the size of the file is unchanged.
** The idea behind dataOnly is that it should only write the file content
** to disk, not the inode.  We only set dataOnly if the file size is
** unchanged since the file size is part of the inode.  However,
** Ted Ts'o tells us that fdatasync() will also write the inode if the
** file size has changed.  The only real difference between fdatasync()
** and fsync(), Ted tells us, is that fdatasync() will not flush the
** inode if the mtime or owner or other inode attributes have changed.
** We only care about the file size, not the other file attributes, so
** as far as SQLite is concerned, an fdatasync() is always adequate.
** So, we always use fdatasync() if it is available, regardless of
** the value of the dataOnly flag.
*/
static int
full_fsync(int fd, int fullSync, int dataOnly)
{
  int rc;
  UNUSED_PARAMETER(fullSync);
  UNUSED_PARAMETER(dataOnly);
#if __APPLE__ && __MACH__
  rc = fcntl(fd, F_FULLFSYNC);
#else
  rc = fdatasync(fd);
#endif
  return rc;
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
static int
seekAndRead(unixFile* id, sqlite3_int64 offset, void* pBuf, int cnt)
{
  int got;
  i64 newOffset;
  newOffset = lseek(id->h, offset, SEEK_SET);
  if (newOffset != offset) {
    if (newOffset == -1) {
      ((unixFile*)id)->lastErrno = errno;
    } else {
      ((unixFile*)id)->lastErrno = 0;
    }
    return -1;
  }
  do {
    got = osRead(id->h, pBuf, cnt);
  } while (got < 0 && errno == EINTR);
  if (got < 0) {
    ((unixFile*)id)->lastErrno = errno;
  }
  return got;
}

/*
** Seek to the offset in id->offset then read cnt bytes into pBuf.
** Return the number of bytes actually read.  Update the offset.
**
** To avoid stomping the errno value on a failed write the lastErrno value
** is set before returning.
*/
static int
seekAndWrite(unixFile* id, i64 offset, const void* pBuf, int cnt)
{
  int got;
  i64 newOffset;
  newOffset = lseek(id->h, offset, SEEK_SET);
  if (newOffset != offset) {
    if (newOffset == -1) {
      ((unixFile*)id)->lastErrno = errno;
    } else {
      ((unixFile*)id)->lastErrno = 0;
    }
    return -1;
  }
  do {
    got = osWrite(id->h, pBuf, cnt);
  } while (got < 0 && errno == EINTR);
  if (got < 0) {
    ((unixFile*)id)->lastErrno = errno;
  }
  return got;
}

/*--------------------------------------------------------------------------*/
/* IoMethods calls */

static int
nolockClose(sqlite3_file* id)
{
  fprintf(stderr, "trace: begin nolockClose ...\n");
  return closeUnixFile(id);
}

/*
** Read data from a file into a buffer.  Return SQLITE_OK if all
** bytes were read successfully and SQLITE_IOERR if anything goes
** wrong.
*/
static int
unixRead(sqlite3_file* id, void* pBuf, int amt, sqlite3_int64 offset)
{
  unixFile* pFile = (unixFile*)id;
  int got;
  fprintf(stderr, "trace: begin unixRead ...\n");
  assert(id);
  /* If this is a database file (not a journal, master-journal or temp
  ** file), the bytes in the locking range should never be read or written. */
#if 0
  assert(pFile->pUnused == 0
         || offset >= PENDING_BYTE + 512
         || offset + amt <= PENDING_BYTE
        );
#endif
  got = seekAndRead(pFile, offset, pBuf, amt);
  if (got == amt) {
    return SQLITE_OK;
  } else if (got < 0) {
    /* lastErrno set by seekAndRead */
    return SQLITE_IOERR_READ;
  } else {
    pFile->lastErrno = 0; /* not a system error */
    /* Unread parts of the buffer must be zero-filled */
    memset(&((char*)pBuf)[got], 0, amt - got);
    return SQLITE_IOERR_SHORT_READ;
  }
}

/*
** Write data from a buffer into a file.  Return SQLITE_OK on success
** or some other error code on failure.
*/
static int
unixWrite(sqlite3_file* id, const void* pBuf, int amt, sqlite3_int64 offset)
{
  unixFile* pFile = (unixFile*)id;
  int wrote = 0;
  fprintf(stderr, "trace: begin unixWrite ...\n");
  assert(id);
  assert(amt > 0);
  /* If this is a database file (not a journal, master-journal or temp
  ** file), the bytes in the locking range should never be read or written. */
#if 0
  assert(pFile->pUnused == 0
         || offset >= PENDING_BYTE + 512
         || offset + amt <= PENDING_BYTE
        );
#endif
  /* If we are doing a normal write to a database file (as opposed to
  ** doing a hot-journal rollback or a write to some file other than a
  ** normal database file) then record the fact that the database
  ** has changed.  If the transaction counter is modified, record that
  ** fact too.
  */
  if (pFile->inNormalWrite) {
    pFile->dbUpdate = 1; /* The database has been modified */
    if (offset <= 24 && offset + amt >= 27) {
      int rc;
      char oldCntr[4];
      rc = seekAndRead(pFile, 24, oldCntr, 4);
      if (rc != 4 || memcmp(oldCntr, &((char*)pBuf)[24 - offset], 4) != 0) {
        pFile->transCntrChng = 1; /* The transaction counter has changed */
      }
    }
  }
  while (amt > 0 && (wrote = seekAndWrite(pFile, offset, pBuf, amt)) > 0) {
    amt -= wrote;
    offset += wrote;
    pBuf = &((char*)pBuf)[wrote];
  }
  if (amt > 0) {
    if (wrote < 0) {
      /* lastErrno set by seekAndWrite */
      return SQLITE_IOERR_WRITE;
    } else {
      pFile->lastErrno = 0; /* not a system error */
      return SQLITE_FULL;
    }
  }
  return SQLITE_OK;
}

/*
** Truncate an open file to a specified size
*/
static int
unixTruncate(sqlite3_file* id, i64 nByte)
{
  unixFile* pFile = (unixFile*)id;
  int rc;
  fprintf(stderr, "trace: begin unixTruncate ...\n");
  assert(pFile);
  /* If the user has configured a chunk-size for this file, truncate the
  ** file so that it consists of an integer number of chunks (i.e. the
  ** actual file size after the operation may be larger than the requested
  ** size).
  */
  if (pFile->szChunk) {
    nByte = ((nByte + pFile->szChunk - 1) / pFile->szChunk) * pFile->szChunk;
  }
  rc = robust_ftruncate(pFile->h, (off_t)nByte);
  if (rc) {
    pFile->lastErrno = errno;
    return unixLogError(SQLITE_IOERR_TRUNCATE, "ftruncate", pFile->zPath);
  } else {
    /* If we are doing a normal write to a database file (as opposed to
    ** doing a hot-journal rollback or a write to some file other than a
    ** normal database file) and we truncate the file to zero length,
    ** that effectively updates the change counter.  This might happen
    ** when restoring a database using the backup API from a zero-length
    ** source.
    */
    if (pFile->inNormalWrite && nByte == 0) {
      pFile->transCntrChng = 1;
    }
    return SQLITE_OK;
  }
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
static int
unixSync(sqlite3_file* id, int flags)
{
  int rc;
  unixFile* pFile = (unixFile*)id;
  int isDataOnly = (flags & SQLITE_SYNC_DATAONLY);
  int isFullsync = (flags & 0x0F) == SQLITE_SYNC_FULL;
  fprintf(stderr, "trace: begin unixSync ...\n");
  /* Check that one of SQLITE_SYNC_NORMAL or FULL was passed */
  assert((flags & 0x0F) == SQLITE_SYNC_NORMAL ||
         (flags & 0x0F) == SQLITE_SYNC_FULL);
  /* Unix cannot, but some systems may return SQLITE_FULL from here. This
  ** line is to test that doing so does not cause any problems.
  */
  assert(pFile);
  rc = full_fsync(pFile->h, isFullsync, isDataOnly);
  if (rc) {
    pFile->lastErrno = errno;
    return unixLogError(SQLITE_IOERR_FSYNC, "full_fsync", pFile->zPath);
  }
  if (pFile->dirfd >= 0) {
    /* The directory sync is only attempted if full_fsync is
    ** turned off or unavailable.  If a full_fsync occurred above,
    ** then the directory sync is superfluous.
    */
    if (full_fsync(pFile->dirfd, 0, 0)) {
      /*
      ** We have received multiple reports of fsync() returning
      ** errors when applied to directories on certain file systems.
      ** A failed directory sync is not a big deal.  So it seems
      ** better to ignore the error.  Ticket #1657
      */
      /* pFile->lastErrno = errno; */
      /* return SQLITE_IOERR; */
    }
    /* Only need to sync once, so close the  directory when we are done */
    robust_close(pFile, pFile->dirfd, __LINE__);
    pFile->dirfd = -1;
  }
  return rc;
}

/*
** Determine the current size of a file in bytes
*/
static int
unixFileSize(sqlite3_file* id, i64* pSize)
{
  int rc;
  struct stat buf;
  assert(id);
  rc = osFstat(((unixFile*)id)->h, &buf);
  if (rc != 0) {
    ((unixFile*)id)->lastErrno = errno;
    return SQLITE_IOERR_FSTAT;
  }
  *pSize = buf.st_size;
  /* When opening a zero-size database, the findInodeInfo() procedure
  ** writes a single byte into that file in order to work around a bug
  ** in the OS-X msdos filesystem.  In order to avoid problems with upper
  ** layers, we need to report this file size as zero even though it is
  ** really 1.   Ticket #3260.
  */
  if (*pSize == 1) {
    *pSize = 0;
  }
  return SQLITE_OK;
}

static int
nolockLock(sqlite3_file* NotUsed, int NotUsed2)
{
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  return SQLITE_OK;
}

static int
nolockUnlock(sqlite3_file* NotUsed, int NotUsed2)
{
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  return SQLITE_OK;
}

static int
nolockCheckReservedLock(sqlite3_file* NotUsed, int* pResOut)
{
  UNUSED_PARAMETER(NotUsed);
  *pResOut = 0;
  return SQLITE_OK;
}

/*
** Information and control of an open file handle.
*/
static int
unixFileControl(sqlite3_file* id, int op, void* pArg)
{
  switch (op) {
    case SQLITE_FCNTL_LOCKSTATE: {
      *(int*)pArg = ((unixFile*)id)->eFileLock;
      return SQLITE_OK;
    }
    case SQLITE_LAST_ERRNO: {
      *(int*)pArg = ((unixFile*)id)->lastErrno;
      return SQLITE_OK;
    }
    case SQLITE_FCNTL_CHUNK_SIZE: {
      ((unixFile*)id)->szChunk = *(int*)pArg;
      return SQLITE_OK;
    }
    case SQLITE_FCNTL_SIZE_HINT: {
      return fcntlSizeHint((unixFile*)id, *(i64*)pArg);
    }
    /* The pager calls this method to signal that it has done
    ** a rollback and that the database is therefore unchanged and
    ** it hence it is OK for the transaction change counter to be
    ** unchanged.
    */
    case SQLITE_FCNTL_DB_UNCHANGED: {
      ((unixFile*)id)->dbUpdate = 0;
      return SQLITE_OK;
    }
    case SQLITE_FCNTL_SYNC_OMITTED: {
      return SQLITE_OK; /* A no-op */
    }
  }
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
static int
unixSectorSize(sqlite3_file* NotUsed)
{
  UNUSED_PARAMETER(NotUsed);
  return SQLITE_DEFAULT_SECTOR_SIZE;
}

/*
** Return the device characteristics for the file. This is always 0 for unix.
*/
static int
unixDeviceCharacteristics(sqlite3_file* NotUsed)
{
  UNUSED_PARAMETER(NotUsed);
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
static int
unixOpen(sqlite3_vfs* pVfs,   /* The VFS for which this is the xOpen method */
         const char* zPath,   /* Pathname of file to be opened */
         sqlite3_file* pFile, /* The file descriptor to be filled in */
         int flags,           /* Input flags to control the opening */
         int* pOutFlags       /* Output flags returned to SQLite core */
)
{
  unixFile* p = (unixFile*)pFile;
  int fd = -1;                    /* File descriptor returned by open() */
  int dirfd = -1;                 /* Directory file descriptor */
  int openFlags = 0;              /* Flags to pass to open() */
  int eType = flags & 0xFFFFFF00; /* Type of file to open */
  int noLock;                     /* True to omit locking primitives */
  int rc = SQLITE_OK;             /* Function Return Code */
  int isExclusive = (flags & SQLITE_OPEN_EXCLUSIVE);
  int isDelete = (flags & SQLITE_OPEN_DELETEONCLOSE);
  int isCreate = (flags & SQLITE_OPEN_CREATE);
  int isReadonly = (flags & SQLITE_OPEN_READONLY);
  int isReadWrite = (flags & SQLITE_OPEN_READWRITE);
  /* If creating a master or main-file journal, this function will open
  ** a file-descriptor on the directory too. The first time unixSync()
  ** is called the directory file descriptor will be fsync()ed and close()d.
  */
  int isOpenDirectory = (isCreate && (eType == SQLITE_OPEN_MASTER_JOURNAL ||
                                      eType == SQLITE_OPEN_MAIN_JOURNAL ||
                                      eType == SQLITE_OPEN_WAL));
  /* If argument zPath is a NULL pointer, this function is required to open
  ** a temporary file. Use this buffer to store the file name in.
  */
  char zTmpname[MAX_PATHNAME + 1];
  const char* zName = zPath;
  fprintf(stderr, "trace: begin unixOpen ...\n");
  /* Check the following statements are true:
  **
  **   (a) Exactly one of the READWRITE and READONLY flags must be set, and
  **   (b) if CREATE is set, then READWRITE must also be set, and
  **   (c) if EXCLUSIVE is set, then CREATE must also be set.
  **   (d) if DELETEONCLOSE is set, then CREATE must also be set.
  */
  assert((isReadonly == 0 || isReadWrite == 0) && (isReadWrite || isReadonly));
  assert(isCreate == 0 || isReadWrite);
  assert(isExclusive == 0 || isCreate);
  assert(isDelete == 0 || isCreate);
  /* The main DB, main journal, WAL file and master journal are never
  ** automatically deleted. Nor are they ever temporary files.  */
  assert((!isDelete && zName) || eType != SQLITE_OPEN_MAIN_DB);
  assert((!isDelete && zName) || eType != SQLITE_OPEN_MAIN_JOURNAL);
  assert((!isDelete && zName) || eType != SQLITE_OPEN_MASTER_JOURNAL);
  assert((!isDelete && zName) || eType != SQLITE_OPEN_WAL);
  /* Assert that the upper layer has set one of the "file-type" flags. */
  assert(eType == SQLITE_OPEN_MAIN_DB || eType == SQLITE_OPEN_TEMP_DB ||
         eType == SQLITE_OPEN_MAIN_JOURNAL ||
         eType == SQLITE_OPEN_TEMP_JOURNAL || eType == SQLITE_OPEN_SUBJOURNAL ||
         eType == SQLITE_OPEN_MASTER_JOURNAL ||
         eType == SQLITE_OPEN_TRANSIENT_DB || eType == SQLITE_OPEN_WAL);
  memset(p, 0, sizeof(unixFile));
  if (eType == SQLITE_OPEN_MAIN_DB) {
    struct UnixUnusedFd* pUnused;
    pUnused = findReusableFd(zName, flags);
    if (pUnused) {
      fd = pUnused->fd;
    } else {
      pUnused = sqlite3_malloc(sizeof(*pUnused));
      if (!pUnused) {
        return SQLITE_NOMEM;
      }
    }
    p->pUnused = pUnused;
  } else if (!zName) {
    /* If zName is NULL, the upper layer is requesting a temp file. */
    assert(isDelete && !isOpenDirectory);
    rc = unixGetTempname(MAX_PATHNAME + 1, zTmpname);
    if (rc != SQLITE_OK) {
      return rc;
    }
    zName = zTmpname;
  }
  /* Determine the value of the flags parameter passed to POSIX function
  ** open(). These must be calculated even if open() is not called, as
  ** they may be stored as part of the file handle and used by the
  ** 'conch file' locking functions later on.  */
  if (isReadonly) {
    openFlags |= O_RDONLY;
  }
  if (isReadWrite) {
    openFlags |= O_RDWR;
  }
  if (isCreate) {
    openFlags |= O_CREAT;
  }
  if (isExclusive) {
    openFlags |= (O_EXCL | O_NOFOLLOW);
  }
  openFlags |= (O_LARGEFILE | O_BINARY);
  if (fd < 0) {
    mode_t openMode; /* Permissions to create file with */
    rc = findCreateFileMode(zName, flags, &openMode);
    if (rc != SQLITE_OK) {
      assert(!p->pUnused);
      assert(eType == SQLITE_OPEN_WAL || eType == SQLITE_OPEN_MAIN_JOURNAL);
      return rc;
    }
    fd = robust_open(zName, openFlags, openMode);
    if (fd < 0 && errno != EISDIR && isReadWrite && !isExclusive) {
      /* Failed to open the file for read/write access. Try read-only. */
      flags &= ~(SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
      openFlags &= ~(O_RDWR | O_CREAT);
      flags |= SQLITE_OPEN_READONLY;
      openFlags |= O_RDONLY;
      isReadonly = 1;
      fd = robust_open(zName, openFlags, openMode);
    }
    if (fd < 0) {
      rc = unixLogError(SQLITE_CANTOPEN_BKPT, "open", zName);
      goto open_finished;
    }
  }
  assert(fd >= 0);
  if (pOutFlags) {
    *pOutFlags = flags;
  }
  if (p->pUnused) {
    p->pUnused->fd = fd;
    p->pUnused->flags = flags;
  }
  if (isDelete) {
    unlink(zName);
  }
  if (isOpenDirectory) {
    rc = openDirectory(zPath, &dirfd);
    if (rc != SQLITE_OK) {
      /* It is safe to close fd at this point, because it is guaranteed not
      ** to be open on a database file. If it were open on a database file,
      ** it would not be safe to close as this would release any locks held
      ** on the file by this process.  */
      assert(eType != SQLITE_OPEN_MAIN_DB);
      robust_close(p, fd, __LINE__);
      goto open_finished;
    }
  }
  noLock = eType != SQLITE_OPEN_MAIN_DB;
  rc =
    fillInUnixFile(pVfs, fd, dirfd, pFile, zPath, noLock, isDelete, isReadonly);
open_finished:
  if (rc != SQLITE_OK) {
    sqlite3_free(p->pUnused);
  }
  return rc;
}

/*
** Delete the file at zPath. If the dirSync argument is true, fsync()
** the directory after deleting the file.
*/
static int
unixDelete(sqlite3_vfs* NotUsed, /* VFS containing this as the xDelete method */
           const char* zPath,    /* Name of file to be deleted */
           int dirSync /* If true, fsync() directory after deleting file */
)
{
  int rc = SQLITE_OK;
  UNUSED_PARAMETER(NotUsed);
  if (unlink(zPath) == (-1) && errno != ENOENT) {
    return unixLogError(SQLITE_IOERR_DELETE, "unlink", zPath);
  }
  if (dirSync) {
    int fd;
    rc = openDirectory(zPath, &fd);
    if (rc == SQLITE_OK) {
      if (fsync(fd)) {
        rc = unixLogError(SQLITE_IOERR_DIR_FSYNC, "fsync", zPath);
      }
      robust_close(0, fd, __LINE__);
    }
  }
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
static int
unixAccess(sqlite3_vfs* NotUsed, /* The VFS containing this xAccess method */
           const char* zPath,    /* Path of the file to examine */
           int flags,   /* What do we want to learn about the zPath file? */
           int* pResOut /* Write result boolean here */
)
{
  int amode = 0;
  UNUSED_PARAMETER(NotUsed);
  switch (flags) {
    case SQLITE_ACCESS_EXISTS:
      amode = F_OK;
      break;
    case SQLITE_ACCESS_READWRITE:
      amode = W_OK | R_OK;
      break;
    case SQLITE_ACCESS_READ:
      amode = R_OK;
      break;
    default:
      assert(!"Invalid flags argument");
  }
  *pResOut = (osAccess(zPath, amode) == 0);
  if (flags == SQLITE_ACCESS_EXISTS && *pResOut) {
    struct stat buf;
    if (0 == stat(zPath, &buf) && buf.st_size == 0) {
      *pResOut = 0;
    }
  }
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
static int
unixFullPathname(sqlite3_vfs* pVfs, /* Pointer to vfs object */
                 const char* zPath, /* Possibly relative input path */
                 int nOut,          /* Size of output buffer in bytes */
                 char* zOut         /* Output buffer */
)
{
  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing. This function could fail if, for example, the
  ** current working directory has been unlinked.
  */
  assert(pVfs->mxPathname == MAX_PATHNAME);
  UNUSED_PARAMETER(pVfs);
  zOut[nOut - 1] = '\0';
  if (zPath[0] == '/') {
    sqlite3_snprintf(nOut, zOut, "%s", zPath);
  } else {
    int nCwd;
    if (osGetcwd(zOut, nOut - 1) == 0) {
      return unixLogError(SQLITE_CANTOPEN_BKPT, "getcwd", zPath);
    }
    nCwd = (int)strlen(zOut);
    sqlite3_snprintf(nOut - nCwd, &zOut[nCwd], "/%s", zPath);
  }
  return SQLITE_OK;
}

/*
** Interfaces for opening a shared library, finding entry points
** within the shared library, and closing the shared library.
*/
static void*
unixDlOpen(sqlite3_vfs* NotUsed, const char* zFilename)
{
  UNUSED_PARAMETER(NotUsed);
  return dlopen(zFilename, RTLD_NOW | RTLD_GLOBAL);
}

/*
** SQLite calls this function immediately after a call to unixDlSym() or
** unixDlOpen() fails (returns a null pointer). If a more detailed error
** message is available, it is written to zBufOut. If no error message
** is available, zBufOut is left unmodified and SQLite uses a default
** error message.
*/
static void
unixDlError(sqlite3_vfs* NotUsed, int nBuf, char* zBufOut)
{
  const char* zErr;
  UNUSED_PARAMETER(NotUsed);
  zErr = dlerror();
  if (zErr) {
    sqlite3_snprintf(nBuf, zBufOut, "%s", zErr);
  }
}

static void (*unixDlSym(sqlite3_vfs* NotUsed, void* p, const char* zSym))(void)
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
  void (*(*x)(void*, const char*))(void);
  UNUSED_PARAMETER(NotUsed);
  x = (void (*(*)(void*, const char*))(void))dlsym;
  return (*x)(p, zSym);
}

static void
unixDlClose(sqlite3_vfs* NotUsed, void* pHandle)
{
  UNUSED_PARAMETER(NotUsed);
  dlclose(pHandle);
}

/*
** Write nBuf bytes of random data to the supplied buffer zBuf.
*/
static int
unixRandomness(sqlite3_vfs* NotUsed, int nBuf, char* zBuf)
{
  UNUSED_PARAMETER(NotUsed);
  assert((size_t)nBuf >= (sizeof(time_t) + sizeof(int)));
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
    } else {
      do {
        nBuf = osRead(fd, zBuf, nBuf);
      } while (nBuf < 0 && errno == EINTR);
      robust_close(0, fd, __LINE__);
    }
  }
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
static int
unixSleep(sqlite3_vfs* NotUsed, int microseconds)
{
  usleep(microseconds);
  UNUSED_PARAMETER(NotUsed);
  return microseconds;
}

/*
** Find the current time (in Universal Coordinated Time).  Write the
** current time and date as a Julian Day number into *prNow and
** return 0.  Return 1 if the time and date cannot be found.
*/
static int
unixCurrentTime(sqlite3_vfs* NotUsed, double* prNow)
{
  sqlite3_int64 i;
  UNUSED_PARAMETER(NotUsed);
  unixCurrentTimeInt64(0, &i);
  *prNow = i / 86400000.0;
  return 0;
}

/*
** We added the xGetLastError() method with the intention of providing
** better low-level error messages when operating-system problems come up
** during SQLite operation.  But so far, none of that has been implemented
** in the core.  So this routine is never called.  For now, it is merely
** a place-holder.
*/
static int
unixGetLastError(sqlite3_vfs* NotUsed, int NotUsed2, char* NotUsed3)
{
  UNUSED_PARAMETER(NotUsed);
  UNUSED_PARAMETER(NotUsed2);
  UNUSED_PARAMETER(NotUsed3);
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
static int
unixCurrentTimeInt64(sqlite3_vfs* NotUsed, sqlite3_int64* piNow)
{
  static const sqlite3_int64 unixEpoch = 24405875 * (sqlite3_int64)8640000;
  struct timeval sNow;
  gettimeofday(&sNow, 0);
  *piNow = unixEpoch + 1000 * (sqlite3_int64)sNow.tv_sec + sNow.tv_usec / 1000;
  UNUSED_PARAMETER(NotUsed);
  return 0;
}

/*
** This is the xSetSystemCall() method of sqlite3_vfs for all of the
** "unix" VFSes.  Return SQLITE_OK opon successfully updating the
** system call pointer, or SQLITE_NOTFOUND if there is no configurable
** system call named zName.
*/
static int
unixSetSystemCall(
  sqlite3_vfs* pNotUsed,       /* The VFS pointer.  Not used */
  const char* zName,           /* Name of system call to override */
  sqlite3_syscall_ptr pNewFunc /* Pointer to new system call value */
)
{
  unsigned int i;
  int rc = SQLITE_NOTFOUND;
  UNUSED_PARAMETER(pNotUsed);
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
  } else {
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
  return rc;
}

/*
** Return the value of a system call.  Return NULL if zName is not a
** recognized system call name.  NULL is also returned if the system call
** is currently undefined.
*/
static sqlite3_syscall_ptr
unixGetSystemCall(sqlite3_vfs* pNotUsed, const char* zName)
{
  unsigned int i;
  UNUSED_PARAMETER(pNotUsed);
  for (i = 0; i < sizeof(aSyscall) / sizeof(aSyscall[0]); i++) {
    if (strcmp(zName, aSyscall[i].zName) == 0) {
      return aSyscall[i].pCurrent;
    }
  }
  return 0;
}

/*
** Return the name of the first system call after zName.  If zName==NULL
** then return the name of the first system call.  Return NULL if zName
** is the last system call or if zName is not the name of a valid
** system call.
*/
static const char*
unixNextSystemCall(sqlite3_vfs* p, const char* zName)
{
  int i = -1;
  UNUSED_PARAMETER(p);
  if (zName) {
    for (i = 0; i < ArraySize(aSyscall) - 1; i++) {
      if (strcmp(zName, aSyscall[i].zName) == 0) {
        break;
      }
    }
  }
  for (i++; i < ArraySize(aSyscall); i++) {
    if (aSyscall[i].pCurrent != 0) {
      return aSyscall[i].zName;
    }
  }
  return 0;
}

int
myvfs_init(void)
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
#define UNIXVFS(VFSNAME, FINDER)                                               \
  {                                                                            \
    1,                  /* iVersion */                                         \
      sizeof(unixFile), /* szOsFile */                                         \
      MAX_PATHNAME,     /* mxPathname */                                       \
      0,                /* pNext */                                            \
      VFSNAME,          /* zName */                                            \
      (void*)&FINDER,   /* pAppData */                                         \
      unixOpen,         /* xOpen */                                            \
      unixDelete,       /* xDelete */                                          \
      unixAccess,       /* xAccess */                                          \
      unixFullPathname, /* xFullPathname */                                    \
      unixDlOpen,       /* xDlOpen */                                          \
      unixDlError,      /* xDlError */                                         \
      unixDlSym,        /* xDlSym */                                           \
      unixDlClose,      /* xDlClose */                                         \
      unixRandomness,   /* xRandomness */                                      \
      unixSleep,        /* xSleep */                                           \
      unixCurrentTime,  /* xCurrentTime */                                     \
      unixGetLastError, /* xGetLastError */                                    \
    /* unixCurrentTimeInt64, v2, xCurrentTimeInt64 */                          \
    /* unixSetSystemCall,    v3, xSetSystemCall */                             \
    /* unixGetSystemCall,    v3, xGetSystemCall */                             \
    /* unixNextSystemCall,   v3, xNextSystemCall */                            \
  }
  /*
  ** All default VFSes for unix are contained in the following array.
  **
  ** Note that the sqlite3_vfs.pNext field of the VFS object is modified
  ** by the SQLite core when the VFS is registered.  So the following
  ** array cannot be const.
  */
  static sqlite3_vfs aVfs[] = {
    UNIXVFS("myvfs", nolockIoFinder),
  };
  unsigned int i; /* Loop counter */
  /* Double-check that the aSyscall[] array has been constructed
  ** correctly.  See ticket [bb3a86e890c8e96ab] */
  assert(ArraySize(aSyscall) == 16);
  /* Register all VFSes defined in the aVfs[] array */
  for (i = 0; i < (sizeof(aVfs) / sizeof(sqlite3_vfs)); i++) {
    sqlite3_vfs_register(&aVfs[i], i == 0);
  }
  return SQLITE_OK;
}
