/* this file exists to quell stupid MSVC warnings */

#ifdef _WIN32
/* non-constant aggregate initializers */
#pragma warning(disable:4204)
/* anonymous struct/unions */
#pragma warning(disable:4201)
/* padding bytes are added */
#pragma warning(disable:4324)
#pragma warning(disable:4820)
/* unknown preprocessor macros */
#pragma warning(disable:4668)
/* assignment within conditional expression */
#pragma warning(disable:4706)
/* initialization using address of automatic variable */
#pragma warning(disable:4221)
#endif
