#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>

#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"

#include "devices/shutdown.h"

#include "userprog/process.h"

#include "filesys/filesys.h"
#include "filesys/file.h"

#include "devices/input.h"


struct lock *filelock;

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
  lock_init(&filelock);
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

	  case SYS_CREATE:
		  _valid_addr(f->esp + 4);
		  _valid_addr(f->esp + 8);
		  f->eax = create(*(uintptr_t*)(f->esp + 4), *(unsigned*)(f->esp + 8));
		  break;

	  case SYS_REMOVE:
		  _valid_addr(f->esp + 4);
		  f->eax = remove(*(uintptr_t*)(f->esp + 4));
		  break;

	  case SYS_OPEN:
		  _valid_addr(f->esp + 4);
		  lock_acquire(&filelock);
		  f->eax = open(*(uintptr_t*)(f->esp + 4));
		  lock_release(&filelock);
		  break;

	  case SYS_FILESIZE:
		  _valid_addr(f->esp + 4);
		  f->eax = filesize(*(int*)(f->esp + 4));
		  break;

      case SYS_READ:
          _valid_addr(f->esp + 4);
          _valid_addr(f->esp + 8);
          _valid_addr(f->esp + 12);
		  lock_acquire(&filelock);
          f->eax = read(*(int*)(f->esp + 4), *(uintptr_t*)(f->esp + 8), *(unsigned*)(f->esp + 12));
		  lock_release(&filelock);
          break;

      case SYS_WRITE:
          _valid_addr(f->esp + 4);
          _valid_addr(f->esp + 8);
          _valid_addr(f->esp + 12);
		  lock_acquire(&filelock);
          f->eax = write(*(int*)(f->esp + 4), *(uintptr_t*)(f->esp + 8), *(unsigned*)(f->esp + 12));
		  lock_release(&filelock);
          break;

	  case SYS_SEEK:
		  _valid_addr(f->esp + 4);
		  _valid_addr(f->esp + 8);
		  seek(*(int*)(f->esp + 4), *(unsigned*)(f->esp + 8));
		  break;

	  case SYS_TELL:
		  _valid_addr(f->esp + 4);
		  f->eax = tell(*(int*)(f->esp + 4));
		  break;

	  case SYS_CLOSE:
		  _valid_addr(f->esp + 4);
		  close(*(int*)(f->esp + 4));
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
	int i;
	for(i = 3; i < 128; ++i){
		if(thread_current()->filelist[i] != NULL)
			close(i);
	}
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

bool create(const char *file, unsigned initial_size){
	if(file == NULL)
		exit(-1);

	return filesys_create(file, initial_size);
}

bool remove(const char *file){
	if(file == NULL)
		exit(-1);

	return filesys_remove(file);
}

int open(const char *file){
	int i;

	if(file == NULL)
		return -1;
	
	struct file *fp = filesys_open(file);
	if(fp == NULL)
		return -1;

	for(i = 3; i < 128; ++i){
		if(thread_current()->filelist[i] == NULL){
			if(strcmp(thread_current()->name , file) == 0)
				file_deny_write(fp);
			break;
		}
	}

	if(i >= 128)
		return -1;

	thread_current()->filelist[i] = fp;

	return i;
}

int filesize(int fd){
	return file_length(thread_current()->filelist[fd]);
}

int read(int fd, void *buffer, unsigned size){
	int i;

	_valid_addr(buffer + size);
	_valid_addr(buffer);

    if (fd == 0){
        for(i = 0; i < size; ++i)
			*(uint8_t*)(buffer + i) = input_getc();
		return size;
	}

	else if((fd >= 2)&&(fd < 128)){
		if(thread_current()->filelist[fd] == NULL)
			return -1;
		
		return file_read(thread_current()->filelist[fd], buffer, size);
	}

	return -1;
}

int write(int fd, const void *buffer, unsigned size){

	_valid_addr(buffer + size);
	if(fd == NULL)
		return -1;

    if(fd == 1){
        putbuf(buffer, size);
        return size;
    }

	else if((fd >= 2)&&(fd < 128)){
		if(thread_current()->filelist[fd] == NULL)
			return -1;
		
		return file_write(thread_current()->filelist[fd], buffer, size);
	}

    return -1;
}

void seek(int fd, unsigned position){
	if(position < 0)
		exit(-1);

	if((fd < 2)||(fd > 127))
		exit(-1);

	if(thread_current()->filelist[fd] == NULL)
		exit(-1);

	file_seek(thread_current()->filelist[fd], position);
}

unsigned tell(int fd){
	if((fd < 2)||(fd > 127))
		exit(-1);

	if(thread_current()->filelist[fd] == NULL)
		exit(-1);

	return file_tell(thread_current()->filelist[fd]);
}

void close(int fd){
	if((fd < 2)||(fd > 127))
		exit(-1);

	if(thread_current()->filelist[fd] == NULL)
		exit(-1);

	file_close(thread_current()->filelist[fd]);
	thread_current()->filelist[fd] = NULL;
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
