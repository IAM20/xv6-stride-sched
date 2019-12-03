//
// Created by Lee on 2019-12-03.
//
#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"

#define NULL 0
#define TRUE 1
#define FALSE 0

/* For runnable queue (Priority queue) */
void
heapify(struct proc *queue[], int size, int start_index)
{
  if (size == 0) {
    return;
  }
  if (size == NPROC) {
    panic("Heap size is full\n");
  }

  int i, left, right, min;
  for (i = start_index; i <= size; i = min) {
    left = i * 2;
    right = i * 2 + 1;
    min = i;

    if (left <= size && queue[left]->stride_info.pass_value < queue[min]->stride_info.pass_value) {
      min = left;
    }

    if (right <= size && queue[right]->stride_info.pass_value < queue[min]->stride_info.pass_value) {
      min = right;
    }

    if (i != min) {
      struct proc *tmp = queue[i];
      queue[i] = queue[min];
      queue[min] = tmp;
    } else {
      break;
    }
  }
}

int
insert_proc(struct proc *queue[], int size, struct proc *proc) {
  if (proc->is_in_runnable_queue == TRUE) {
    return size;
  }
  proc->is_in_runnable_queue = TRUE;
  queue[++size] = proc;
  int i = size;
  while (i > 1) {
    int parent = i / 2;
    if (queue[i]->stride_info.pass_value < queue[parent]->stride_info.pass_value) {
      struct proc *tmp = queue[parent];
      queue[parent] = queue[i];
      queue[i] = tmp;
    } else {
      break;
    }
    i = parent;
  }
  return size;
}

struct proc *
pop(struct proc *queue[], int size) {
  if (size == 0) {
    return NULL;
  }
  struct proc *result = queue[1];
  result->is_in_runnable_queue = FALSE;
  delete_proc(queue, 1, size);
  return result;
}

int
delete_proc(struct proc *queue[], int delete_index, int size)
{
  if (size == 0 || delete_index > size || delete_index == 0) {
    return 0;
  }

  queue[delete_index] = queue[size];
  queue[size] = NULL;
  heapify(queue, --size, delete_index);

  return size;
}

/* For idle queue */
void
insert_waiting_queue(struct waiting_q *q, struct proc *p)
{
  if (p->next != NULL) {
    return;
  }

  if (q->size == 0) {
    q->head = q->tail = p;
    p->next = p->prev = p;
    q->size++;
    return;
  }

  q->tail->next = p;
  p->next = q->head;
  p->prev = q->tail;
  q->head->prev = p;

  q->tail = p;
  q->size++;
}

void
delete_proc_in_wq(struct waiting_q *q, struct proc *p)
{
  if (p->next == NULL) {
    return;
  }

  if (q->size == 1) {
    q->tail = q->head = NULL;
    p->next = p->prev = NULL;
    q->size = 0;
    return;
  }

  p->prev->next = p->next;
  p->next->prev = p->prev;

  if (q->head == p) {
    q->head = p->next;
  } else if (q->tail == p) {
    q->tail = p->prev;
  }

  p->prev = p->next = NULL;
  q->size--;
}
