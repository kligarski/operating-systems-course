# Clients-Printers
## Task description
Using semaphores and shared memory from System V IPC or the POSIX standard, 
write a program that simulates the operation of a printing system:

The system consists of N users and M printers. 
Each user can send a print job to the system, consisting of a text string with 10 characters. 
A printer that is not currently occupied takes on the task of "printing" the text from the print job. 
Printing in this task involves outputting the characters sent for printing to standard output, 
with each subsequent character being printed at one-second intervals. If all printers are busy, 
the print jobs are queued in the printing system. If the queue is full,
a user attempting to submit a print job waits until space is available in the queue.

Each of the N users should send a print job with 10 random characters (from 'a' to 'z') 
to the printing system and then wait for a random number of seconds. 
The tasks of submitting a print job and waiting should be performed in an infinite loop.

The users and printers' operations need to be synchronized. System V or POSIX mechanisms should be used.

## Usage
Run `./initlialize_queue` to initialize and watch in real-time the queue. Then run as many clients and printers as you want.
```bash
make
./initialize_queue
./client
./printer
```