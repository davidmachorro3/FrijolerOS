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
#include "lib/kernel/stdio.h"
#include "filesys/off_t.h"
#include "devices/shutdown.h"
#include "userprog/process.h"

void halt (void) {
  //Terminar PintoS

  //Revisar esta funcion (no esta en init.h)

  shutdown_power_off();
}


void exit (int status) {
  //Creo que aqui tambien hay que liberar espacio
  
  printf("%s: exit(%d)", thread_current()->name, status);
  thread_exit();
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
  
  return filesys_create(file, (off_t)initial_size);
}

bool remove (const char *file) {
  
  return filesys_remove(file);
}

int open (const char *file) {
  //Provisional

  //Hacer mapeo entre files y file descriptors

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

  int tamano_escrito = 0;

  if(fd == 1) {

    putbuf((char *)buffer, (size_t)size);
    tamano_escrito = size;
  } else {

  }

  return tamano_escrito;
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

  if(is_kernel_vaddr(f->esp) || ((uint32_t)f->esp) < 0x08084000 || puntero == NULL) {
    // Terminar el proceso que mando puntero (exit) y liberar sus recursos
    exit(-1);
  }

  //Obtener el codigo de la syscall

  int sys_code = *(int *)f->esp;

  switch(sys_code) 
  {
    case SYS_HALT:
    {
      printf("JOSUE1");

      halt();
      break;
    }
      
    case SYS_EXIT: 
    {

      int *puntero_status = (int *)f->esp + 1;

      puntero = pagedir_get_page(page_directory, (void *)puntero_status);

      if(is_kernel_vaddr((void *)puntero_status) || ((int)puntero_status) < 0x08084000 || puntero == NULL) {
        exit(-1);
      } else {
        int status = *puntero_status;

        exit(status);
      }

	    break;
    }
    case SYS_EXEC:
    {
      printf("JOSUE2");

      char *cmd_line = (char *)(*((int*)f->esp + 1));
      

      //Pendiente de implementar
      break;
    }
    case SYS_WAIT:
    {
      printf("JOSUE3");

      pid_t pid = (pid_t)(*((int*)f->esp + 1));
      break;
    }
    case SYS_CREATE:
    {
      printf("JOSUE4");

      char *file = (char *)(*((int*)f->esp + 1));
      unsigned initial_size = *((unsigned *)f->esp + 2);
      break;
    }
    case SYS_REMOVE:
    {
      printf("JOSUE5");

      char *file = (char *)(*((int*)f->esp + 1));
      break;
    }
    case SYS_OPEN:
    {
      printf("JOSUE6");

      char *file = (char *)(*((int*)f->esp + 1));
      break;
    }
    case SYS_FILESIZE:
    {

      printf("JOSUE7");
      int fd = *((int*)f->esp + 1);
      break;
    }
    case SYS_READ:
    {
      printf("JOSUE8");

      int fd = *((int*)f->esp + 1);
      void* buffer = (void*)(*((int*)f->esp + 2));
      unsigned size = *((unsigned*)f->esp + 3);
      break;
    }
    case SYS_WRITE: 
    {
      printf("++++++++JOSUE+++++++");

      int *puntero_fd = ((int*)f->esp) + 1;
      int *puntero_buffer = ((int*)f->esp) + 2;
      unsigned *puntero_size = ((unsigned*)f->esp) + 3;

      puntero = pagedir_get_page(page_directory, (void *)puntero_fd);
      
      if(is_kernel_vaddr((void *)puntero_fd) || ((uint32_t)puntero_fd) < 0x08084000 || puntero == NULL) {
        exit(-1);
        return;
      }

      puntero = pagedir_get_page(page_directory, (void *)puntero_buffer);

      if(is_kernel_vaddr((void *)puntero_buffer) || ((uint32_t)puntero_buffer) < 0x08084000 || puntero == NULL) {
        exit(-1);
        return;
      }
      
      puntero = pagedir_get_page(page_directory, (void *)puntero_size);

      printf("\n\nPUNTEROPOO : %p\n\n", puntero);

      if(is_kernel_vaddr((void *)puntero_size) || ((uint32_t)puntero_size) < 0x08084000 || puntero == NULL) {
        exit(-1);
        return;
      }

      int fd = *puntero_fd;
      void* buffer = (void *)(*puntero_buffer);
      unsigned size = *puntero_size;


      f->eax = write(fd, buffer, size);

      break;
    }
    case SYS_SEEK:
    {
      printf("++++++++JOSUE9");

      int fd = *((int*)f->esp + 1);
      unsigned position = *((unsigned*)f->esp + 2);
      break;
    }
    case SYS_TELL:
    {
      printf("JOSUE10");
      int fd = *((int*)f->esp + 1);
      break;
    }
    case SYS_CLOSE:
    {
      printf("JOSUE11");
      int fd = *((int*)f->esp + 1);
      break;
    }
  }

}

