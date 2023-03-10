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

#include "ReadWriteLock.h"

namespace FeabhOS
{
  ReadWriteLock::ReadWriteLock() :
    handle(0)
  {
    feabhOS_rwlock_create(&handle);
  }


  ReadWriteLock::~ReadWriteLock()
  {
    feabhOS_rwlock_destroy(&handle);
  }


  void ReadWriteLock::readAcquire()
  {
    feabhOS_rwlock_read_acquire(&handle);
  }


  void ReadWriteLock::readRelease()
  {
    feabhOS_rwlock_read_release(&handle);
  }


  void ReadWriteLock::writeAcquire()
  {
    feabhOS_rwlock_write_acquire(&handle);
  }


  void ReadWriteLock::writeRelease()
  {
    feabhOS_rwlock_write_release(&handle);
  }

} // namespace FeabhOS


