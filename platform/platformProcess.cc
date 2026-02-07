//-----------------------------------------------------------------------------
// Copyright (c) 2025-2026 korkscript contributors.
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "platform/platform.h"
#include "platform/platformProcess.h"
#include "platform/platformFileIO.h"
#include "platform/threads/thread.h"
#include "platform/threads/mutex.h"
#include "platform/threads/semaphore.h"
#include "core/safeDelete.h"
#include "core/stringTable.h"

#include <filesystem>
namespace fs = std::filesystem;

#include <mutex>

#include "raylib.h"

U32 gCurrentPlatformTime;


namespace Platform
{

void init()
{
   
}

void process()
{
   
}

void shutdown()
{
   
}

void sleep(U32 ms)
{
   
}

void restartInstance()
{
   
}

void postQuitMessage(const U32 in_quitVal)
{
   
}

void forceShutdown(S32 returnValue)
{
   exit(returnValue);
}

StringTableEntry getUserHomeDirectory()
{
   return NULL;
}

StringTableEntry getUserDataDirectory()
{
   return NULL;
}


U32 getTime( void )
{
   time_t tv;
   time( &tv );
   return tv;
}

U32 getVirtualMilliseconds( void )
{
   return gCurrentPlatformTime;
}

U32 getRealMilliseconds( void )
{
   return GetTime();
}

void advanceTime(U32 delta)
{
   gCurrentPlatformTime += delta;
}

void getLocalTime(LocalTime& out)
{
   struct tm *lt;
   time_t tv;
   time( &tv );
   lt = localtime(&tv);

   out.sec = lt->tm_sec;
   out.min = lt->tm_min;
   out.hour = lt->tm_hour;
   out.month = lt->tm_mon;
   out.monthday = lt->tm_mday;
   out.weekday = lt->tm_wday;
   out.year = lt->tm_year;
   out.yearday = lt->tm_yday;
   out.isdst = lt->tm_isdst;
}

S32 compareFileTimes(const FileTime &a, const FileTime &b)
{
   if(a > b)
      return 1;
   if(a < b)
      return -1;
   return 0;
}

/// Math.
float getRandom()
{
   return 3;
}

void outputDebugString(const char *string)
{
   printf("%s\n", string);
}

/// File IO.
StringTableEntry getCurrentDirectory()
{
   return StringTable->insert(fs::current_path().c_str());
}

bool setCurrentDirectory(StringTableEntry newDir)
{
   std::error_code ec;
   fs::current_path(newDir, ec);
   return ec ? true : false;
}

StringTableEntry getExecutableName()
{
   return NULL;
}

StringTableEntry getExecutablePath()
{
   return NULL;
}

bool dumpPath(const char *in_pBasePath, std::vector<FileInfo>& out_rFileVector, S32 recurseDepth)
{
   fs::path rootPath(Platform::getCurrentDirectory());
   fs::path newPath(in_pBasePath);

   // If the path is relative to getCurrentDirectory, simplify it
   if (newPath.is_absolute())
   {
      if (newPath.string().find(rootPath.string()) == 0)
      {
         newPath = newPath.lexically_relative(rootPath);
      }
   }

   // NOTE: depth is simple in this case: 0=/file, 1=/subdir/file, ...
   // If recurseDepth=0, we stop right away
   // If recurseDepth=1, we stop at subdirectory

   //printf("dumpPath(%s)\n", path);
   for (auto p = fs::recursive_directory_iterator(newPath);
             p != fs::recursive_directory_iterator();
           ++p)
   {
      fs::directory_entry entry = *p;

      // dont descend further than depth
      int depth = p.depth();
      if (recurseDepth >= 0 && depth >= recurseDepth) {
         p.disable_recursion_pending();
      }

      if (entry.is_directory())
      {
         if (Platform::isExcludedDirectory(entry.path().filename().c_str()))
         {
            p.disable_recursion_pending();
         }
         continue;
      }

      FileInfo fi;

      // Trim end for directory
      fs::path tpath = entry.path();
      tpath = tpath.remove_filename();//.lexically_relative(rootPath);

      std::string pn = tpath;
      int strip = 0;
      if (pn.size() > 0 && (pn[pn.size()-1] == '/'|| pn[pn.size()-1] == '\\'))
      {
         pn[pn.size()-1] = '\0';
      }

      // Strip off ./ if present
      if (pn.size() > 1)
      {
         if (pn[0] == '.' && (pn[1] == '/' || pn[1] == '\\'))
            strip = 2;
         else if (pn[0] == '.')
            strip = 1;
      }

      fi.pFullPath = StringTable->insert(pn.c_str()+strip);
      fi.pFileName = StringTable->insert(entry.path().filename().c_str());
      fi.fileSize = fs::file_size(entry.path());
      //printf("\t%s:%s\n", fi.pFullPath, fi.pFileName);

      out_rFileVector.push_back(fi);
   }

   return true;
}

bool dumpDirectories( const char *path, std::vector<StringTableEntry> &directoryVector, S32 recurseDepth, bool noBasePath )
{
   fs::path rootPath(Platform::getCurrentDirectory());

   for (auto p = fs::recursive_directory_iterator(path);
             p != fs::recursive_directory_iterator();
           ++p)
   {
      fs::directory_entry entry = *p;

      // dont descend further than depth
      int depth = p.depth();
      if (recurseDepth >= 0 &&
          depth >= recurseDepth) {
         p.disable_recursion_pending();
      }
      
      if (entry.is_directory())
      {
         directoryVector.push_back(StringTable->insert(entry.path().lexically_relative(rootPath).c_str()));
      }
   }

   return true;
}

bool hasSubDirectory( const char *pPath )
{
   fs::path rootPath = fs::path(pPath);

   for (auto p = fs::directory_iterator(rootPath);
             p != fs::directory_iterator();
           ++p)
   {
      fs::directory_entry entry = *p;
      if (entry.is_directory())
         return true;
   }

   return false;
}

bool getFileTimes(const char *filePath, FileTime *createTime, FileTime *modifyTime)
{
   try {
           fs::path thePath(filePath);
           
           // Check if file exists to avoid exceptions
           if (!fs::exists(thePath)) return false;

           auto t1 = fs::last_write_time(thePath);

           // Convert file_clock to system_clock
           // We calculate the offset between the two clocks to get the 'wall clock' time
           auto systemTime = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
               t1 - decltype(t1)::clock::now() + std::chrono::system_clock::now()
           );

           std::time_t cftime = std::chrono::system_clock::to_time_t(systemTime);

           if (createTime)
               *createTime = static_cast<FileTime>(cftime);
           if (modifyTime)
               *modifyTime = static_cast<FileTime>(cftime);

           return true;
       }
       catch (...) {
           // Handle cases like permission denied or file moved during execution
           return false;
       }
}

bool isFile(const char *pFilePath)
{
   return fs::is_regular_file(pFilePath);
}

S32  getFileSize(const char *pFilePath)
{
   return fs::file_size(pFilePath);
}

bool isDirectory(const char *pDirPath)
{
   return fs::is_directory(pDirPath);
}

bool isSubDirectory(const char *pParent, const char *pDir)
{
   return fs::is_directory(fs::path(pParent) / fs::path(pDir));
}

bool createPath(const char *path)
{
   fs::path thePath(path);
   fs::path dir = thePath.parent_path();

   if (dir.empty()) {
       return true;
   }

   std::error_code ec;
   fs::create_directories(dir, ec);
   return !ec;
}

bool fileDelete(const char *name)
{
   fs::path thepath(name);
   return fs::remove(thepath);
}

bool fileRename(const char *oldName, const char *newName)
{
   std::error_code ec;
   fs::path oldPath(oldName);
   fs::path newPath(newName);
   fs::rename(oldPath, newPath, ec);
   return ec ? true : false;
}

bool fileTouch(const char *name)
{
   std::error_code ec;
   fs::path thePath(name);

   // Use the native clock for filesystem timestamps
   auto now = fs::file_time_type::clock::now();
   
   fs::last_write_time(thePath, now, ec);

   return !ec;
}

bool pathCopy(const char *fromName, const char *toName, bool nooverwrite)
{
   std::error_code ec;
   fs::copy(fromName, toName);
   return ec ? false : true;
}

}

