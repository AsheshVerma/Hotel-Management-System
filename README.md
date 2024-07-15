# Hotel-Management-System
Operating Systems
There are 4 Processes : customers,tables,waiters and a manager.
They communicate through inter-process communication (IPC) techniques like pipes and shared memory(SHM).
waiters take order from customers which are child processes of table process and then finally calculate the total bill for the designated table.
Then all the waiters paas info about totall bill to the manager using SHM which is then used for calculting total revenue,wage of waiters,profit.
