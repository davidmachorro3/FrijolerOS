#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"

void exit (int status) {
  //Ver que mas hay que hacer aqui
  
  printf("%s: exit(%d)", thread_current()->name, status);

}

int write (int fd, const void *buffer, unsigned size) {
  
  //Provisionalmente

  return 0;
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

  switch(sys_code) {
    case SYS_HALT:
      break;
    case SYS_EXIT: {
      // Leer parametro status y llamar a funcion

      int status = *((int*)f->esp + 1);

      exit(status);

      break;
    }
    case SYS_EXEC:
      break;
    case SYS_WAIT:
      break;
    case SYS_CREATE:
      break;
    case SYS_REMOVE:
      break;
    case SYS_OPEN:
      break;
    case SYS_FILESIZE:
      break;
    case SYS_READ:
      break;
    case SYS_WRITE: {

      //Obtener parametros para write

      int fd = *((int*)f->esp + 1);
      void* buffer = (void*)(*((int*)f->esp + 2));
      unsigned size = *((unsigned*)f->esp + 3);

      //Verificar aqui o en write que el puntero buffer es valido

      f->eax = write(fd, buffer, size);

      break;
    }
    case SYS_SEEK:
      break;
    case SYS_TELL:
      break;
    case SYS_CLOSE:
      break;
  }

}

