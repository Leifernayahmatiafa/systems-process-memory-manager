# Systems Programming & Memory Management

A low-level C systems program demonstrating custom heap memory allocation tracking, POSIX process lifecycle management (`fork`, `execvp`, `waitpid`), and Unix signal handling.

## Technical Highlights
* **Memory Management & Boundary Tags:** Implements custom allocation wrappers (`custom_malloc` / `custom_free`) using pointer arithmetic to attach heap boundary tags (`BlockHeader`), tracking allocation size, block state, and magic number integrity markers to prevent buffer corruption.
* **POSIX Process Control:** Orchestrates isolated process execution using `fork()`, context swapping via `execvp()`, and parent state synchronization using `waitpid()` with status macro evaluations (`WIFEXITED`, `WEXITSTATUS`).
* **Signal Handling:** Configures POSIX signal handlers with `sigaction()` to safely trap and handle asynchronous interrupts (`SIGINT`) without unhandled runtime aborts.

## Build and Run (Linux / Unix / WSL)

1. **Clone the repository:**
   ```bash
   git clone [https://github.com/Leifernayahmatiafa/systems-process-memory-manager.git](https://github.com/Leifernayahmatiafa/systems-process-memory-manager.git)
   cd systems-process-memory-manager
