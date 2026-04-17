#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

//project 2 task 3 and 4: used for its functions that validate user pointers and buffers
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "devices/shutdown.h" // used for shutdown_power_off() function in syscall_halt()

static void syscall_handler (struct intr_frame *);


//project 2 task 3: helper function used to validate user pointers passed to system calls
static void
validate_user_pointer(const void* ptr) {
  if (ptr == NULL || !is_user_vaddr(ptr) || pagedir_get_page(thread_current()->pagedir, ptr) == NULL) {
    thread_current()->exit_status = -1; // set exit status to -1 for invalid pointer thread terminated by the kernel
    thread_exit(); //terminate the thread immediately
  }
}
// project 2 task 3: helper function used to validate user buffers passed to system calls
static void
validate_user_buffer(const void* buffer, size_t size) {
  const char* buf= (const char*) buffer;
  size_t i;
  for (i = 0; i < size; i++) {
    validate_user_pointer(buf + i); // validate each byte in the buffer
  }
}
//project 2 task 3: helper function used to copy a uint32_t value from user space to kernel space safely
static uint32_t
copy_in_u32(const void* ptr){
  validate_user_buffer(ptr, sizeof(uint32_t)); // validate the buffer for 4 bytes
  return *(const uint32_t*)ptr; // return the value at the pointer
}

//project 2 task 4: helper function used to handle the SYS_HALT system call by shutting down the machine
static void
syscall_halt(void){
  shutdown_power_off(); // power off the machine
}

static void
syscall_exit(int status){
  thread_current()->exit_status = status; // set the exit status of the current thread
  thread_exit(); // terminate the thread
}
static int
syscall_write(int fd, const void* buffer, unsigned size){
  // for simplicity, we will only handle writing to standard output (fd == 1)
  if (fd != 1)  return -1;
  putbuf(buffer, size); // write the buffer to the console
  return size; // return the number of bytes written
}
//end of project 2 adds

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

//project 2 task 4: modified syscall_handler to handle different system calls based on the system call number passed in the eax register
static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  int syscall_num = copy_in_u32(f->esp); // get the system call number from the stack
  switch (syscall_num) {
    case SYS_HALT:
      syscall_halt();
      break;
    case SYS_EXIT: {
      int status = (int)copy_in_u32(f->esp + 4); // get the exit status argument from the stack
      syscall_exit(status);
      break;
    }
    case SYS_WRITE: {
      int fd = (int)copy_in_u32(f->esp + 4); // get the file descriptor argument from the stack
      const void* buffer = (const void*) copy_in_u32(f->esp + 8); // get the buffer pointer argument from the stack
      unsigned size = copy_in_u32(f->esp + 12); // get the size argument from the stack

      validate_user_buffer(buffer, size); // validate the user buffer before writing

      f->eax = syscall_write(fd, buffer, size); // call the write system call handler and store the return value in the eax register
      break;
    }
    default:
      syscall_exit(-1); // for unrecognized system calls, exit with status -1
  }
}
