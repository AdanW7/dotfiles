/*
 * iar_clangd_shim.h — IAR-compatibility shim for clangd (gcc/clang backend).
 *
 * Why this exists:
 *   We tell clangd to parse IAR (EWARM) projects as arm-none-eabi-gcc while
 *   still defining __ICCARM__ so the code takes its IAR-specific #if branches.
 *   Those branches pull in IAR's real headers (intrinsics.h, iccarm_builtin.h,
 *   cmsis_iccarm.h, ...) and IAR firmware uses IAR *extended keywords*
 *   (__root, __no_init, __ramfunc, __intrinsic, ...). clang doesn't know those
 *   keywords, so every declaration that uses one errors out
 *   (e.g. "unknown type name '__root'").
 *
 *   Instead of shadowing ~250 IAR builtins with fake prototypes (which would
 *   have to track every IAR release), we neutralize the *keywords* to nothing.
 *   IAR's own declarations (including all __iar_builtin_* prototypes, which are
 *   tagged __ATTRIBUTES == "__intrinsic __nounwind") then parse as ordinary C,
 *   so navigation / completion / indexing work and the red squiggles disappear.
 *
 * How it's used:
 *   Force-included via `-include .../iar_clangd_shim.h` from the IAR block in
 *   the global clangd config.yaml. It is a no-op unless __ICCARM__ is defined,
 *   so it is harmless if it is ever pulled into a non-IAR translation unit.
 *
 * Scope:
 *   This only affects how clangd *parses* code for IDE features. It never
 *   reaches the IAR compiler and has no effect on real builds.
 */
#ifndef IAR_CLANGD_SHIM_H
#define IAR_CLANGD_SHIM_H

#if defined(__ICCARM__)

/*
 * Type / function effect attributes used inside IAR system headers.
 * yvals.h defines __ATTRIBUTES as "__intrinsic __nounwind", so neutralizing
 * these leaf keywords is enough to make iccarm_builtin.h et al. parse.
 */
#ifndef __intrinsic
#define __intrinsic
#endif
#ifndef __nounwind
#define __nounwind
#endif
#ifndef __noreturn
#define __noreturn
#endif

/*
 * IAR object / memory placement attributes that appear as declaration
 * specifiers in firmware and BSP code (e.g. `__root const foo_t bar;`,
 * `__no_init uint8_t buf[N];`, `__ramfunc void isr(void);`).
 */
#ifndef __root
#define __root
#endif
#ifndef __no_init
#define __no_init
#endif
#ifndef __ramfunc
#define __ramfunc
#endif
#ifndef __noinit
#define __noinit
#endif
#ifndef __absolute
#define __absolute
#endif
#ifndef __stackless
#define __stackless
#endif
#ifndef __weak
#define __weak
#endif
/*
 * CMSIS/IAR object-attribute. IAR's DLib_Defaults.h would define __WEAK as
 * `_Pragma("object_attribute = __weak")`; we neutralize it so declarations like
 * `__WEAK __ATTRIBUTES void __iar_Locksyslock_Malloc(void);` in yvals.h parse.
 * NOTE: do NOT pass -D__WEAK on the command line — with no value it becomes 1,
 * turning into a stray integer token in front of the return type.
 */
#ifndef __WEAK
#define __WEAK
#endif

/*
 * Type-qualifier / calling-convention keywords.
 * NOTE: __packed is neutralized for parsing only; clangd's view of struct
 * layout may differ from IAR's, which is fine for IDE features.
 */
#ifndef __packed
#define __packed
#endif
#ifndef __arm
#define __arm
#endif
#ifndef __thumb
#define __thumb
#endif
#ifndef __interwork
#define __interwork
#endif
#ifndef __swi
#define __swi
#endif
#ifndef __irq
#define __irq
#endif
#ifndef __fiq
#define __fiq
#endif
#ifndef __nested
#define __nested
#endif
#ifndef __task
#define __task
#endif
#ifndef __monitor
#define __monitor
#endif

/* printf/scanf format-checking markers on IAR stdio declarations. */
#ifndef __printf_args
#define __printf_args
#endif
#ifndef __scanf_args
#define __scanf_args
#endif

/*
 * Marker on system-register-name parameters of ACLE intrinsics
 * (e.g. `#define __sys_reg __spec_string const char *`).
 */
#ifndef __spec_string
#define __spec_string
#endif

/*
 * Function-like IAR attributes. __constrange(min,max) marks a parameter that
 * must be a compile-time constant in a range; it appears inside intrinsic
 * parameter types (e.g. `#define __cpid unsigned __constrange(0,15)`), so it
 * must expand to nothing rather than be treated as a call.
 */
#ifndef __constrange
#define __constrange(lo, hi)
#endif

/*
 * CMSE (TrustZone) entry/call markers — rare on Cortex-M0+, included so shared
 * headers used by secure/non-secure targets still parse.
 */
#ifndef __cmse_nonsecure_entry
#define __cmse_nonsecure_entry
#endif
#ifndef __cmse_nonsecure_call
#define __cmse_nonsecure_call
#endif

/*
 * IAR compiler-predefined "underlying type" macros.
 *
 * IAR's dlib headers (yvals.h, stdint.h, stddef.h, wchar.h, ...) use macros
 * like `typedef __WCHAR_T_TYPE__ wchar_t;` and expect the ICCARM compiler to
 * predefine them. When clangd parses as gcc/clang they are undefined, which
 * breaks those headers ("unknown type name '__WCHAR_T_TYPE__'").
 *
 * We map each IAR `__X_T_TYPE__` onto the equivalent clang/gcc built-in
 * `__X_TYPE__`, so the underlying types still match the arm-none-eabi ABI.
 * (__JMP_BUF_ELEMENT_TYPE__ is intentionally omitted — setjmp.h defines its
 * own fallback for it.)
 */
#ifndef __WCHAR_T_TYPE__
#define __WCHAR_T_TYPE__    __WCHAR_TYPE__
#endif
#ifndef __SIZE_T_TYPE__
#define __SIZE_T_TYPE__     __SIZE_TYPE__
#endif
#ifndef __PTRDIFF_T_TYPE__
#define __PTRDIFF_T_TYPE__  __PTRDIFF_TYPE__
#endif
#ifndef __INTPTR_T_TYPE__
#define __INTPTR_T_TYPE__   __INTPTR_TYPE__
#endif
#ifndef __UINTPTR_T_TYPE__
#define __UINTPTR_T_TYPE__  __UINTPTR_TYPE__
#endif
#ifndef __INTMAX_T_TYPE__
#define __INTMAX_T_TYPE__   __INTMAX_TYPE__
#endif
#ifndef __UINTMAX_T_TYPE__
#define __UINTMAX_T_TYPE__  __UINTMAX_TYPE__
#endif

/*
 * stdarg: tell IAR's yvals.h that the compiler already provides va_list.
 * Otherwise yvals.h defines its own `struct __va_list __va_list`, which clashes
 * with clang's built-in ARM `__va_list` ("typedef redefinition with different
 * types 'struct __va_list' vs '__builtin_va_list'"). IAR's own stdarg.h already
 * uses __builtin_va_list, so this matches its intent.
 */
#ifndef _VA_DEFINED
#define _VA_DEFINED
#endif
#ifndef _VA_LIST
#define _VA_LIST __builtin_va_list
#endif

#ifndef __INT8_T_TYPE__
#define __INT8_T_TYPE__     __INT8_TYPE__
#endif
#ifndef __INT16_T_TYPE__
#define __INT16_T_TYPE__    __INT16_TYPE__
#endif
#ifndef __INT32_T_TYPE__
#define __INT32_T_TYPE__    __INT32_TYPE__
#endif
#ifndef __INT64_T_TYPE__
#define __INT64_T_TYPE__    __INT64_TYPE__
#endif
#ifndef __UINT8_T_TYPE__
#define __UINT8_T_TYPE__    __UINT8_TYPE__
#endif
#ifndef __UINT16_T_TYPE__
#define __UINT16_T_TYPE__   __UINT16_TYPE__
#endif
#ifndef __UINT32_T_TYPE__
#define __UINT32_T_TYPE__   __UINT32_TYPE__
#endif
#ifndef __UINT64_T_TYPE__
#define __UINT64_T_TYPE__   __UINT64_TYPE__
#endif

#ifndef __INT_LEAST8_T_TYPE__
#define __INT_LEAST8_T_TYPE__    __INT_LEAST8_TYPE__
#endif
#ifndef __INT_LEAST16_T_TYPE__
#define __INT_LEAST16_T_TYPE__   __INT_LEAST16_TYPE__
#endif
#ifndef __INT_LEAST32_T_TYPE__
#define __INT_LEAST32_T_TYPE__   __INT_LEAST32_TYPE__
#endif
#ifndef __INT_LEAST64_T_TYPE__
#define __INT_LEAST64_T_TYPE__   __INT_LEAST64_TYPE__
#endif
#ifndef __UINT_LEAST8_T_TYPE__
#define __UINT_LEAST8_T_TYPE__   __UINT_LEAST8_TYPE__
#endif
#ifndef __UINT_LEAST16_T_TYPE__
#define __UINT_LEAST16_T_TYPE__  __UINT_LEAST16_TYPE__
#endif
#ifndef __UINT_LEAST32_T_TYPE__
#define __UINT_LEAST32_T_TYPE__  __UINT_LEAST32_TYPE__
#endif
#ifndef __UINT_LEAST64_T_TYPE__
#define __UINT_LEAST64_T_TYPE__  __UINT_LEAST64_TYPE__
#endif

#ifndef __INT_FAST8_T_TYPE__
#define __INT_FAST8_T_TYPE__     __INT_FAST8_TYPE__
#endif
#ifndef __INT_FAST16_T_TYPE__
#define __INT_FAST16_T_TYPE__    __INT_FAST16_TYPE__
#endif
#ifndef __INT_FAST32_T_TYPE__
#define __INT_FAST32_T_TYPE__    __INT_FAST32_TYPE__
#endif
#ifndef __INT_FAST64_T_TYPE__
#define __INT_FAST64_T_TYPE__    __INT_FAST64_TYPE__
#endif
#ifndef __UINT_FAST8_T_TYPE__
#define __UINT_FAST8_T_TYPE__    __UINT_FAST8_TYPE__
#endif
#ifndef __UINT_FAST16_T_TYPE__
#define __UINT_FAST16_T_TYPE__   __UINT_FAST16_TYPE__
#endif
#ifndef __UINT_FAST32_T_TYPE__
#define __UINT_FAST32_T_TYPE__   __UINT_FAST32_TYPE__
#endif
#ifndef __UINT_FAST64_T_TYPE__
#define __UINT_FAST64_T_TYPE__   __UINT_FAST64_TYPE__
#endif

#endif /* __ICCARM__ */

#endif /* IAR_CLANGD_SHIM_H */
