#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// A simple busy-wait delay loop to substitute for the user sleep function
static void
delay_ticks(int loops)
{
  volatile int count = 0;
  for(int i = 0; i < loops * 500000; i++) {
    count++;
  }
}

// ------------------------------------------------------------------
// Scenario A — Timeout
// ------------------------------------------------------------------
static void
scenario_a(void)
{
  printf("\n=== Scenario A: Timeout ===\n");
  printf("Arming bomb for 50 ticks, then entering infinite loop...\n");

  if(suicide_bomb(50) < 0){
    printf("FAIL: suicide_bomb(50) returned error\n");
    exit(1);
  }

  printf("Bomb armed. Spinning. Kernel should terminate this process.\n");
  while(1)
    ;
}

// ------------------------------------------------------------------
// Scenario B — Safe execution with disarm
// ------------------------------------------------------------------
static void
scenario_b(void)
{
  int i, result = 0;

  printf("\n=== Scenario B: Safe execution ===\n");
  printf("Arming bomb for 200 ticks...\n");

  if(suicide_bomb(200) < 0){
    printf("FAIL: suicide_bomb(200) returned error\n");
    exit(1);
  }

  printf("Bomb armed. Performing quick computation...\n");

  for(i = 0; i < 1000000; i++)
    result += i;

  // Replaced sleep(5) with our local busy wait loop
  delay_ticks(2);

  printf("Computation done (result=%d). Disarming bomb...\n", result);

  if(suicide_bomb(0) < 0){
    printf("FAIL: suicide_bomb(0) returned error\n");
    exit(1);
  }

  printf("Bomb disarmed safely. Exiting cleanly.\n\n");
  exit(0);
}

// ------------------------------------------------------------------
// Error path — negative ticks
// ------------------------------------------------------------------
static void
scenario_neg(void)
{
  int ret;

  printf("\n=== Error path: negative ticks ===\n");

  suicide_bomb(100);
  printf("Bomb armed with 100 ticks.\n");

  ret = suicide_bomb(-5);
  printf("suicide_bomb(-5) returned %d (expected -1)\n", ret);

  suicide_bomb(0);
  printf("Disarmed. Exiting.\n\n");
  exit(0);
}

// ------------------------------------------------------------------
// Edge Case 1 — Double Arming
// ------------------------------------------------------------------
static void
test_double_arm(void)
{
  printf("\n--- Edge Case 1: Double-Arming Test ---\n");
  printf("Arming bomb with 500 ticks...\n");
  suicide_bomb(500);

  printf("Overriding bomb with 20 ticks (should shorten countdown)...\n");
  suicide_bomb(20);

  printf("Spinning. Process should detonate quickly (~20 ticks), not 500.\n");
  while(1);
}

// ------------------------------------------------------------------
// Edge Case 2 — Fork Safety
// ------------------------------------------------------------------
static void
test_fork_safety(void)
{
  printf("\n--- Edge Case 2: Fork Inheritance Test ---\n");
  printf("Parent arming bomb for 150 ticks...\n");
  suicide_bomb(150);

  int pid = fork();
  if(pid < 0) {
    printf("Fork failed\n");
    exit(1);
  }

  if(pid == 0) {
    printf("Child born. Waiting briefly to see if parent's bomb blows it up...\n");
    delay_ticks(3);
    printf("SUCCESS: Child survived! Wiping slate clean worked. Disarming child.\n");
    suicide_bomb(0);
    exit(0);
  } else {
    wait(0);
    printf("Parent spinning until detonation...\n");
    while(1);
  }
}

// ------------------------------------------------------------------
// Edge Case 3 — Multi-Process Stress Test
// ------------------------------------------------------------------
static void
test_multi_process(void)
{
  printf("\n--- Edge Case 3: Multi-Process Stress Test ---\n");
  printf("Spawning 3 concurrent armed processes to check kernel lock stability...\n");

  for(int i = 0; i < 3; i++) {
    int pid = fork();
    if(pid == 0) {
      int ticks = 30 + (i * 10);
      printf("Child %d armed for %d ticks\n", getpid(), ticks);
      suicide_bomb(ticks);
      while(1);
    }
  }
  
  for(int i = 0; i < 3; i++) {
    wait(0);
  }
  printf("SUCCESS: All children detonated without kernel panicking!\n");
  exit(0);
}

// ------------------------------------------------------------------
// main entry point
// ------------------------------------------------------------------
int
main(int argc, char *argv[])
{
  if(argc < 2){
    printf("Usage: bomb_test [a|b|neg|double|fork|multi]\n");
    exit(1);
  }

  if(strcmp(argv[1], "a") == 0){
    scenario_a();
  } else if(strcmp(argv[1], "b") == 0){
    scenario_b();
  } else if(strcmp(argv[1], "neg") == 0){
    scenario_neg();
  } else if(strcmp(argv[1], "double") == 0){
    test_double_arm();
  } else if(strcmp(argv[1], "fork") == 0){
    test_fork_safety();
  } else if(strcmp(argv[1], "multi") == 0){
    test_multi_process();
  } else {
    printf("Unknown scenario '%s'\n", argv[1]);
  }

  exit(0);
}