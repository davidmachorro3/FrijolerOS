/* This file is derived from source code for the Nachos
   instructional operating system.  The Nachos copyright notice
   is reproduced in full below. */

/* Copyright (c) 1992-1996 The Regents of the University of California.
   All rights reserved.

   Permission to use, copy, modify, and distribute this software
   and its documentation for any purpose, without fee, and
   without written agreement is hereby granted, provided that the
   above copyright notice and the following two paragraphs appear
   in all copies of this software.

   IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO
   ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
   CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE
   AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA
   HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
   BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
   PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
   MODIFICATIONS.
*/

#include "threads/synch.h"
#include <stdio.h>
#include <string.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include <string.h>

/* One semaphore in a list. */
struct semaphore_elem 
  {
    struct list_elem elem;              /* List element. */
    struct semaphore semaphore;         /* This semaphore. */

    struct thread *thread;              /* Thread al que pertenese*/

    
  };


static bool priority_compare(const struct list_elem *a_, const struct list_elem *b_,void *aux UNUSED)
{
  const struct thread *a = list_entry (a_, struct thread, elem);
  const struct thread *b = list_entry (b_, struct thread, elem);
  return a->priority > b->priority;
}

static bool priority_compare_cond(const struct list_elem *a_, const struct list_elem *b_,void *aux UNUSED)
{
  const struct thread *a = (list_entry (a_, struct semaphore_elem, elem))->thread;
  const struct thread *b = (list_entry (b_, struct semaphore_elem, elem))->thread;
  //msg("%d", a->priority > b->priority);
  return a->priority > b->priority;
}

void
donate_lock_pals(struct thread *donator, int priority)
{
  struct list_elem *actual_elem = list_begin(&(donator->participating_locks));
  struct lock_part_taking *temp_lock;
  while(actual_elem != list_end(&(donator->participating_locks)))
  {
    temp_lock = list_entry(actual_elem, struct lock_part_taking, elem);
    if((((temp_lock) -> lock) -> holder) -> priority <= donator ->priority)
    {
      (((temp_lock) -> lock) -> holder) ->priority = priority;
      donate_lock_pals((temp_lock->lock)->holder, priority);
    }
    actual_elem = list_next(actual_elem);

  }
}

/* Initializes semaphore SEMA to VALUE.  A semaphore is a
   nonnegative integer along with two atomic operators for
   manipulating it:

   - down or "P": wait for the value to become positive, then
     decrement it.

   - up or "V": increment the value (and wake up one waiting
     thread, if any). */
void
sema_init (struct semaphore *sema, unsigned value) 
{
  ASSERT (sema != NULL);

  sema->value = value;
  list_init (&sema->waiters);
}

/* Down or "P" operation on a semaphore.  Waits for SEMA's value
   to become positive and then atomically decrements it.

   This function may sleep, so it must not be called within an
   interrupt handler.  This function may be called with
   interrupts disabled, but if it sleeps then the next scheduled
   thread will probably turn interrupts back on. */
void
sema_down (struct semaphore *sema) 
{
  enum intr_level old_level;

  ASSERT (sema != NULL);
  ASSERT (!intr_context ());

  old_level = intr_disable ();
  while (sema->value == 0) 
    {
      list_insert_ordered (&sema->waiters, &thread_current ()->elem, priority_compare, NULL);
      
      thread_block ();
    }
  
  sema->value--;
  
  intr_set_level (old_level);
  
  
}

/* Down or "P" operation on a semaphore, but only if the
   semaphore is not already 0.  Returns true if the semaphore is
   decremented, false otherwise.

   This function may be called from an interrupt handler. */
bool
sema_try_down (struct semaphore *sema) 
{
  enum intr_level old_level;
  bool success;

  ASSERT (sema != NULL);

  old_level = intr_disable ();
  if (sema->value > 0) 
    {
      sema->value--;
      success = true; 
    }
  else
    success = false;
  intr_set_level (old_level);

  return success;
}

/* Up or "V" operation on a semaphore.  Increments SEMA's value
   and wakes up one thread of those waiting for SEMA, if any.

   This function may be called from an interrupt handler. */
void
sema_up (struct semaphore *sema) 
{
  enum intr_level old_level;

  ASSERT (sema != NULL);
  
  old_level = intr_disable ();
  struct thread *hilo = NULL;
  if (!list_empty (&sema->waiters))
  {
    list_sort(&sema->waiters,priority_compare,NULL);
    //msg("- %d -", list_size(&sema->waiters));
    hilo = list_entry (list_pop_front (&sema->waiters),struct thread, elem);
    //OSWALDO THREAD UNBLOCK
   //msg("current: %s, pri: %d", thread_current()->name, thread_current()->priority);
    
    thread_unblock(hilo); 
    //msg("hilo: %s, pri: %d", hilo->name, hilo->priority);
  } 
  sema->value++;
  intr_set_level (old_level);
  //OSWALDO IF YIELD

  if(hilo != NULL && hilo->priority > thread_current()->priority)
  {
    //add_to_waiting_list(0);
    thread_yield();
  }
}

static void sema_test_helper (void *sema_);

/* Self-test for semaphores that makes control "ping-pong"
   between a pair of threads.  Insert calls to printf() to see
   what's going on. */
void
sema_self_test (void) 
{
  struct semaphore sema[2];
  int i;

  printf ("Testing semaphores...");
  sema_init (&sema[0], 0);
  sema_init (&sema[1], 0);
  thread_create ("sema-test", PRI_DEFAULT, sema_test_helper, &sema);
  for (i = 0; i < 10; i++) 
    {
      sema_up (&sema[0]);
      sema_down (&sema[1]);
    }
  printf ("done.\n");
}

/* Thread function used by sema_self_test(). */
static void
sema_test_helper (void *sema_) 
{
  struct semaphore *sema = sema_;
  int i;

  for (i = 0; i < 10; i++) 
    {
      sema_down (&sema[0]);
      sema_up (&sema[1]);
    }
}

/* Initializes LOCK.  A lock can be held by at most a single
   thread at any given time.  Our locks are not "recursive", that
   is, it is an error for the thread currently holding a lock to
   try to acquire that lock.

   A lock is a specialization of a semaphore with an initial
   value of 1.  The difference between a lock and such a
   semaphore is twofold.  First, a semaphore can have a value
   greater than 1, but a lock can only be owned by a single
   thread at a time.  Second, a semaphore does not have an owner,
   meaning that one thread can "down" the semaphore and then
   another one "up" it, but with a lock the same thread must both
   acquire and release it.  When these restrictions prove
   onerous, it's a good sign that a semaphore should be used,
   instead of a lock. */
void
lock_init (struct lock *lock)
{
  ASSERT (lock != NULL);

  lock->holder = NULL;
  sema_init (&lock->semaphore, 1);
}

/* Acquires LOCK, sleeping until it becomes available if
   necessary.  The lock must not already be held by the current
   thread.

   This function may sleep, so it must not be called within an
   interrupt handler.  This function may be called with
   interrupts disabled, but interrupts will be turned back on if
   we need to sleep. */

void lock_acquire (struct lock *lock)
{
  ASSERT (lock != NULL);
  ASSERT (!intr_context ());
  ASSERT (!lock_held_by_current_thread (lock));

  enum intr_level old_level = intr_disable();

  if((lock-> holder) != NULL)
  {

    struct list *colaDelLock = &((lock->semaphore).waiters);
    struct  list_elem *elemento_actual = list_begin(colaDelLock);
    int max_priority = thread_current()->priority;

    while (elemento_actual != list_end(colaDelLock))
    {
      struct thread *threadIteracion = list_entry(elemento_actual,struct thread, elem);
      if(threadIteracion->priority > max_priority)
      {
        max_priority = threadIteracion->priority;
      }
      elemento_actual = list_next(elemento_actual);
    }
    
    if(((lock->holder)->priority) < max_priority)
    { 
       
      //msg("Antes \n%s: old: %d, new: %d\n",(lock->holder)->name, (lock->holder)->old_priority, (lock->holder)->priority);
      /*
      (lock->holder)->old_priority = (lock->holder)->priority;
      (lock->holder)->priority = max_priority;
      (lock->holder)->touched = 1;
      */      

      struct list_elem *actual_elem = list_begin(&((lock->holder)->old_priority_list));
      int found = 0;
      struct old_priority *actual_old;
      struct thread *thread_priority_shared;

      while(actual_elem != list_end(&((lock->holder)->old_priority_list))) {
        actual_old = list_entry(actual_elem, struct old_priority, elem);
        if(actual_old->lock == lock) {
          found = 1;
          break;
        }
        actual_elem = list_next(actual_elem);
      }
      if(!found){
        struct old_priority *new_old = (struct old_priority *)malloc(sizeof(struct old_priority));
        new_old->old_pr = (lock->holder)->priority;
        new_old->lock = lock;
        list_push_back(&((lock->holder)->old_priority_list), &(new_old->elem));
        //msg("Despues \n%s: old: %d, new: %d\n",(lock->holder)->name, new_old->old_pr, max_priority);
        struct lock_part_taking *new_lock = (struct lock_part_taking *)malloc(sizeof(struct lock_part_taking));
        new_lock->lock = lock;
        list_push_back(&(thread_current()->participating_locks), &(new_lock->elem));
      }

      donate_lock_pals(lock->holder, max_priority);

      (lock->holder)->priority = max_priority;
      
      //msg("Despues \n%s: old: %d, new: %d\n",(lock->holder)->name, (lock->holder)->old_priority, (lock->holder)->priority);

    }
    //msg("\nSemaforo %d\n", (lock->semaphore).value);
  }
  
  sema_down (&lock->semaphore);
  //msg("- %d -", list_size(&(lock->semaphore).waiters));
  lock->holder = thread_current ();
  //msg("\nHolde: %s", (lock->holder)->name);
  intr_set_level(old_level);
  
}

/* Tries to acquire LOCK and returns true if successful or false
   on failure.  The lock must not already be held by the current
   thread.

   This function will not sleep, so it may be called within an
   interrupt handler. */
bool
lock_try_acquire (struct lock *lock)
{
  bool success;

  ASSERT (lock != NULL);
  ASSERT (!lock_held_by_current_thread (lock));

  success = sema_try_down (&lock->semaphore);
  if (success)
    lock->holder = thread_current ();
  return success;
}

/* Releases LOCK, which must be owned by the current thread.

   An interrupt handler cannot acquire a lock, so it does not
   make sense to try to release a lock within an interrupt
   handler. */
void
lock_release (struct lock *lock) 
{
  ASSERT (lock != NULL);
  ASSERT (lock_held_by_current_thread (lock));

  enum intr_level old_level;
  old_level  = intr_disable();
  
  struct old_priority *actual_old;
  int found = 0 ;
  int mayor = 0;
  char *name = "main";
  char *name2 = (lock->holder)->name;
  //msg("\n--------Current\nSoy %s\n", thread_current()->name);
  //msg("\nRelease \n%s: old: %d, new: %d\n",thread_current()->name, thread_current()->old_priority, thread_current()->priority);
  if(!strcmp(name, name2) && (lock == -1073505144) && 0){
    msg("Holder: %s, pri: %d, lock: %d", thread_current()->name, (lock->holder)->priority,lock);
  }
  if(lock->holder != NULL && list_size(&((lock->holder)->old_priority_list)) > 0)
  {

    //msg("\n-------Holder\nSoy %s\n", (lock->holder)->name);
    /*
    
    if(((lock->holder)->old_priority) >= 0)
    {
      (lock->holder)->priority = (lock->holder)->old_priority;
      (lock->holder)->old_priority = -1;
      (lock->holder)->touched = 0;
    }
    */
    struct list_elem *actual_elem = list_begin(&((lock->holder)->old_priority_list));
    while(actual_elem != list_end(&((lock->holder)->old_priority_list))){
      actual_old = list_entry(actual_elem, struct old_priority, elem);
      if(actual_old->lock == lock){
        found = 1;
        break;
      }
      actual_elem = list_next(actual_elem);
    }


    struct old_priority *bigger_old;
    struct old_priority *temp_old; 
    actual_elem = list_begin(&((lock->holder)->old_priority_list));
    while(found && actual_elem != list_end(&((lock->holder)->old_priority_list))){
      temp_old = list_entry(actual_elem, struct old_priority, elem);
      if(actual_old->old_pr < temp_old->old_pr && bigger_old->old_pr > actual_old->old_pr){
        if((bigger_old->old_pr - actual_old->old_pr) > (temp_old->old_pr - actual_old->old_pr)){
          bigger_old = temp_old;
          mayor = 1;
        }
      }
      actual_elem = list_next(actual_elem);
    }
    //msg("\n-+%d-\n", actual_old->lock);
    if(found && !mayor){
      //msg("holder of %d: %s, pri: %d, return: %d\n", lock,(lock->holder)->name, (lock->holder)->priority, actual_old->old_pr);
      (lock->holder)->priority = actual_old->old_pr;
      list_remove(&(actual_old->elem));
      free(actual_old);
    }else if(found && mayor){
      //(lock->holder)->priority = actual_old_priority->old_pr;
      //msg("holder of %d: %s, pri: %d, new: %d\n", lock,(lock->holder)->name, bigger_old->old_pr, actual_old->old_pr);
      bigger_old->old_pr = actual_old->old_pr;
      list_remove(&(actual_old->elem));
      free(actual_old);
    }

    if(list_size(&(lock->holder)->old_priority_list) == 0 && (lock->holder)->old_priority != -1 && (lock->holder)->old_priority < (lock->holder)->priority){
      (lock->holder)->priority = (lock->holder)->old_priority;
      (lock->holder)->old_priority = -1;
    }
    
    //msg("\n-+%d-\n", actual_old->lock);
    /*
    if(found){
      (lock->holder)->priority = actual_old->old_pr;
      list_remove(&(actual_old->elem));
      free(actual_old);
    }
    */

  }

  lock->holder = NULL;
  sema_up (&lock->semaphore);
  intr_set_level(old_level);
  
  
}

/* Returns true if the current thread holds LOCK, false
   otherwise.  (Note that testing whether some other thread holds
   a lock would be racy.) */
bool
lock_held_by_current_thread (const struct lock *lock) 
{
  ASSERT (lock != NULL);

  return lock->holder == thread_current ();
}



/* Initializes condition variable COND.  A condition variable
   allows one piece of code to signal a condition and cooperating
   code to receive the signal and act upon it. */
void
cond_init (struct condition *cond)
{
  ASSERT (cond != NULL);

  list_init (&cond->waiters);
}

/* Atomically releases LOCK and waits for COND to be signaled by
   some other piece of code.  After COND is signaled, LOCK is
   reacquired before returning.  LOCK must be held before calling
   this function.

   The monitor implemented by this function is "Mesa" style, not
   "Hoare" style, that is, sending and receiving a signal are not
   an atomic operation.  Thus, typically the caller must recheck
   the condition after the wait completes and, if necessary, wait
   again.

   A given condition variable is associated with only a single
   lock, but one lock may be associated with any number of
   condition variables.  That is, there is a one-to-many mapping
   from locks to condition variables.

   This function may sleep, so it must not be called within an
   interrupt handler.  This function may be called with
   interrupts disabled, but interrupts will be turned back on if
   we need to sleep. */
void
cond_wait (struct condition *cond, struct lock *lock) 
{
  struct semaphore_elem waiter;
  waiter.thread = thread_current();

  ASSERT (cond != NULL);
  ASSERT (lock != NULL);
  ASSERT (!intr_context ());
  ASSERT (lock_held_by_current_thread (lock));
  
  sema_init (&waiter.semaphore, 0);
  list_push_back (&cond->waiters, &waiter.elem);
  //msg("-- Size %d",list_size(&cond->waiters));
  lock_release (lock);
  sema_down (&waiter.semaphore);
  
  lock_acquire (lock);
}

/* If any threads are waiting on COND (protected by LOCK), then
   this function signals one of them to wake up from its wait.
   LOCK must be held before calling this function.

   An interrupt handler cannot acquire a lock, so it does not
   make sense to try to signal a condition variable within an
   interrupt handler. */
void
cond_signal (struct condition *cond, struct lock *lock UNUSED) 
{
  ASSERT (cond != NULL);
  ASSERT (lock != NULL);
  ASSERT (!intr_context ());
  ASSERT (lock_held_by_current_thread (lock));

  if (!list_empty (&cond->waiters))
  {
    list_sort(&cond->waiters,priority_compare_cond, NULL);
    //list_reverse(&cond->waiters);
    struct semaphore_elem *waiter = list_entry(list_pop_front(&cond->waiters),struct semaphore_elem, elem);
    //msg("Current: %s, Pri: %d", thread_current()->name, thread_current()->priority);
    //msg("-- sema V %d", list_size(&waiter->semaphore.waiters));
    sema_up (&waiter->semaphore);
    //msg("-- sema V %d", waiter->semaphore.value);
  }
}

/* Wakes up all threads, if any, waiting on COND (protected by
   LOCK).  LOCK must be held before calling this function.

   An interrupt handler cannot acquire a lock, so it does not
   make sense to try to signal a condition variable within an
   interrupt handler. */
void
cond_broadcast (struct condition *cond, struct lock *lock) 
{
  ASSERT (cond != NULL);
  ASSERT (lock != NULL);

  while (!list_empty (&cond->waiters))
    cond_signal (cond, lock);
}
