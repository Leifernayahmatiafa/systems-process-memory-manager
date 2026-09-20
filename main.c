#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

/* =========================================================================
 * PART 1: LOW-LEVEL MEMORY MANAGEMENT & BOUNDARY TAGS
 * Demonstrates heap allocation bookkeeping, pointer arithmetic, and bounds.
 * ========================================================================= */

// Boundary tag header prepended to every raw heap allocation
typedef struct BlockHeader {
    size_t size;            // Usable payload size in bytes
    int is_free;            // Allocation state flag: 0 = in-use, 1 = freed
    unsigned int magic;     // Integrity validation marker (detects buffer overflow)
} BlockHeader;

#define MAGIC_TAG 0xDEADBEEF

/**
 * Custom memory allocator wrapper:
 * Reserves space for metadata boundary tags + raw memory payload.
 */
void* custom_malloc(size_t size) {
    size_t total_size = sizeof(BlockHeader) + size;
    void* raw_block = malloc(total_size);

    if (raw_block == NULL) {
        perror("[ERROR] Heap allocation failed");
        return NULL;
    }

    // Embed boundary tag at the base of the allocated segment
    BlockHeader* header = (BlockHeader*)raw_block;
    header->size = size;
    header->is_free = 0;
    header->magic = MAGIC_TAG;

    // Pointer arithmetic: return address immediately following header metadata
    void* user_payload = (char*)raw_block + sizeof(BlockHeader);
    printf("[MEM_ALLOC] Allocated %zu bytes payload (Total block: %zu bytes) at %p\n",
           size, total_size, user_payload);
    return user_payload;
}

/**
 * Custom free routine:
 * Validates heap integrity via boundary tags before releasing memory.
 */
void custom_free(void* ptr) {
    if (ptr == NULL) return;

    // Shift pointer backwards to read block metadata
    BlockHeader* header = (BlockHeader*)((char*)ptr - sizeof(BlockHeader));

    // Verify heap boundary tag integrity to detect corruption/overflow
    if (header->magic != MAGIC_TAG) {
        fprintf(stderr, "[ERROR] Memory corruption detected! Boundary tag compromised.\n");
        return;
    }

    if (header->is_free) {
        fprintf(stderr, "[WARN] Double free attempted on pointer %p\n", ptr);
        return;
    }

    header->is_free = 1;
    printf("[MEM_FREE] Successfully released %zu bytes at %p\n", header->size, ptr);
    free(header);
}

/* =========================================================================
 * PART 2: UNIX SIGNAL HANDLING & POSIX PROCESS CONTROL
 * Demonstrates signal trapping, fork(), execvp(), and waitpid().
 * ========================================================================= */

volatile sig_atomic_t sigint_received = 0;

void handle_sigint(int sig) {
    (void)sig;
    sigint_received = 1;
    const char msg[] = "\n[SIGNAL] SIGINT caught! Handled gracefully by process.\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
}

/**
 * Spawns a child process using fork(), executes a command via execvp(),
 * and tracks the exit status with waitpid().
 */
void execute_posix_task(char* command, char* args[]) {
    printf("\n[PROCESS] Spawning child process to execute: %s\n", command);

    pid_t pid = fork();

    if (pid < 0) {
        perror("[ERROR] fork() system call failed");
        return;
    } else if (pid == 0) {
        // Child Process
        printf("[CHILD] PID %d running '%s'...\n", getpid(), command);
        if (execvp(command, args) == -1) {
            perror("[CHILD ERROR] execvp execution failed");
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent Process
        int status;
        printf("[PARENT] PID %d waiting for child PID %d to terminate...\n", getpid(), pid);

        if (waitpid(pid, &status, 0) == -1) {
            perror("[ERROR] waitpid failed");
            return;
        }

        if (WIFEXITED(status)) {
            printf("[PARENT] Child PID %d exited normally with code: %d\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("[PARENT] Child PID %d terminated by signal: %d\n", pid, WTERMSIG(status));
        }
    }
}

/* =========================================================================
 * PART 3: MAIN ENTRY
 * ========================================================================= */

int main(void) {
    printf("=====================================================\n");
    printf("  Systems Programming: Memory & POSIX Process Manager\n");
    printf("=====================================================\n\n");

    // 1. Register POSIX Signal Handler
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("[ERROR] sigaction registration failed");
        return 1;
    }
    printf("[INIT] SIGINT signal handler registered successfully.\n\n");

    // 2. Demonstrate Custom Heap Allocation & Boundary Verification
    printf("--- Phase 1: Heap Allocation & Boundary Verification ---\n");
    char* test_buffer = (char*)custom_malloc(64);
    if (test_buffer != NULL) {
        strncpy(test_buffer, "Low-level C Memory Buffer Active", 64);
        printf("[BUFFER CONTENT] \"%s\"\n", test_buffer);
        custom_free(test_buffer);
    }

    // 3. Demonstrate POSIX Process Lifecycle
    printf("\n--- Phase 2: POSIX Process Lifecycle Management ---\n");
    char* cmd = "uname";
    char* args[] = {"uname", "-s", "-r", NULL};
    execute_posix_task(cmd, args);

    printf("\n=====================================================\n");
    printf("  Execution Completed Cleanly\n");
    printf("=====================================================\n");

    return 0;
}
