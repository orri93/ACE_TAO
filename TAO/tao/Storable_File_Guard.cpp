//=============================================================================
/**
 *  @file   Storable_File_Guard.cpp
 *
 *  $Id$
 *
 *  @author Rich Seibel (seibelr@ociweb.com)
 *  @author Byron Harris (harrisb@ociweb.com)
 */
//=============================================================================

#include "tao/Storable_File_Guard.h"
#include "tao/Storable_Base.h"

#include "tao/SystemException.h"

TAO::Storable_File_Guard::
Storable_File_Guard (bool redundant)
  : redundant_(redundant)
  , closed_(1)
{
  ACE_TRACE("TAO::Storable_File_Guard::Storable_File_Guard");
}

void
TAO::Storable_File_Guard::init(const char * mode)
{
  ACE_TRACE("TAO::Storable_File_Guard::init");

  // We only accept a subset of mode argument, check it
  rwflags_ = 0;
  for( unsigned int i = 0; i<ACE_OS::strlen(mode); i++ )
    {
      switch (mode[i])
        {
        case 'r': rwflags_ |= mode_read;
          break;
        case 'w': rwflags_ |= mode_write;
          break;
        case 'c': rwflags_ |= mode_create;
          break;
        default: rwflags_ = -1;
        }
    }
  if( rwflags_ <= 0 )
    {
      errno = EINVAL;
      throw CORBA::PERSIST_STORE();
    }

  // Create the stream
  fl_ = this->create_stream(mode);
  if (redundant_)
    {
      if (fl_->open() != 0)
        {
          delete fl_;
          throw CORBA::PERSIST_STORE();
        }

      // acquire a lock on it
      if (fl_ -> flock(0, 0, 0) != 0)
        {
          fl_->close();
          delete fl_;
          throw CORBA::INTERNAL();
        }

      // now that the file is successfully opened and locked it must be
      // unlocked/closed before we leave this class
      closed_ = 0;

      if ( ! (rwflags_ & mode_create) )
        {
          // Check if our copy is up to date
          if (this->parent_obsolete ())
            {
              this->mark_parent_current ();
              this->create_child ();
            }
        }
    }
  else if ( ! this->is_child_created () || (rwflags_ & mode_write) )
    {
      if (fl_->open() != 0)
        {
          delete fl_;
          throw CORBA::PERSIST_STORE();
        }

      // now that the file is successfully opened
      // unlocked/closed before we leave this class
      closed_ = 0;

      if(! this->is_child_created ())
        {
          this->create_child ();
        }
    }
  else
    {
      // Need to insure that fl_ gets deleted
      delete fl_;
    }
}

bool
TAO::Storable_File_Guard::parent_obsolete (void)
{ // Default implementation uses time to determine
  // if obsolete.
  return (fl_->last_changed () > this->get_parent_last_changed ());
}

void
TAO::Storable_File_Guard::mark_parent_current (void)
{ // Default implementation is to set the last changed
  // time to that of the file lock.
  this->set_parent_last_changed (fl_->last_changed ());
}

void
TAO::Storable_File_Guard::release (void)
{
  ACE_TRACE("TAO::Storable_File_Guard::release");
  if ( ! closed_ )
    {
      // If we updated the disk, save the time stamp
      if(redundant_)
        {
          if( rwflags_ & mode_write )
            this->mark_parent_current ();
          fl_->funlock(0, 0, 0);
        }
      fl_->close();
      delete fl_;
      closed_ = 1;
    }
}

TAO::Storable_File_Guard::
~Storable_File_Guard ()
{
  ACE_TRACE("TAO::Storable_File_Guard::~Storable_File_Guard");
}
