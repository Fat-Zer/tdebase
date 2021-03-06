#################################################
#
#  (C) 2010-2011 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

include( CheckCSourceCompiles )
include( CheckFunctionExists )

check_c_source_compiles( "
  #define _GNU_SOURCE 1
  #include <sys/socket.h>
  int main(int argc, char *argv[]) { struct ucred red; return 0; } "
  HAVE_STRUCT_UCRED )

check_c_source_runs( "
  #include <grp.h>
  #include <sys/types.h>
  int main() { struct group *grp = getgrnam(\"nogroup\"); if(grp) return 0; return 1; }"
  HAVE_NOGROUP )

if( HAVE_NOGROUP )
  set( nogroup nogroup )
else()
  check_c_source_runs("
  #include <grp.h>
  #include <sys/types.h>
  int main() { struct group *grp = getgrnam(\"nobody\"); if(grp) return 0; return 1; }"
  HAVE_NOBODY )
  if( HAVE_NOBODY )
    set( nogroup nobody )
  else()
    set( nogroup 65534 )
  endif()
endif()

check_function_exists( getpeereid HAVE_GETPEEREID )
check_include_file( "sys/select.h" HAVE_SYS_SELECT_H )
check_include_file( "sys/wait.h" HAVE_SYS_WAIT_H )

if( WITH_SUDO_TDESU_BACKEND )
  set( DEFAULT_SUPER_USER_COMMAND sudo CACHE INTERNAL "" FORCE )
else()
  set( DEFAULT_SUPER_USER_COMMAND su CACHE INTERNAL "" FORCE )
endif()
find_file( TDESU_BACKEND ${DEFAULT_SUPER_USER_COMMAND} PATHS ENV PATH )
if( TDESU_BACKEND-NOTFOUND )
  tde_message_fatal( "${DEFAULT_SUPER_USER_COMMAND} was chosen as tdesu backend, but was not found in path." )
endif( TDESU_BACKEND-NOTFOUND )

add_subdirectory( tdesu )
add_subdirectory( tdesud )
