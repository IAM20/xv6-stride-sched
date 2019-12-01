// Per-CPU state
struct cpu {
  uchar apicid;                // Local APIC ID
  struct context *scheduler;   // swtch() here to enter scheduler
  struct taskstate ts;         // Used by x86 to find stack for interrupt
  struct segdesc gdt[NSEGS];   // x86 global descriptor table
  volatile uint started;       // Has the CPU started?
  int ncli;                    // Depth of pushcli nesting.
  int intena;                  // Were interrupts enabled before pushcli?
  struct proc *proc;           // The process running on this cpu or null
};

extern struct cpu cpus[NCPU];
extern int ncpu;

//PAGEBREAK: 17
// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and allocproc() manipulates it.
struct context {
  uint edi;
  uint esi;
  uint ebx;
  uint ebp;
  uint eip;
};

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

struct stride_info
{
  int stride;           // Stride value of the process
  int tickets;          // Tickets given to the process
  long long pass_value; // Pass value of the process
};

// Per-process state
struct proc {
  uint sz;                     // Size of process memory (bytes)
  pde_t* pgdir;                // Page table
  char *kstack;                // Bottom of kernel stack for this process
  enum procstate state;        // Process state
  int pid;                     // Process ID
  struct proc *parent;         // Parent process
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // swtch() here to run process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)

  /* For runnable queue */
  int is_in_runnable_queue;

  /* For waiting queue */
  struct proc * next;
  struct proc * prev;

  /* stride scheduling */
  struct stride_info stride_info;
};

struct waiting_q {
  uint size;
  struct proc * head;
  struct proc * tail;
};

struct proc *remove_min();
void update_pass_value(struct proc *proc);
void update_min_pass_value();
void assign_min_pass_value(struct proc *proc);
void assign_tickets(int tickets);
void initialize_stride_info(struct proc *proc);

/* For runnable queue */
void heapify(struct proc *queue[], int size, int start_index);
int insert_proc(struct proc *queue[], int size, struct proc *proc);
struct proc *pop(struct proc *queue[], int size);
int delete_proc(struct proc *queue[], int delete_index, int size);

/* For idle queue */
void insert_waiting_queue(struct waiting_q *q, struct proc *p);
void delete_proc_in_wq(struct waiting_q *q, struct proc *p);

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap
