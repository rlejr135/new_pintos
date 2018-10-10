#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "userprog/process.h"

void _valid_addr(const void *vaddr){
	struct thread *cur = thread_current();
    if (vaddr == NULL){
        exit(-1);
    }
    if (is_kernel_vaddr(vaddr)){
        exit(-1);
	}
	if (pagedir_get_page(cur->pagedir, vaddr) == NULL){
		exit(-1);
	}
 
}

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f ) 
{
  switch(*(uintptr_t*)(f->esp)){
      case SYS_HALT:
          halt();
          break;

      case SYS_EXIT:
          _valid_addr(f->esp + 4);
          exit(*(int*)(f->esp + 4));
          break;
          
      case SYS_EXEC:
          _valid_addr(f->esp + 4);
          f->eax = exec(*(uintptr_t*)(f->esp + 4));
          break;

      case SYS_WAIT:
		  _valid_addr(f->esp + 4);
          f->eax = wait(*(pid_t*)(f->esp + 4));
          break;

      case SYS_READ:
          _valid_addr(f->esp + 4);
          _valid_addr(f->esp + 8);
          _valid_addr(f->esp + 12);
          f->eax = read(*(int*)(f->esp + 4), *(uintptr_t*)(f->esp + 8), *(unsigned*)(f->esp + 12));
          break;

      case SYS_WRITE:
          _valid_addr(f->esp + 4);
          _valid_addr(f->esp + 8);
          _valid_addr(f->esp + 12);
          f->eax = write(*(int*)(f->esp + 4), *(uintptr_t*)(f->esp + 8), *(unsigned*)(f->esp + 12));
          break;

      case SYS_FIBO:
		  _valid_addr(f->esp + 4);
		  f->eax = fibonacci(*(int*)(f->esp + 4));
		  break;

	  case SYS_SUM4:
		  _valid_addr(f->esp + 4);
		  _valid_addr(f->esp + 8);
		  _valid_addr(f->esp + 12);
		  _valid_addr(f->esp + 16);
		  f->eax = sum_of_four_integers(*(int*)(f->esp + 4), *(int*)(f->esp + 8), *(int*)(f->esp + 12), *(int*)(f->esp + 16));
		  break;
  }
}

void halt(void){
    shutdown_power_off();
}

void exit(int status){
    printf("%s: exit(%d)\n", thread_name(), status);
	thread_current()->exit_status = status;
    thread_exit();
}

pid_t exec(const char *cmd_line){
    return process_execute(cmd_line);
}

int wait(pid_t pid){
	return process_wait(pid);
}

int read(int fd, void *buffer, unsigned size){
    int i;
    if (fd == 0){
        return input_getc();
    }
    else if (fd == 1){
        for (i = 0; i < size; ++i){
           if(((char*)buffer)[i] == '\0')
               break;
        }
        return i;
    }
    else{
        return -1;
    }
}

int write(int fd, const void *buffer, unsigned size){
    if(fd == 1){
        putbuf(buffer, size);
        return size;
    }
    return -1;
}

int fibonacci(int n){
	int n3 = 0, n2 = 1, n1 = 1;
	int i;

	if(n < 3)
		return 1;

	for(i = 2; i < n; ++i){
		n3 = (n2 + n1);
		n1 = n2;
		n2 = n3;
	}

	return n3;
}

int sum_of_four_integers(int a, int b, int c, int d){
	return a + b + c + d;
}
