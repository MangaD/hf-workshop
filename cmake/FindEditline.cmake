# - Try to find editline include dirs and libraries 
#
# Usage of this module as follows:
#
#     find_package(Editline)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  Editline_ROOT_DIR         Set this variable to the root installation of
#                            readline if the module has problems finding the
#                            proper installation path.
#
# Variables defined by this module:
#
#  EDITLINE_FOUND            System has editline, include and lib dirs found
#  Editline_INCLUDE_DIR      The editline include directories. 
#  Editline_LIBRARY          The editline library.

find_path(Editline_ROOT_DIR
    NAMES include/editline/readline.h
)

find_path(Editline_INCLUDE_DIR
    NAMES editline/readline.h
    HINTS ${Editline_ROOT_DIR}/include
)

find_library(Editline_LIBRARY
    NAMES edit
    HINTS ${Editline_ROOT_DIR}/lib
)

if(Editline_INCLUDE_DIR AND Editline_LIBRARY AND Ncurses_LIBRARY)
  set(EDITLINE_FOUND TRUE)
else(Editline_INCLUDE_DIR AND Editline_LIBRARY AND Ncurses_LIBRARY)
  FIND_LIBRARY(Editline_LIBRARY NAMES editline)
  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(Editline DEFAULT_MSG Editline_INCLUDE_DIR Editline_LIBRARY )
  MARK_AS_ADVANCED(Editline_INCLUDE_DIR Editline_LIBRARY)
endif(Editline_INCLUDE_DIR AND Editline_LIBRARY AND Ncurses_LIBRARY)

mark_as_advanced(
    Editline_ROOT_DIR
    Editline_INCLUDE_DIR
    Editline_LIBRARY
)
