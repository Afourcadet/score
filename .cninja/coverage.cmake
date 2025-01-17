cninja_require(compiler=clang)
cninja_require(lld)
cninja_require(linkerwarnings)
cninja_require(debugsyms)
#cninja_require(debugmode)
cninja_require(debugsplit)
cninja_require(score-warnings)

set_cache(CMAKE_BUILD_TYPE Debug)
set_cache(CMAKE_LINK_DEPENDS_NO_SHARED 1)
set_cache(SCORE_PCH True)
set_cache(SCORE_DYNAMIC_PLUGINS True)

if(UNIX AND NOT APPLE)
  string(APPEND CMAKE_C_FLAGS_INIT " -ggnu-pubnames -fdebug-types-section -fprofile-arcs -ftest-coverage ")
  string(APPEND CMAKE_CXX_FLAGS_INIT " -ggnu-pubnames -fdebug-types-section -fprofile-arcs -ftest-coverage ")
endif()

