/**
 * This file is part of the kpimutils library.
 *
 * Copyright (C) 2008 Jarosław Staniek <staniek@kde.org>
 * Copyright (C) 2012 Andre Heinecke <aheinecke@intevation.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
/**
  @file
  This file is part of the KDEPIM Utilities library and provides
  static methods for process handling (Windows only at this time).

  @author Jarosław Staniek \<staniek@kde.org\>
*/

//krazy:excludeall=captruefalse,null

#include "processes.h"
using namespace KontactInterface;

#ifdef Q_OS_WIN

#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <signal.h>
#include <unistd.h>

#include <QtCore/QList>
#include <QtCore/QtDebug>

#include <QDebug>

// Copy from kdelibs/kinit/kinit_win.cpp
PSID copySid( PSID from )
{
  if ( !from ) {
    return 0;
  }

  int sidLength = GetLengthSid( from );
  PSID to = (PSID) malloc( sidLength );
  CopySid( sidLength, to, from );
  return to;
}

// Copy from kdelibs/kinit/kinit_win.cpp
static PSID getProcessOwner( HANDLE hProcess )
{
  HANDLE hToken = NULL;
  PSID sid;

  OpenProcessToken( hProcess, TOKEN_READ, &hToken );
  if ( hToken ) {
    DWORD size;
    PTOKEN_USER userStruct;

    // check how much space is needed
    GetTokenInformation( hToken, TokenUser, NULL, 0, &size );
    if ( ERROR_INSUFFICIENT_BUFFER == GetLastError() ) {
      userStruct = reinterpret_cast<PTOKEN_USER>( new BYTE[size] );
      GetTokenInformation( hToken, TokenUser, userStruct, size, &size );

      sid = copySid( userStruct->User.Sid );
      CloseHandle( hToken );
      delete [] userStruct;
      return sid;
    }
  }
  return 0;
}

// Copy from kdelibs/kinit/kinit_win.cpp
static HANDLE getProcessHandle( int processID )
{
  return OpenProcess( SYNCHRONIZE |
                      PROCESS_QUERY_INFORMATION |
                      PROCESS_VM_READ |
                      PROCESS_TERMINATE,
                      false, processID );
}

void KPIMUtils::getProcessesIdForName( const QString &processName, QList<int> &pids )
{
  HANDLE h;
  PROCESSENTRY32 pe32;

  h = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
  if ( h == INVALID_HANDLE_VALUE ) {
    return;
  }

  pe32.dwSize = sizeof( PROCESSENTRY32 ); // Necessary according to MSDN
  if ( !Process32First( h, &pe32 ) ) {
    return;
  }

  pids.clear();

  do {
    if ( QString::fromWCharArray( pe32.szExeFile ) == processName ) {
      PSID user_sid = getProcessOwner( GetCurrentProcess() );
      if ( user_sid ) {
        // Also check that we are the Owner of that process
        HANDLE hProcess = getProcessHandle( pe32.th32ProcessID );
        if ( !hProcess ) {
          continue;
        }

        PSID sid = getProcessOwner( hProcess );
        PSID userSid = getProcessOwner( GetCurrentProcess() );
        if ( !sid || userSid && !EqualSid( userSid, sid ) ) {
          free ( sid );
          continue;
        }
      }
      pids.append( (int)pe32.th32ProcessID );
      qDebug() << "found PID: " << (int)pe32.th32ProcessID;
    }
  } while ( Process32Next( h, &pe32 ) );
  CloseHandle( h );
}

bool KPIMUtils::otherProcessesExist( const QString &processName )
{
  QList<int> pids;
  getProcessesIdForName( processName, pids );
  int myPid = getpid();
  foreach ( int pid, pids ) {
    if ( myPid != pid ) {
//      qDebug() << "Process ID is " << pid;
      return true;
    }
  }
  return false;
}

bool KPIMUtils::killProcesses( const QString &processName )
{
  QList<int> pids;
  getProcessesIdForName( processName, pids );
  if ( pids.empty() ) {
    return true;
  }

  qWarning() << "Killing process \"" << processName << " (pid=" << pids[0] << ")..";
  int overallResult = 0;
  foreach ( int pid, pids ) {
    int result;
    result = kill( pid, SIGTERM );
    if ( result == 0 ) {
      continue;
    }
    result = kill( pid, SIGKILL );
    if ( result != 0 ) {
      overallResult = result;
    }
  }
  return overallResult == 0;
}

struct EnumWindowsStruct
{
  EnumWindowsStruct() : windowId( 0 ) {}
  int pid;
  HWND windowId;
};

BOOL CALLBACK EnumWindowsProc( HWND hwnd, LPARAM lParam )
{
  if ( GetWindowLong( hwnd, GWL_STYLE ) & WS_VISIBLE ) {

    DWORD pidwin;

    GetWindowThreadProcessId( hwnd, &pidwin );
    if ( pidwin == ( (EnumWindowsStruct *)lParam )->pid ) {
      ( (EnumWindowsStruct *)lParam )->windowId = hwnd;
      return FALSE;
    }
  }
  return TRUE;
}

void KPIMUtils::activateWindowForProcess( const QString &executableName )
{
  QList<int> pids;
  KPIMUtils::getProcessesIdForName( executableName, pids );
  int myPid = getpid();
  int foundPid = 0;
  foreach ( int pid, pids ) {
    if ( myPid != pid ) {
      qDebug() << "activateWindowForProcess(): PID to activate:" << pid;
      foundPid = pid;
      break;
    }
  }
  if ( foundPid == 0 ) {
    return;
  }
  EnumWindowsStruct winStruct;
  winStruct.pid = foundPid;
  EnumWindows( EnumWindowsProc, (LPARAM)&winStruct );
  if ( winStruct.windowId == 0 ) {
    return;
  }
  SetForegroundWindow( winStruct.windowId );
}

#endif // Q_OS_WIN
