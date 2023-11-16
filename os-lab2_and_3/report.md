# Report Group 26 Lab 2
## Data structures
We edited the thread structure by adding  `int64_t wakeuptime` which is used to have each thread keep track of its own time to wake up. 

## Algorithms and Synchronization
We added a new function we call wake_up which takes a thread as an input. It checks if that thread is blocked and if it has reached its wake up time. 

If it fulfills this, then the thread is unblocked. 

We modified the timer_sleep function to set a threads wake up time. Wake up time is calculated by adding the starting time and the ticks. It also sets a thread's status to blocked using thread_block. Removing the busy-wait loop. 

We disable interrupts during this operation to synchronize all threads, ensuring that a thread won't change its status during the operation. 

We also modified the timer_interrupt function. We disable interrupts then call the thread_foreach function with our wake_up function to update the threads if necessary. Afterwards we enable interrupts again and increment the ticks. 

## Time and space complexity
An informal analysis of our code: since we go through all threads each time to check their status, we have a time complexity of O(n). Our space complexity is also O(n) since we need to check all threads.

We were thinking of having another list collecting only the blocked threads, however we went for the simpler solution in the end. Our idea being that we would iterate over fewer threads per timer_interrupt call. 