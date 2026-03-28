#ifndef CUTELOGGER_GLOBAL_HPP
#define CUTELOGGER_GLOBAL_HPP

#include <QtCore/qglobal.h>

// clang-format off
#if defined(CUTELOGGER_LIBRARY)
#  define CUTELOGGERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CUTELOGGERSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // CUTELOGGER_GLOBAL_HPP
// clang-format on