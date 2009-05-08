//
// Copyright (c) 2007 Tridium, Inc.
// Licensed under the Academic Free License version 3.0
//
// History:
//   3 Mar 07  Brian Frank  Creation
//

#include "sedona.h"
#ifdef USE_STANDARD_MAIN

// includes
#include <stdio.h>
#include <stdlib.h>
#include "errorcodes.h"

// sys::Sys forward
int64_t sys_Sys_ticks(SedonaVM* vm, Cell* params);
void sys_Sys_sleep(SedonaVM* vm, Cell* params);

// forwards
static int printUsage(const char* exe);
static int printVersion();
static int loadFile(const char* filename, uint8_t** addr, size_t* size);
static void onAssertFailure(const char* location, uint16_t linenum);

// auto-generated by sedonac in "nativetable.c"
extern NativeMethod* nativeTable[];

////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  int result;
  int i;
  char* filename = NULL;
  SedonaVM vm;
  int optCount = 0;
  int64_t t1, t2;

  // parse arguments
  for (i=1; i<argc; ++i)
  {
    char* arg = argv[i];
    if (arg[0] == '-')
    {
      if (strcmp(arg, "--?") == 0)   return printUsage(argv[0]);
      if (strcmp(arg, "--ver") == 0) return printVersion();
      if (strncmp(arg, "--home=", 7) == 0)
      {
        char* home = arg+7;
        if (strlen(arg) < 8) return printUsage(argv[0]);
        if (_chdir(home) != 0) return printUsage(argv[0]);
        optCount++;
      }
    }
    else
    {
      if (filename == NULL)
        filename = arg;
    }
  }

  // if no argument then print usage and bail
  if (filename == NULL)
  {
    printUsage(argv[0]);
    return ERR_INPUT_FILE_NOT_FOUND;
  }

  // load input file into memory
  result = loadFile(filename, (uint8_t**)&vm.codeBaseAddr, &vm.codeSize);
  if (result != 0)
  {
    printf("Cannot load input file (%d): %s\n", result, filename);
    return result;
  }

  // alloc stack (hardcoded for now)
  vm.stackMaxSize  = 16384;
  vm.stackBaseAddr = (uint8_t*)malloc(vm.stackMaxSize);
  if (vm.stackBaseAddr == NULL)
  {
    printf("Cannot malloc stack segments\n");
    return ERR_MALLOC_STACK;
  }

  // setup arguments
  vm.args = argv + (2+optCount);
  vm.argsLen = argc - (2 + optCount);

  // setup callbacks
  vm.onAssertFailure = onAssertFailure;

  // setup native method table
  vm.nativeTable = nativeTable;

  // setup call function pointer
  vm.call = vmCall;

  // print version info
  printVersion();

  // run the VM
  t1 = sys_Sys_ticks(NULL, NULL);
  result = vmRun(&vm);

  // if the VM exited into a hibernation state, then
  // simulate a hibernate with a sleep and resume
  while (result == ERR_HIBERNATE)
  {
    printf("-- Simulated hibernate --\n");
    result = vmUnhibernate(&vm);
  }

  // done
  if (result != 0)
  {
    printf("Cannot run VM (%d)\n", result);
    return result;
  }
  t2 = sys_Sys_ticks(NULL, NULL);

  printf("\n");
  printf("VM Completed\n");
#ifdef _WIN32
  printf("Total Time       = %I64d ms\n", (t2-t1)/1000000i64);
#else
  printf("Total Time       = %lld ms\n", (t2-t1)/1000000ll);
#endif
  printf("Assert Successes = %d\n", vm.assertSuccesses);
  printf("Assert Failures  = %d\n", vm.assertFailures);
  printf("\n");
  if (vm.assertFailures == 0)
  {
    printf("--\n");
    printf("-- All tests passed\n");
    printf("--\n");
    return 0;
  }
  else
  {
    printf("--\n");
    printf("-- %d TESTS FAILED!!!\n", vm.assertFailures);
    printf("--\n");
    return vm.assertFailures;
  }
}

////////////////////////////////////////////////////////////////
// Print Usage
////////////////////////////////////////////////////////////////

static int printUsage(const char* exe)
{
  printf("usage:\n");
  printf("  %s [options] <scode file>\n", exe);
  printf("options:\n");
  printf("  --?       dump usage\n");
  printf("  --ver     dump version\n");
  printf("  --home=d  set current working directory\n");
  return 0;
}

////////////////////////////////////////////////////////////////
// Print Version
////////////////////////////////////////////////////////////////

static int printVersion()
{
#ifdef IS_BIG_ENDIAN
  const char* endian = "big";
#else
  const char* endian = "little";
#endif

  printf("\n");
  printf("Sedona VM 1.0\n");
  printf("buildDate: %s %s\n", __DATE__, __TIME__);
  printf("endian:    %s\n", endian);
  printf("blockSize: %d\n", SCODE_BLOCK_SIZE);
  printf("refSize:   %d\n", sizeof(void*));
  printf("\n");
  return 0;
}

////////////////////////////////////////////////////////////////
// Load File
////////////////////////////////////////////////////////////////

static int loadFile(const char* filename, uint8_t** paddr, size_t* psize)
{
  size_t result;
  FILE* file;
  size_t size;
  uint8_t* addr;

  // open file
  file = fopen(filename, "rb");
  if (file == NULL) return ERR_INPUT_FILE_NOT_FOUND;

  // seek to end to get file size
  result = fseek(file, 0, SEEK_END);
  if (result != 0) return ERR_CANNOT_READ_INPUT_FILE;
  size = ftell(file);
  rewind(file);

  // allocate memory for image
  addr = (uint8_t*)malloc(size);
  if (addr == NULL)
    return ERR_MALLOC_IMAGE;

  // read file into memory
  result = fread(addr, 1, size, file);
  if (result != size) return ERR_CANNOT_READ_INPUT_FILE;

  // success
  *paddr = addr;
  *psize = size;
  fclose(file);
  return 0;
}

//////////////////////////////////////////////////////////////////////////
// VM Callbacks
//////////////////////////////////////////////////////////////////////////

static void onAssertFailure(const char* location, uint16_t linenum)
{
  printf("ASSERT FAILURE: %s [Line %d]\n", location, linenum);
}

#endif
