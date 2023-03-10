// -------------------------------------------------------------------------------------
//  FeabhOS OS abstraction layer
//
//  DISCLAIMER:
//  Feabhas is furnishing this item "as is". Feabhas does not provide any warranty
//  of the item whatsoever, whether express, implied, or statutory, including, but
//  not limited to, any warranty of merchantability or fitness for a particular
//  purpose or any warranty that the contents of the item will be error-free.
//  In no respect shall Feabhas incur any liability for any damages, including, but
//  limited to, direct, indirect, special, or consequential damages arising out of,
//  resulting from, or any way connected to the use of the item, whether or not
//  based upon warranty, contract, tort, or otherwise; whether or not injury was
//  sustained by persons or property or otherwise; and whether or not loss was
//  sustained from, or arose out of, the results of, the item, or any services that
//  may be provided by Feabhas.
//
// -------------------------------------------------------------------------------------
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


