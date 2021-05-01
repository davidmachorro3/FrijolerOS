#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "lib/user/syscall.h"
#include "threads/init.h"
#include "filesys/filesys.h"
#include "filesys/file.h"

void halt (void) {
  //Terminar PintoS

  //Revisar esta funcion (no esta en init.h)

  shutdown_power_off();
}


void exit (int status) {
  //Ver que mas hay que hacer aqui
  
  printf("%s: exit(%d)", thread_current()->name, status);

}

pid_t exec (const char *cmd_line) {
  
  //Provisional

  

  return 0;
}

int wait (pid_t pid) {

  //Provisional 
  return 0;  
}

bool create (const char *file, unsigned initial_size) {
  //Provisional
  return false;
}

bool remove (const char *file) {
  //Provisional
  return false;
}

int open (const char *file) {
  //Provisional
  return 0;
}

int filesize (int fd) {
  //Provisional
  return 0;
}

int read (int fd, void *buffer, unsigned size) {
  //Provisional
  return 0;
}

int write (int fd, const void *buffer, unsigned size) {
  
  //Provisionalmente

  return 0;
}

void seek (int fd, unsigned position) {

}

unsigned tell (int fd) {
  //Provisional
  return 0;
}

void close (int fd) {

}

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}



static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n");

  //Conseguir directorio de paginas actual

  uint32_t *page_directory = thread_current()->pagedir;

  //Si 'puntero' es null, la direccion no esta mapeada

  void *puntero = pagedir_get_page(page_directory, (void *)f->esp);

  //Verificar que el stack pointer sea valido

  if(is_kernel_vaddr(f->esp) || ((int)f->esp) < 0x08084000 || puntero == NULL) {
    // Terminar el proceso que mando puntero (exit) y liberar sus recursos
    exit(-1);
  }

  //Obtener el codigo de la syscall

  int sys_code = *(int *)f->esp;

  switch(sys_code) 
  {
    case SYS_HALT:
    {
      halt();
      break;
    }
      
    case SYS_EXIT: 
    {
      // Leer parametro status y llamar a funcion

      int status = *((int*)f->esp + 1);

      exit(status);

      break;
    }
    case SYS_EXEC:
    {
      char *cmd_line = (char *)(*((int*)f->esp + 1));
      

      //Pendiente de implementar
      break;
    }
    case SYS_WAIT:
    {
      pid_t pid = (pid_t)(*((int*)f->esp + 1));
      break;
    }
    case SYS_CREATE:
    {
      char *file = (char *)(*((int*)f->esp + 1));
      unsigned initial_size = *((unsigned *)f->esp + 2);
      break;
    }
    case SYS_REMOVE:
    {
      char *file = (char *)(*((int*)f->esp + 1));
      break;
    }
    case SYS_OPEN:
    {
      char *file = (char *)(*((int*)f->esp + 1));
      break;
    }
    case SYS_FILESIZE:
    {
      int fd = *((int*)f->esp + 1);
      break;
    }
    case SYS_READ:
    {
      int fd = *((int*)f->esp + 1);
      void* buffer = (void*)(*((int*)f->esp + 2));
      unsigned size = *((unsigned*)f->esp + 3);
      break;
    }
    case SYS_WRITE: 
    {

      //Obtener parametros para write

      int fd = *((int*)f->esp + 1);
      void* buffer = (void*)(*((int*)f->esp + 2));
      unsigned size = *((unsigned*)f->esp + 3);

      //Verificar aqui o en write que el puntero buffer es valido

      f->eax = write(fd, buffer, size);

      break;
    }
    case SYS_SEEK:
    {
      int fd = *((int*)f->esp + 1);
      unsigned position = *((unsigned*)f->esp + 2);
      break;
    }
    case SYS_TELL:
    {
      int fd = *((int*)f->esp + 1);
      break;
    }
    case SYS_CLOSE:
    {
      int fd = *((int*)f->esp + 1);
      break;
    }
  }

}

