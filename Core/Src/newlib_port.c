#include <errno.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <assert.h>
#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef huart1;

int _write (int fd __unused, char *ptr, int len)
{
	int i;
	for(i=0;i<len;i++)
	{
		HAL_UART_Transmit(&huart1, (uint8_t*)&ptr[i], 1, HAL_MAX_DELAY);
		if(ptr[i] == '\n')
			HAL_UART_Transmit(&huart1, (uint8_t*)"\r", 1, HAL_MAX_DELAY);
	}
	return i;
}

int _read(int fd __unused, char* ptr, int len)
{
	return len;
}

/* _exit */
void _exit(int status __unused) {
	while(1);
}

/* close */
int _close(int file __unused) {
	return -1;
}

/* fstat */
int _fstat(int file __unused, struct stat *st) {
	st->st_mode = S_IFCHR;
	return 0;
}

int _getpid(void) {
	return 1;
}

int _isatty(int file __unused) {
	return 1;
}

int _kill(int pid __unused, int sig __unused) {
	errno = EINVAL;
	return -1;
}

int _lseek(int file __unused, int ptr __unused, int dir __unused) {
	return 0;
}

void *_sbrk(int incr) {
	extern char __HEAP_START__;
	extern char __HEAP_END__;
	static unsigned char *heap = (unsigned char *)(uintptr_t)(&__HEAP_START__);
	unsigned char *prev_heap;
	prev_heap = heap;
	if((uintptr_t)(heap + incr) > (uintptr_t)&__HEAP_END__)
	{
		_write(1, "Heap Overflow!\n\r", 16);
		while(1);
	}
	heap += incr;
	// if((uintptr_t)heap&7)
		// _write(1, "Heap is not Aligned!\n\r", 22);
	return prev_heap;
}

#if defined(FREERTOS)

#include "FreeRTOS.h"
#include "task.h"
void __malloc_lock(struct _reent *r __unused)   
{
	configASSERT(!xPortIsInsideInterrupt()); // 禁止在中断中使用malloc
	if(xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)
		return;
	vTaskSuspendAll();
}

void __malloc_unlock(struct _reent *r __unused) 
{
	configASSERT(!xPortIsInsideInterrupt()); // 禁止在中断中使用malloc
	if(xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)
		return;
	(void)xTaskResumeAll();
}
#endif

/* environment */
char *__env[1] = { 0 };
char **environ = __env;

int link(char *old __unused, char *new __unused) {
	errno = EMLINK;
	return -1;
}

int _open(const char *name __unused, int flags __unused, int mode __unused) {
	return -1;
}

/* execve */
// int execve(char *name __unused, char **argv __unused, char **env __unused) {
// 	errno = ENOMEM;
// 	return -1;
// }

/* fork */
int fork(void) {
	errno = EAGAIN;
	return -1;
}


int stat (const char *__restrict __path __unused, struct stat *__restrict __sbuf ) {
	__sbuf->st_mode = S_IFCHR;
	return 0;
}

// int times(struct tms *buf) {
//   return -1;
// }

int unlink(char *name __unused) {
	errno = ENOENT;
	return -1;
}

int wait(int *status __unused) {
	errno = ECHILD;
	return -1;
}

typedef void (*ptr_func_t)();
extern char __preinit_array_start;
extern char __preinit_array_end;

extern char __init_array_start;
extern char __init_array_end;

extern char __fini_array_start;
extern char __fini_array_end;

/** Call constructors for static objects
 */
void call_init_array() {
    uintptr_t* func = (uintptr_t*)&__preinit_array_start;
    while (func < (uintptr_t*)&__preinit_array_end) {
		(*(ptr_func_t)(*func))();
        func++;
    }

    func = (uintptr_t*)&__init_array_start;
    while (func < (uintptr_t*)&__init_array_end) {
        (*(ptr_func_t)(*func))();
        func++;
    }
}

/** Call destructors for static objects
 */
void call_fini_array() {
    ptr_func_t array = (ptr_func_t)&__fini_array_start;
    while (array < (ptr_func_t)&__fini_array_end) {
        (*array)();
        array++;
    }
}



// void _fini()
// {
// 	return;
// }
