// Trace.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef TRACE_H
#define TRACE_H

#ifdef TRACE_ENABLED
#include <iostream>

#define TRACE_MSG(msg) std::clog << "DEBUG: " << msg << '\n'
#define TRACE_VALUE(variable)                                                  \
  std::clog << "DEBUG: " << #variable << " : " << variable << '\n'

#else
#define TRACE_MSG(msg)
#define TRACE_VALUE(variable)

#endif

#endif
