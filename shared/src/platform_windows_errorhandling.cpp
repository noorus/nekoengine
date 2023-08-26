#include "pch.h"

#ifdef NEKO_PLATFORM_WINDOWS

#include "neko_types.h"
#include "neko_exception.h"
#include "neko_platform.h"

namespace neko {

  namespace platform {

    // clang-format off

    static const map<DWORD, neko::utf8String> c_exceptionTitlesMap = {
      { EXCEPTION_ACCESS_VIOLATION, "Access violation" },
      { EXCEPTION_IN_PAGE_ERROR, "Page unavailable" },
      { EXCEPTION_ARRAY_BOUNDS_EXCEEDED, "Array bounds exceeded" },
      { EXCEPTION_BREAKPOINT, "Breakpoint hit" },
      { EXCEPTION_DATATYPE_MISALIGNMENT, "Datatype misalignment" },
      { EXCEPTION_FLT_DENORMAL_OPERAND, "Denormal floating point operand" },
      { EXCEPTION_FLT_DIVIDE_BY_ZERO, "Floating point division by zero" },
      { EXCEPTION_FLT_INEXACT_RESULT, "Inexact floating point result" },
      { EXCEPTION_FLT_INVALID_OPERATION, "Floating point exception" },
      { EXCEPTION_FLT_OVERFLOW, "Floating point overflow" },
      { EXCEPTION_FLT_STACK_CHECK, "Floating point stack corruption" },
      { EXCEPTION_FLT_UNDERFLOW, "Floating point underflow" },
      { EXCEPTION_ILLEGAL_INSTRUCTION, "Illegal instruction" },
      { EXCEPTION_INT_DIVIDE_BY_ZERO, "Integer division by zero" },
      { EXCEPTION_INT_OVERFLOW, "Integer overflow" },
      { EXCEPTION_INVALID_DISPOSITION, "Invalid disposition" },
      { EXCEPTION_NONCONTINUABLE_EXCEPTION, "Noncontinuable exception" },
      { EXCEPTION_PRIV_INSTRUCTION, "Privileged instruction" },
      { EXCEPTION_SINGLE_STEP, "Single step hit" },
      { EXCEPTION_STACK_OVERFLOW, "Stack overflow" },
      { EXCEPTION_GUARD_PAGE, "Guard page violation" },
      { EXCEPTION_INVALID_HANDLE, "Invalid handle" },
      { EXCEPTION_POSSIBLE_DEADLOCK, "Possible deadlock" }
    };

    static const map<DWORD, neko::utf8String> c_exceptionDescriptionsMap = {
      { EXCEPTION_ARRAY_BOUNDS_EXCEEDED, "The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking." },
      { EXCEPTION_BREAKPOINT, "The thread hit a breakpoint, but no debugger was attached to catch it." },
      { EXCEPTION_DATATYPE_MISALIGNMENT, "The thread tried to read or write data that is misaligned on hardware that does not provide alignment." },
      { EXCEPTION_FLT_DENORMAL_OPERAND, "One of the operands in a floating-point operation is denormal." },
      { EXCEPTION_FLT_DIVIDE_BY_ZERO, "The thread tried to divide a floating-point value by a floating-point divisor of zero." },
      { EXCEPTION_FLT_INEXACT_RESULT, "The result of a floating-point operation cannot be represented exactly as a decimal fraction." },
      { EXCEPTION_FLT_INVALID_OPERATION, "An unspecified error occurred in a floating point operation." },
      { EXCEPTION_FLT_OVERFLOW, "The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type." },
      { EXCEPTION_FLT_STACK_CHECK, "The stack overflowed or underflowed as the result of a floating-point operation." },
      { EXCEPTION_FLT_UNDERFLOW, "The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type." },
      { EXCEPTION_ILLEGAL_INSTRUCTION, "The thread tried to execute an invalid instruction." },
      { EXCEPTION_INT_DIVIDE_BY_ZERO, "The thread tried to divide an integer value by an integer divisor of zero." },
      { EXCEPTION_INT_OVERFLOW, "The result of an integer operation caused a carry out of the most significant bit of the result." },
      { EXCEPTION_INVALID_DISPOSITION, "An exception handler returned an invalid disposition to the exception dispatcher." },
      { EXCEPTION_NONCONTINUABLE_EXCEPTION, "The thread tried to continue execution after a noncontinuable exception." },
      { EXCEPTION_PRIV_INSTRUCTION, "The thread tried to execute an instruction whose operation is not allowed in the current machine mode." },
      { EXCEPTION_SINGLE_STEP, "A trace trap or other single-instruction mechanism signaled that one instruction has been executed, but no debugger was attached to catch it." },
      { EXCEPTION_STACK_OVERFLOW, "The thread used up its stack." },
      { EXCEPTION_GUARD_PAGE, "The thread attempted to access an address within a guard page." },
      { EXCEPTION_INVALID_HANDLE, "An invalid handle was specified." },
      { EXCEPTION_POSSIBLE_DEADLOCK, "A wait operation on a critical section timed out." },
      { 0xe06d7363, "A C++ exception was not handled by user code." },
      { STATUS_NO_MEMORY, "Not enough virtual memory is available to complete an operation." },
      { STATUS_STACK_BUFFER_OVERRUN, "The system detected an overrun of a stack-based buffer in the application." },
      { STATUS_INVALID_CRUNTIME_PARAMETER, "An invalid parameter was passed to a C runtime function." }
    };

    // clang-format on

    utf8String windowsExceptionMessage( HANDLE thrd, EXCEPTION_POINTERS* ptrs )
    {
      if ( !ptrs || !ptrs->ExceptionRecord )
        return {};

      auto code = ptrs->ExceptionRecord->ExceptionCode;
      auto info = ptrs->ExceptionRecord->ExceptionInformation;

      static thread_local utf8String title( "Unknown exception" );
      if ( c_exceptionTitlesMap.find( code ) != c_exceptionTitlesMap.end() )
        title = c_exceptionTitlesMap.at( code );

      static thread_local utf8String desc( 512, '\0' );
      if ( code == EXCEPTION_ACCESS_VIOLATION )
      {
        if ( info[0] == 0 )
          StringCchPrintfA(
            desc.data(), desc.size(), "The thread attempted to read from invalid address 0x%08llx.", (DWORD64)info[1] );
        else if ( info[0] == 1 )
          StringCchPrintfA(
            desc.data(), desc.size(), "The thread attempted to write to invalid address 0x%08llx.", (DWORD64)info[1] );
        else if ( info[0] == 8 )
          StringCchPrintfA( desc.data(), desc.size(), "The thread caused a DEP violation at 0x%08llx.", (DWORD64)info[1] );
        else
          desc =
            "The thread attempted to read from or write to a virtual address for which it does not have the appropriate access.";
      }
      else if ( code == EXCEPTION_IN_PAGE_ERROR )
      {
        if ( info[0] == 0 || info[0] == 1 )
          StringCchPrintfA( desc.data(), desc.size(),
            "The thread attempted to %s memory at 0x%08llx, but the system was unable to load the page. Error code: 0x%08llx.",
            info[0] == 0 ? "read" : "write", (DWORD64)info[1], (DWORD64)info[2] );
        else if ( info[0] == 8 )
          StringCchPrintfA( desc.data(), desc.size(), "The thread caused a DEP violation at 0x%08llx.", (DWORD64)info[1] );
        else
          desc = "The thread attempted to access a page that was not present, and the system was unable to load the page.";
      }
      else if ( c_exceptionDescriptionsMap.find( code ) != c_exceptionDescriptionsMap.end() )
        desc = c_exceptionDescriptionsMap.at( code );
      else
        desc = "An unknown exception occurred.";

      static thread_local char out[2048];
      StringCchPrintfA( out, 2048, "Type: %s\nThread: %s\nAddress: %p\nCode: 0x%8.8X | Flags: 0x%X\nDescription: %s",
        title.c_str(), getThreadDescriptor( thrd ).c_str(), ptrs->ExceptionRecord->ExceptionAddress, code,
        ptrs->ExceptionRecord->ExceptionFlags, desc.c_str() );

      return out;
    }

    utf8String windowsExceptionMessage( HANDLE thrd, const Exception& e )
    {
      static thread_local char out[2048];
      StringCchPrintfA( out, 2048, "Type: %s\nThread: %s\nDescription: %s", typeid( e ).name(),
        getThreadDescriptor( thrd ).c_str(), e.getFullDescription().c_str() );

      return out;
    }

    utf8String windowsExceptionMessage( HANDLE thrd, const std::exception& e )
    {
      static thread_local char out[2048];
      StringCchPrintfA( out, 2048, "Type: %s\nThread: %s\nDescription: %s", typeid( e ).name(),
        getThreadDescriptor( thrd ).c_str(), e.what() );

      return out;
    }

    // SinkedStackWalker implementation

    void SinkedStackWalker::OnOutput( LPCSTR txt )
    {
      //printf( "%s\r\n", txt );
      sink_.append( txt );
    }

    void SinkedStackWalker::OnSymInit( LPCSTR szSearchPath, DWORD symOptions, LPCSTR szUserName )
    {
      // Don't care
    }

    void SinkedStackWalker::OnDbgHelpErr( LPCSTR szFuncName, DWORD gle, DWORD64 addr )
    {
      // Don't care
    }

    void SinkedStackWalker::OnLoadModule( LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size, DWORD result, LPCSTR symType,
      LPCSTR pdbName,
      ULONGLONG fileVersion )
    {
      // Don't care
    }

    void SinkedStackWalker::OnCallstackEntry( CallstackEntryType eType, CallstackEntry& entry )
    {
      static thread_local char buf[STACKWALK_MAX_NAMELEN];
      if ( ( eType != lastEntry ) && ( entry.offset != 0 ) )
      {
        if ( entry.name[0] == 0 )
          StringCchCopyNA( entry.name, STACKWALK_MAX_NAMELEN, "(unknown function)", static_cast<size_t>( STACKWALK_MAX_NAMELEN ) - 1 );
        if ( entry.undName[0] != 0 )
          StringCchCopyNA( entry.name, STACKWALK_MAX_NAMELEN, entry.undName, static_cast<size_t>( STACKWALK_MAX_NAMELEN ) - 1 );
        if ( entry.undFullName[0] != 0 )
          StringCchCopyNA( entry.name, STACKWALK_MAX_NAMELEN, entry.undFullName, static_cast<size_t>( STACKWALK_MAX_NAMELEN ) - 1 );
        if ( entry.lineFileName[0] == 0 )
        {
          StringCchCopyNA( entry.lineFileName, STACKWALK_MAX_NAMELEN, "(unknown file)", static_cast<size_t>( STACKWALK_MAX_NAMELEN ) - 1 );
          if ( entry.moduleName[0] == 0 )
            StringCchCopyNA( entry.moduleName, STACKWALK_MAX_NAMELEN, "(unknown module)", static_cast<size_t>( STACKWALK_MAX_NAMELEN ) - 1 );
          StringCchPrintfA(
            buf, STACKWALK_MAX_NAMELEN, "%p (%s): %s\n", (LPVOID)entry.offset, entry.moduleName, entry.name );
        }
        else
          StringCchPrintfA( buf, STACKWALK_MAX_NAMELEN, "%p (%s): %s (%s line %d)\n", (LPVOID)entry.offset, entry.moduleName,
            entry.name, entry.lineFileName, entry.lineNumber );
        buf[STACKWALK_MAX_NAMELEN - 1] = 0;
        OnOutput( buf );
      }
    }
  }

}

#endif