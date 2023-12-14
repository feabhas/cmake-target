// feabhOS_error.c
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#include "feabhOS_errors.h"

static const char* error_strings[] =
{
  "Success",
  "Memory cannot be allocated for the OS construct",
  "The FeabhOS OS-construct is invalid (null)",
  "First parameter (NOT the handle) is invalid",
  "Second parameter is invalid",
  "Third parameter is invalid",
  "Fourth parameter is invalid",
  "An invalid call has been made",
  "A non-FeabhOS error has occurred"
  "A blocking call has timed out"
  "No data on message queue",
  "Message queue is full"
};

const char* feabhOS_error_as_string(feabhOS_error err)
{
  return error_strings[err];
}


