/*
 *
 * (C) 2014-20 - ntop.org
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include "ntop_includes.h"

// #define DEBUG_FIFO_QUEUE

FifoQueue::FifoQueue(u_int32_t queue_size) {
  m = new Mutex();
  size = queue_size;
  head = tail = 0;
  cur_items = 0;

  items = (void**)calloc(size, sizeof(void*));
  if(items == NULL) throw 1;
}

/* ******************************************* */

FifoQueue::~FifoQueue() {
  delete m;
  free(items);
}

/* ******************************************* */

bool FifoQueue::enqueue(void *item) {
  bool rv = false;

  if (item == NULL)
    return rv;

  m->lock(__FILE__, __LINE__);

  if(canEnqueue()) {
    items[tail] = item;

    rv = true;
    cur_items++;
#ifdef DEBUG_FIFO_QUEUE
    ntop->getTrace()->traceEvent(TRACE_NORMAL, "Enqueue [pos=%d, new_length=%d]", tail, cur_items);
#endif

    if(++tail >= size)
      tail = 0;
  } else {
#ifdef DEBUG_FIFO_QUEUE
    ntop->getTrace()->traceEvent(TRACE_NORMAL, "Enqueue: queue full [%d items], skipping", cur_items);
#endif
  }

  m->unlock(__FILE__, __LINE__);
  return(rv);
}

/* ******************************************* */

void* FifoQueue::dequeue() {
  void *rv;

  if(!cur_items) {
#ifdef DEBUG_FIFO_QUEUE
    ntop->getTrace()->traceEvent(TRACE_NORMAL, "Dequeue: no items [head=%d, tail=%d]", head, tail);
#endif
    return(NULL);
  }

  m->lock(__FILE__, __LINE__);

  rv = items[head];
  items[head] = NULL;

  cur_items--;

#ifdef DEBUG_FIFO_QUEUE
  ntop->getTrace()->traceEvent(TRACE_NORMAL, "Dequeue: [pos=%d, item=%p, remaining=%d]", head, rv, cur_items);
#endif

  if(++head >= size)
    head = 0;

  m->unlock(__FILE__, __LINE__);
  return(rv);
}

