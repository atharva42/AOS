#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

struct cpu cpus[NCPU];

struct proc proc[NPROC];

struct proc *initproc;

int nextpid = 1;
struct spinlock pid_lock;

extern void forkret(void);
static void freeproc(struct proc *p);

extern char trampoline[];

// helps ensure that wakeups of wait()ing
// parents are not lost. helps obey the
// memory model when using p->parent.
// must be acquired before any p->lock.
struct spinlock wait_lock;

// MLFQ STUFF
Queue mlfqQueue[5];
void push(Queue *qq, struct proc *element) {
  qq->arr[qq->tail] = element, qq->tail++, qq->size++;
  if (qq->tail == NPROC + 1) qq->tail = 0;
}
void pop(Queue *elements) {
  elements->head++, elements->size--;
  if (elements->head == NPROC + 1) elements->head = 0;
}
struct proc *front(Queue *qq) {
  if (qq->head == qq->tail) return 0;
  return qq->arr[qq->head];
}
void queueEr(Queue *qq, int pid) {
  for (int ind = qq->head; ind != qq->tail; ind = (ind + 1) % (NPROC + 1)) {
    if (qq->arr[ind]->pid == pid) {
      struct proc *temp = qq->arr[ind];
      qq->arr[ind] = qq->arr[(ind + 1) % (NPROC + 1)], qq->arr[(ind + 1) % (NPROC + 1)] = temp;
    }
  }
  qq->tail--, qq->size--;
  if (qq->tail < 0) qq->tail = NPROC;
}
void pinit() {
  for (int i = 0; i < 5; i++) mlfqQueue[i].size = mlfqQueue[i].head = mlfqQueue[i].tail = 0;
}


void proc_mapstacks(pagetable_t kpgtbl)
{
  struct proc *p;
  
  for(p = proc; p < &proc[NPROC]; p++) {
    char *pa = kalloc();
    if(pa == 0)
      panic("kalloc");
    uint64 va = KSTACK((int) (p - proc));
    kvmmap(kpgtbl, va, (uint64)pa, PGSIZE, PTE_R | PTE_W);
  }
}

// initialize the proc table.
void
procinit(void)
{
  struct proc *p;
  
  initlock(&pid_lock, "nextpid");
  initlock(&wait_lock, "wait_lock");
  for(p = proc; p < &proc[NPROC]; p++) {
      initlock(&p->lock, "proc");
      p->state = UNUSED;
      p->kstack = KSTACK((int) (p - proc));
  }
}

// Must be called with interrupts disabled,
// to prevent race with process being moved
// to a different CPU.
int cpuid()
{
  int mark = r_tp();
  return mark;
}

// Interrupts must be disabled.
struct cpu* mycpu(void)
{
  int process_pid = cpuid();
  struct cpu *c = &cpus[process_pid];
  return c;
}

// Return the current struct proc *, or zero if none.
struct proc* myproc(void)
{
  push_off();
  struct cpu *c = mycpu();
  struct proc *p = c->proc;
  pop_off();
  return p;
}

int allocpid()
{
  int pid;
  acquire(&pid_lock);
  pid = nextpid;
  nextpid = nextpid + 1;
  release(&pid_lock);
  return pid;
}

// Look in the process table for an UNUSED proc.
// If found, initialize state required to run in the kernel,
// and return with p->lock held.
// If there are no free procs, or a memory allocation fails, return 0.
static struct proc* allocproc(void)
{
  struct proc *p;


  for(p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if(p->state == UNUSED) {
      goto found;
    } else {
      release(&p->lock);
    }
  }
  return 0;

found:
  p->pid = allocpid();
  p->state = USED;
  p->createTime = ticks;
  p->runTime = p->exitTime = p->runNum = p->level = p->lastRun = p->inQueue = 0;
  p->statPri = 60, p->queueChange = 1 << p->level;
  p->queueEnterTime = ticks;
  for (int i = 0; i < 5; i++)
    p->queue[i] = 0;

  p->cur_ticks = 0;

  // Allocate a trapframe page.
  if((p->trapframe = (struct trapframe *)kalloc()) == 0){
    freeproc(p);
    release(&p->lock);
    return 0;
  }

  // An empty user page table.
  p->pagetable = proc_pagetable(p);
  if(p->pagetable == 0){
    freeproc(p);
    release(&p->lock);
    return 0;
  }

  // Set up new context to start executing at forkret,
  // which returns to user space.
  memset(&p->context, 0, sizeof(p->context));
  p->context.ra = (uint64)forkret;
  p->context.sp = p->kstack + PGSIZE;

  return p;
}

// free a proc structure and the data hanging from it,
// including user pages.
// p->lock must be held.
static void freeproc(struct proc *p)
{
  if(p->trapframe)
    kfree((void*)p->trapframe);
  p->trapframe = 0;
  if(p->pagetable)
    proc_freepagetable(p->pagetable, p->sz);
  p->pagetable = 0;
  p->sz = 0;
  p->pid = 0;
  p->parent = 0;
  p->name[0] = 0;
  p->chan = 0;
  p->killed = 0;
  p->xstate = 0;
  p->state = UNUSED;
}

// Create a user page table for a given process, with no user memory,
// but with trampoline and trapframe pages.
pagetable_t
proc_pagetable(struct proc *p)
{
  pagetable_t pgtable;

  // An empty page table.
  pgtable = uvmcreate();
  if(pgtable == 0)
    return 0;

  // map the trampoline code (for system call return)
  // at the highest user virtual address.
  // only the supervisor uses it, on the way
  // to/from user space, so not PTE_U.
  if(mappages(pgtable, TRAMPOLINE, PGSIZE,
              (uint64)trampoline, PTE_R | PTE_X) < 0){
    uvmfree(pgtable, 0);
    return 0;
  }

  // map the trapframe page just below the trampoline page, for
  // trampoline.S.
  if(mappages(pgtable, TRAPFRAME, PGSIZE,
              (uint64)(p->trapframe), PTE_R | PTE_W) < 0){
    uvmunmap(pgtable, TRAMPOLINE, 1, 0);
    uvmfree(pgtable, 0);
    return 0;
  }

  return pgtable;
}

void proc_freepagetable(pagetable_t pagetable, uint64 sz)
{
  uvmunmap(pagetable, TRAMPOLINE, 1, 0);
  uvmunmap(pagetable, TRAPFRAME, 1, 0);
  uvmfree(pagetable, sz);
}

// a user program that calls exec("/init")
// assembled from ../user/initcode.S
// od -t xC ../user/initcode
uchar initcode[] = {
  0x17, 0x05, 0x00, 0x00, 0x13, 0x05, 0x45, 0x02,
  0x97, 0x05, 0x00, 0x00, 0x93, 0x85, 0x35, 0x02,
  0x93, 0x08, 0x70, 0x00, 0x73, 0x00, 0x00, 0x00,
  0x93, 0x08, 0x20, 0x00, 0x73, 0x00, 0x00, 0x00,
  0xef, 0xf0, 0x9f, 0xff, 0x2f, 0x69, 0x6e, 0x69,
  0x74, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};

// Set up first user process.
void userinit(void)
{
  struct proc *p;

  p = allocproc();
  initproc = p;
  
  // allocate one user page and copy initcode's instructions
  // and data into it.
  uvmfirst(p->pagetable, initcode, sizeof(initcode));
  p->sz = PGSIZE;

  // prepare for the very first "return" from kernel to user.
  p->trapframe->epc = 0;      // user program counter
  p->trapframe->sp = PGSIZE;  // user stack pointer

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  p->state = RUNNABLE;

  release(&p->lock);
}

// Grow or shrink user memory by n bytes.
// Return 0 on success, -1 on failure.
int growproc(int n)
{
  uint64 sz;
  struct proc *p = myproc();

  sz = p->sz;
  if(n > 0){
    if((sz = uvmalloc(p->pagetable, sz, sz + n, PTE_W)) == 0) {
      return -1;
    }
  } else if(n < 0){
    sz = uvmdealloc(p->pagetable, sz, sz + n);
  }
  p->sz = sz;
  return 0;
}

// Create a new process, copying the parent.
// Sets up child kernel stack to return as if from fork() system call.
int fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *p = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy user memory from parent to child.
  if(uvmcopy(p->pagetable, np->pagetable, p->sz) < 0){
    freeproc(np);
    release(&np->lock);
    return -1;
  }
  np->sz = p->sz;

  // copy saved user registers.
  *(np->trapframe) = *(p->trapframe);

  // copy mask for process
  np->mask = p->mask;

  // Cause fork to return 0 in the child.
  np->trapframe->a0 = 0;

  // increment reference counts on open file descriptors.
  for(i = 0; i < NOFILE; i++)
    if(p->ofile[i])
      np->ofile[i] = filedup(p->ofile[i]);
  np->cwd = idup(p->cwd);

  safestrcpy(np->name, p->name, sizeof(p->name));

  pid = np->pid;

  release(&np->lock);

  acquire(&wait_lock);
  np->parent = p;
  release(&wait_lock);

  acquire(&np->lock);
  np->state = RUNNABLE;
  release(&np->lock);

  return pid;
}

// Pass p's abandoned children to init.
// Caller must hold wait_lock.
void reparent(struct proc *p)
{
  struct proc *pp;

  for(pp = proc; pp < &proc[NPROC]; pp++){
    if(pp->parent == p){
      pp->parent = initproc;
      wakeup(initproc);
    }
  }
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait().
void exit(int status)
{
  struct proc *p = myproc();

  if(p == initproc)
    panic("init exiting");
  for(int fd = 0; fd < NOFILE; fd++){
    if(p->ofile[fd]){
      struct file *f = p->ofile[fd];
      fileclose(f);
      p->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(p->cwd);
  end_op();
  p->cwd = 0;

  acquire(&wait_lock);

  // Give any children to init.
  reparent(p);

  // Parent might be sleeping in wait().
  wakeup(p->parent);
  
  acquire(&p->lock);

  p->xstate = status;
  p->state = ZOMBIE;
  p->exitTime = ticks;

  release(&wait_lock);

  // Jump into the scheduler, never to return.
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int wait(uint64 addr)
{
  struct proc *pp;
  int no_of_kids, pid;
  struct proc *p = myproc();

  acquire(&wait_lock);

  for(;;){
    no_of_kids = 0;
    for(pp = proc; pp < &proc[NPROC]; pp++){
      if(pp->parent == p){
        acquire(&pp->lock);

        no_of_kids = 1;
        if(pp->state == ZOMBIE){
          pid = pp->pid;
          if(addr != 0 && copyout(p->pagetable, addr, (char *)&pp->xstate,
                                  sizeof(pp->xstate)) < 0) {
            release(&pp->lock);
            release(&wait_lock);
            return -1;
          }
          freeproc(pp);
          release(&pp->lock);
          release(&wait_lock);
          return pid;
        }
        release(&pp->lock);
      }
    }

    
    if(!no_of_kids || killed(p)){
      release(&wait_lock);
      return -1;
    }
    
    // Wait for a child to exit.
    sleep(p, &wait_lock); 
  }
}

int waitx(uint64 addr, uint *wtime, uint *rtime) {
  struct proc *np;
  int no_of_kids, pid;
  struct proc *p = myproc();

  acquire(&wait_lock);

  for (;;) {
    // Scan through table looking for exited children.
    no_of_kids = 0;
    for (np = proc; np < &proc[NPROC]; np++) {
      if (np->parent == p) {
        // make sure the child isn't still in exit() or swtch().
        acquire(&np->lock);

        no_of_kids = 1;
        if (np->state == ZOMBIE) {
          // Found one.
          pid = np->pid;
          *rtime = np->runTime;
          *wtime = np->exitTime - np->createTime - np->runTime;
          if (addr != 0 && copyout(p->pagetable, addr, (char *)&np->xstate,
                                   sizeof(np->xstate)) < 0) {
            release(&np->lock);
            release(&wait_lock);
            return -1;
          }
          freeproc(np);
          release(&np->lock);
          release(&wait_lock);
          return pid;
        }
        release(&np->lock);
      }
    }
    if (!no_of_kids || p->killed) {
      release(&wait_lock);
      return -1;
    }
    sleep(p, &wait_lock);  // DOC: wait-sleep
  }
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run.
//  - swtch to start running that process.
//  - eventually that process transfers control
//    via swtch back to the scheduler.
void updateTime() {
  struct proc *process;
  for (process = proc; process < &proc[NPROC]; process++) {
    acquire(&process->lock);
    if (process->state == RUNNING) {
      process->runTime++;
#ifdef PBS
      process->lastRun++;
#endif
#ifdef MLFQ
      process->queueChange--;
      process->queue[process->level]++;
#endif
    }
    release(&process->lock);
  }
}

void scheduler(void) {
  struct cpu *c = mycpu();

  c->proc = 0;
#ifdef DEFAULT
  struct proc *p;
  for (;;) {
    intr_on();
    for (p = proc; p < &proc[NPROC]; p++) {
      acquire(&p->lock);
      if (p->state == RUNNABLE) {
        p->state = RUNNING;
        c->proc = p;
        swtch(&c->context, &p->context);
        c->proc = 0;
      }
      release(&p->lock);
    }
  }
#endif

#ifdef FCFS
  struct proc *process;
  while (1) {
    intr_on();  // interrupt
    struct proc *min = 0;
    int choice = 0;
    for (process = proc; process < &proc[NPROC]; process++) {
      if (process->state == RUNNABLE) {
        choice = 1;
        if ((!min) || process->createTime < min->createTime)
          min = process;
      }
    }
    if (!choice) continue;
    acquire(&min->lock);
    if (min->state == RUNNABLE) {
      min->state = RUNNING;
      c->proc = min;
      swtch(&c->context, &min->context);
      c->proc = 0;
    }
    release(&min->lock);
  }
#endif

#ifdef PBS
  struct proc *process;
  while (1) {
    intr_on();
    struct proc *min = 0;
    int choice = 0, lowestdynaP;
    for (process = proc; process < &proc[NPROC]; process++) {
      acquire(&process->lock);
      if (process->state == RUNNABLE) {
        choice = 1;
        int dynaP, sleeptime, niceness;
        if (process->newProc) {
          process->newProc = 0, niceness = 5;
          dynaP = process->statPri - niceness + 5;
          if (dynaP > 100) dynaP = 100;
          if (dynaP < 0) dynaP = 0;
        } else {
          sleeptime = process->exitSched - process->lastSched + process->lastRun;
          niceness = (sleeptime / (sleeptime + process->lastRun)) * 10;
          dynaP = process->statPri - niceness + 5;
          if (dynaP > 100) dynaP = 100;
          if (dynaP < 0) dynaP = 0;
        }
        if (!min || dynaP < lowestdynaP || (dynaP == lowestdynaP && process->runNum < min->runNum))
          min = process, lowestdynaP = dynaP;
        else if ((dynaP == lowestdynaP && process->runNum == min->runNum) && process->createTime < min->createTime)
          min = process, lowestdynaP = dynaP;
      }
      release(&process->lock);
    }
    if (!choice) continue;
    acquire(&min->lock);
    if (min->state == RUNNABLE) {
      min->lastSched = ticks, min->runNum++, min->lastRun = 0, min->state = RUNNING;
      c->proc = min;
      swtch(&c->context, &min->context);
      c->proc = 0;
    }
    release(&min->lock);
  }
#endif

#ifdef MLFQ
  struct proc *process;
  while (1) {
    intr_on();

    process = proc;
    while (process < &proc[NPROC]) {
      if (process->state != RUNNABLE) continue;
      if (ticks - process->queueEnterTime < 128) continue;

      if (process->inQueue) {
        queueEr(&mlfqQueue[process->level], process->pid);
        process->inQueue = 0;
      }
      process->level--;
      if (process->level == -1) {
        process->level++;
      }
      process->queueEnterTime = ticks;
      process++;
    }

    process = proc;
    while (process < &proc[NPROC]) {
      if (process->state != RUNNABLE) continue;
      if (process->inQueue != 0) continue;

      push(&mlfqQueue[process->level], process);
      process->inQueue = 1;
      process++;
    }

    int level = 0;
    while (level - 5) {
      while (mlfqQueue[level].size) {
        process = front(&mlfqQueue[level]);
        pop(&mlfqQueue[level]);
        process->inQueue = 0;
        if (process->state != RUNNABLE) continue;
        process->queueEnterTime = ticks;
        goto end;
      }
      level++;
    }

    process = 0;

  end:;

    for (struct proc *p = proc; p < &proc[NPROC];)
      if (p) {
        acquire(&p->lock);
        p->queueChange = 1 << p->level;
        c->proc = p;
        p->state = RUNNING, p->queueEnterTime = ticks, p->runNum++;
        swtch(&c->context, &p->context);
        c->proc = 0;
        p->queueEnterTime = ticks;
        release(&p->lock);
      }
  }
#endif
}

void sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&p->lock))
    panic("sched p->lock");
  if(mycpu()->noff != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(intr_get())
    panic("sched interruptible");

  intena = mycpu()->intena;
  swtch(&p->context, &mycpu()->context);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void yield(void)
{
  struct proc *p = myproc();
  acquire(&p->lock);
#ifdef PBS
  p->exitSched = ticks;
#endif
  p->state = RUNNABLE;
  sched();
  release(&p->lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch to forkret.
void forkret(void)
{
  static int one = 1;

  // Still holding p->lock from scheduler.
  release(&myproc()->lock);

  if (one) {
    // File system initialization must be run in the context of a
    // regular process (e.g., because it calls sleep), and thus cannot
    // be run from main().
    one = 0;
    fsinit(ROOTDEV);
  }

  usertrapret();
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  // Must acquire p->lock in order to
  // change p->state and then call sched.
  // Once we hold p->lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup locks p->lock),
  // so it's okay to release lk.

  acquire(&p->lock);
  release(lk);
  p->chan = chan;
  p->state = SLEEPING;

  sched();
  p->chan = 0;
  release(&p->lock);
  acquire(lk);
}

// Must be called without any p->lock.
void wakeup(void *chan)
{
  struct proc *p;

  for(p = proc; p < &proc[NPROC]; p++) {
    if(p != myproc()){
      acquire(&p->lock);
      if(p->state == SLEEPING && p->chan == chan) {
        p->state = RUNNABLE;
#ifdef PBS
        p->exitSched = ticks;
#endif
      }
      release(&p->lock);
    }
  }
}

// Kill the process with the given pid.
// The victim won't exit until it tries to return
// to user space (see usertrap() in trap.c).
int kill(int pid)
{
  struct proc *p;

  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);
    if(p->pid == pid){
      p->killed = 1;
      if(p->state == SLEEPING){
        // Wake process from sleep().
        p->state = RUNNABLE;
#ifdef PBS
        p->exitSched = ticks;
#endif
      }
      release(&p->lock);
      return 0;
    }
    release(&p->lock);
  }
  return -1;
}

void setkilled(struct proc *p)
{
  acquire(&p->lock);
  p->killed = 1;
  release(&p->lock);
}

int killed(struct proc *p)
{
  int check_killed;
  acquire(&p->lock);
  check_killed = p->killed;
  release(&p->lock);
  return check_killed;
}

// Copy to either a user address, or kernel address,
// depending on usr_dst.
// Returns 0 on success, -1 on error.
int either_copyout(int user_dst, uint64 dst, void *src, uint64 len)
{
  struct proc *p = myproc();
  if(user_dst){
    return copyout(p->pagetable, dst, src, len);
  } else {
    memmove((char *)dst, src, len);
    return 0;
  }
}

// Copy from either a user address, or kernel address,
// depending on usr_src.
// Returns 0 on success, -1 on error.
int either_copyin(void *dst, int user_src, uint64 src, uint64 len)
{
  struct proc *p = myproc();
  if(user_src){
    return copyin(p->pagetable, dst, src, len);
  } else {
    memmove(dst, (char*)src, len);
    return 0;
  }
}
// void procdump(void)
// {
//   static char *states[] = {
//   [UNUSED]    "unused",
//   [USED]      "used",
//   [SLEEPING]  "sleep ",
//   [RUNNABLE]  "runble",
//   [RUNNING]   "run   ",
//   [ZOMBIE]    "zombie"
//   };
//   struct proc *p;
//   char *state;

//   printf("\n");
//   for(p = proc; p < &proc[NPROC]; p++){
//     if(p->state == UNUSED)
//       continue;
//     if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
//       state = states[p->state];
//     else
//       state = "???";
//     #ifdef MLFQ
//     printf("%d %s %s", p->pid, state, p->name);
//     printf("\n");
//     #endif
//   }
// }

void procdump(void)
{
  static char *states[] = {
      [UNUSED] "unused",
      [SLEEPING] "sleep ",
      [RUNNABLE] "runnable",
      [RUNNING] "running",
      [ZOMBIE] "zombie"};
  struct proc *p;
  char *state;;
  #ifdef PBS
  printf("PID   Priority    State     rtime     wtime     nrun\n");
  #endif
  #ifdef MLFQ
  printf("PID Priority State rtime wtime nrun q0 q1 q2 q3 q4\n");
  #endif
  printf("\n");
  for (p = proc; p < &proc[NPROC]; p++)
  {
    if (p->state == UNUSED)
      continue;
    if (p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    #ifdef DEFAULT
    printf("%d %s %s", p->pid, state, p->name);
    printf("\n");
    #endif
    #ifdef FCFS
    printf("%d %s %s", p->pid, state, p->name);
    printf("\n");
    #endif
    #ifdef PBS
    int dp, niceness, sleeptime;
    if (p->newProc)
        {
          p->newProc = 0;
          niceness = 5;
          dp = p->statPri - niceness + 5;
          if (dp > 100)
            dp = 100;
          if (dp < 0)
            dp = 0;
        }
        else
        {
          sleeptime = p->exitSched - p->lastSched + p->lastRun;
          niceness = (sleeptime / (p->lastRun + sleeptime)) * 10;
          dp = p->statPri - niceness + 5;
          if (dp > 100)
            dp = 100;
          if (dp < 0)
            dp = 0;
        }
    printf("%d  %d  %s   %d   %d    %d\n", p->pid, dp, state, p->runTime, ticks - p->createTime - p->runTime, p->runNum);
    #endif

    #ifdef MLFQ
    int dp, niceness, sleeptime;
    sleeptime = p->exitSched - p->lastSched + p->lastRun;
    niceness = (sleeptime / (p->lastRun + sleeptime)) * 10;
    dp = p->statPri - niceness + 5;
    printf("%d    %d  %s    %d    %d    %d    %d    %d    %d    %d    %d\n", p->pid, dp, state, p->runTime, ticks - p->queueEnterTime, p->runNum, p->queue[0], p->queue[1], p->queue[2], p->queue[3], p->queue[4]);
    #endif
  }
}

int set_priority(int new_priority, int pid) {
  int prev_priority = 0;
  for (struct proc *p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if (p->pid != pid) {
      release(&p->lock);
      continue;
    }
    p->newProc = 1;
    prev_priority = p->statPri;
    p->statPri = new_priority;
    release(&p->lock);
    yield();
    return prev_priority;
  }
  yield();
  return prev_priority;
}
