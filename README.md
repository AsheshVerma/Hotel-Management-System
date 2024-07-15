# Hotel-Management-System
Operating Systems
There are 4 Processes : customers,tables,waiters and a manager.
They communicate through inter-process communication (IPC) techniques like pipes and shared memory(SHM).
Waiters take order from Customers which are child processes of Table process and then finally calculate the total bill for the designated table.
Then all the waiters pass info about total bill to the manager using SHM which is then used for calculating total revenue,wage of waiters,profit.
