#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  printf("\n--- STARTING SUICIDE BOMB TEST ---\n");
  printf("Parent: My PID is %d\n", getpid());

  int pid = fork();

  if(pid < 0) {
    printf("Fork failed.\n");
    exit(1);
  }

  if(pid == 0) {
    // Give the parent 5 ticks to finish printing before the child starts
    // You can use a dummy loop for this too
    int i = 0;
    while(i < 1000000) i++; 
    
    printf("Child: I am alive! My PID is %d\n", getpid());
    while(1) { }
  } else {
    // Parent Process
    int i = 0;
    while(i < 500000000) i++;
    int ticks = 50;
    printf("Parent: Arming the bomb for %d ticks...\n\n", ticks);
    
    suicide_bomb(ticks);

    // Keep the parent busy too
    while(1) {
         // Empty loop
    }
  }
  
  exit(0);
}