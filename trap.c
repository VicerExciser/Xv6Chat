#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"
#include "e1000.h"
#include "lwip/tcp.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;


void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(proc->killed)
      exit();
    proc->tf = tf;
    syscall();
    if(proc->killed)
      exit();
    return;
  }

  if (tf->trapno == (T_IRQ0 + (int)e1000.irq)) {
    //cprintf("trapno=%d\n", (T_IRQ0 + (int)e1000.irq));
    e1000_intr(tf);
    lapiceoi();
    goto exit_trap;
  }

  // Increment nlock to make sure interrupts stay off
  // during interrupt handler.  Decrement before returning.
  // cpus[cpu()].nlock++;

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpunum() == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }

    if (ticks % 25 == 0) tcp_tmr();
    if (ticks % 50 == 0) e1000_intr(tf);  // Check NIC interrupt (again, in case first check above fell thru)

    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_ETH:   // For Ethernet interrupt requests
    cprintf("E1000 interrupt [#2]! (trapno=%d)\n", (int)(T_IRQ0 + IRQ_ETH));
    e1000_intr(tf);
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpunum(), tf->cs, tf->eip);
    lapiceoi();
    break;

  case T_PGFLT:
  {
    // ADDED FOR LAB 5 -- E1000 NEEDS TO BE MEMORY MAPPED FOR NEW PROCESSES
    if (proc != 0 && rcr2() >= (uint)e1000.va
                  && rcr2() <= (uint)(e1000.va + e1000.size))
    {
      // cprintf("(T_PGFLT handler mapping E1000 for user proc)\n");
      mmio_map_region(e1000.pa, e1000.size);
      break;
    }

    if (proc == 0 || ((tf->cs&3) == 0)) 
    {
      // In kernel, it must be our mistake.
      cprintf("unexpected PGFLT on cpu %d eip %x (cr2=0x%x) err=%d\n",
              cpunum(), tf->eip, rcr2(), tf->err);
      panic("PGFLT trap");
    }

    cprintf("pid %d %s: PGFLT va 0x%x out of range! err=%d on cpu %d "
          "eip 0x%x addr 0x%x\n",
          proc->pid, proc->name, rcr2(), tf->err, cpunum(),
          tf->eip, rcr2());
    proc->killed = 1;
    break;
  }

  //PAGEBREAK: 13
  default:
    if(proc == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d err %d on cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, tf->err, cpunum(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            proc->pid, proc->name, tf->trapno, tf->err, cpunum(), tf->eip,
            rcr2());
    proc->killed = 1;
  }

  // cpus[cpu()].nlock--;

exit_trap:
  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(proc && proc->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(proc && proc->state == RUNNING && tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(proc && proc->killed && (tf->cs&3) == DPL_USER)
    exit();
}
