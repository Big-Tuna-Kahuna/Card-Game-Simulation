#include <iostream>
#include <fstream>
#include <queue>
#include <cstdlib>

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

////////////////////////////////////////////////////////////////
#define ARR 1
#define TSO 2
#define DEP 3
#define FCFS 1
#define SJF 2
#define RR 3
////////////////////////////////////////////////////////////////
//event structure
struct event{
    float time;
    int   type;
    struct event* next;
    struct process* p;
};

////////////////////////////////////////////////////////////////
//process structure
struct process {
    int pid;
    struct process* next;
    struct event* e;
    float arrivalTime;
    float serviceTime;
    float originalBurst;
    float burstTime;
    float waitTime;
    float startQueueTime;
    //float remainingTime;
};
//This is for object creation for each process in order to just simply pass the whole
//object into .dat file. So create a process object off of data recieved from process at its departure?
/** VALUES:
*          pid- Pass in the pid of the process that is being written along with its following data.
*          timeArrival- Passes in the time that the process has entered the CPU
*          timeCompletion- Pass in the time that the process has completed its run.
*          timeRemaining- Pass in the time that the process has remaining in order to complete.
*          timeBurst- Pass in the burst value the process is doing for ROUND ROBIN ONLY if other just pass 0 or null?
*          state- Pass in the state of the process, 1 for arrivals and 2 for departures?
*          avgWait- Average waiting time the process encountered.
*          All of these variables are based off of the template values and needs for the excel sheet that Dr. Palacios gave us.
*/
class processObj {
public:
    int pid;
    float timeArrival;
    float timeCompletion;
    float timeRemaining;
    float timeBurst;
    float wait;
    int state;
    float originalBurst;
    float startQueueTime;
    float turnAround;
};
////////////////////////////////////////////////////////////////
// function definition
void init();
int run_sim();
void generate_report(int condition, processObj processObj);
int schedule_event(int, float, struct process* p);
int process_Arrival(struct event* eve);
int process_Depart(struct event* eve);
int process_TimeSlice(struct event* eve);
void firstComeFirstServe(struct event* eve);
void shortJobFirst(struct event* eve);
void roundRobin(struct event* eve);

////////////////////////////////////////////////////////////////
//Global variables
struct event* head; // head of event list
struct process* pHead;//head of process list
struct process* currentRunning;//pointer to current process being serviced, NULL if no process.
float sClock; // simulation clock
float arrivalTime;//simulation clock + genxp(1-30)
float serviceTime;//simulation clock + genxp(1/.06)
int PID = 0;
int numProcess = 0;
float quantum = 0;
int userAlgoSelection;
float arrivalRate;
float burstSeed;
float originalBurst;
float userQuantum = 0;
////////////////////////////////////////////////////////////////
int main(int argc, char *argv[] )
{
    // parse arguments
    // the user arguments from commandline are passed in here
    userAlgoSelection = atoi(argv[1]);
    arrivalRate = atof(argv[2]);
    burstSeed = atof(argv[3]);
    userQuantum = atof(argv[4]);
    init();
    run_sim();

    return 0;
}
////////////////////////////////////////////////////////////////
// returns a random number between 0 and 1
float urand()
{
    return( (float) rand()/RAND_MAX );
}
/////////////////////////////////////////////////////////////
// returns a random number that follows an exp distribution(what point in time the event arrives)
float genexp(float lambda)
{
    float u,x;
    x = 0;
    while (x == 0)
    {
        u = urand();
        x = (-1/lambda)*log(u);
    }
    return(x);
}
// Data is being written to our .dat file here, to later be imported into excel to make visual data.
/** PARAMETERS: endCondtion- How do we want to end the finishing of writing all data to file? (num processes completed?)
*               processObj- Takes in the created process object and all of its data in order to easily place all data for the process accurately.s
*/
void generate_report(int endCondition, processObj processObj)
{
    // output statistics
    ofstream write_file; //File stream file create
    write_file.open("main.dat", fstream::app);

    //Error handling if the file doesn't exist for some reason prompt user to create file.
    if( !write_file ) { // file couldn't be opened
        cerr << "Error: file could not be opened, please create the file as 'main.dat'" << endl;
        exit(1);
    }

    //While loop to start writing data from inputs into the .dat file?
    //Create an object and write the data to it per process then write each whole process object to the file.
    //End condition value needs to move and change with the value of processes we are running so in the end will be 10000.
    if (endCondition != 10000)
    {
        //Write whole object data into file:
        write_file <<  processObj.pid << ", ";
        write_file << processObj.startQueueTime << ", ";
        write_file <<  processObj.timeCompletion << ", ";
        write_file <<  processObj.originalBurst << ", ";
        write_file <<  processObj.wait << ", ";
        write_file << processObj.turnAround << ", ";
        write_file.write("\n", 1); //Start new line to keep track of each individual processes.
        //Eventually just have this count each process in and uptick by 1 ? uncomment below line if needed.
        //endCondition++;
    }
    //Close the file when done
    write_file.close();
}
////////////////////////////////////////////////////////////////
//schedules a future event for either an arrival or departure of a process. If the request is for a new
//arrival event then a new process will be created and the address will be passed to the new arrival event.
//if the event requested is a departure event then that means a process entered the simulated cpu
//to be serviced and will come with a service time of the process, an address or ID of that process, and a departure
//event will be created and insertion sorted into a linked list of processes that are sorted from least time to greatest.
int schedule_event(int type, float t, struct process* p)
{
    event* e;
    event* iterator;
    event* current;

    switch(type){
        case ARR:
            e = new event();
            p = new process();
            p->pid = PID;
            p->burstTime = genexp(1/burstSeed);
            p->originalBurst = p->burstTime;
            p->arrivalTime = 0;
            p->waitTime = 0;
            PID++;
            e->type = type;
            e->time = t;
            e->p = p;
            numProcess++;
            break;
        case TSO:
            e = new event();
            e->type = type;
            e->time = t;
            e->p = p;
            break;
        case DEP:
            e = new event();
            e->type = type;
            e->time = t;
            e->p = p;//event pointer to process
            break;
    }

    iterator = head;
    if (iterator == NULL || iterator->time > e->time) {//case for head
        e->next = iterator;
        head = e;
    }
    else {
        while (iterator->next != NULL && iterator->next->time <= e->time) {//while iterator event is less than e time and iterator not at end of list
            iterator = iterator->next;//increment pointer
        }
        e->next = iterator->next;
        iterator->next = e;
    }
    ///////////Debug-Logging//////////////
    iterator = head;
    if (e->type == ARR) {cout << "Scheduler: Arrival event scheduled for " << e->time << " with PID: " << e->p->pid << endl;}
    else if (e->type == TSO) {cout << "Scheduler: TimeSlice event scheduled for " << e->time << " with PID: " << e->p->pid << endl;}
    else {cout << "Scheduler: Departure event scheduled for " << e->time << " with PID: " << e->p->pid << endl;}
    cout << "Simulated clock: " << sClock << endl;
    cout << "\n****START Events in List****" << endl;
    while (iterator != NULL) {
        cout << "The time of event is " << iterator->time << " Type of event is " << iterator->type << endl;
        iterator = iterator->next;
    }
    cout << "****END Events in List****\n" << endl;
    return 0;
}
////////////////////////////////////////////////////////////////
void init()
{
    sClock = 0;
    quantum = userQuantum;//alternate value is .2
    event* head = NULL;
    arrivalTime = sClock + genexp(arrivalRate);//!!!!!!!!Remember to change lambda to variable from arg line!!!!!!
    process* p = NULL;//new process is only created in schedule_event
    schedule_event(ARR, arrivalTime, p);//schedule a new event and create new process
}
////////////////////////////////////////////////////////////////
int run_sim()
{
    struct event* eve;//create pointer
    bool run = true;

    while (run)//on 10,000th departure created and handled, set end_condition to true
    {
        eve = head;//pointer points to head of event list

        if (eve != NULL) {
            sClock = eve->time;

            switch (eve->type) {
                case ARR:
                    process_Arrival(eve);
                    break;
                case TSO:
                    process_TimeSlice(eve);
                    break;
                case DEP:
                    process_Depart(eve);
                    break;
                default:
                    cout << "Error - Event type out of bounds" << endl;
            }

            head = eve->next;//point to next event in list
            free(eve);//free event
            eve = NULL;//delete event
        }
        else {
            run = false;
        }
    }
    return 0;
}
////////////////////////////////////////////////////////////////
int process_Arrival(struct event* eve) {

    process* iterator;
    iterator = pHead;

    switch(userAlgoSelection) {//Selects which scheduler to use
        case FCFS:
            firstComeFirstServe(eve);
            break;
        case SJF:
            shortJobFirst(eve);
            break;
        case RR:
            roundRobin(eve);
            break;
    }

    iterator = pHead;//reset iterator
    while (iterator != NULL) {//print list of processes in queue
        cout << iterator->pid << ", ";
        iterator = iterator->next;
    }
    cout << endl;

    if (numProcess <= 10000) {//!!!Change to command line arg!!!///
        arrivalTime = sClock + genexp(arrivalRate);//!!!!Remember to replace with command line arg variable!!!!////
        process *p = NULL;//new process is only created in schedule_event
        schedule_event(ARR, arrivalTime, p);//Schedule new arrival event and create new process
    }
    return 0;
}
//////////////////////////////////////////////////////////////////
int process_Depart(struct event* eve) {

    //Generate report here: Gets snapshot of processes data before it is deleted:
    processObj processObj;
    processObj.pid = eve->p->pid;
    processObj.startQueueTime = eve->p->startQueueTime; //This is technically the real arrival time.
    processObj.originalBurst = eve->p->originalBurst;
    processObj.wait = eve->p->waitTime;
    processObj.timeCompletion = eve->p->serviceTime;
    processObj.turnAround = eve->time - eve->p->startQueueTime; //Calculates process individual turnaround time.

    generate_report(1, processObj);

    cout << "Process: " << eve->p->pid << " has finished running." << endl;
    if (pHead == NULL ) {//if process ready queue is empty
        currentRunning = NULL;//then no processes are being serviced
    }
    else {

        cout << "DEPARTURE HANDLER: Process ID: " << pHead->pid << " fetched from ready queue and sent to CPU" << endl;
        currentRunning = pHead;//current running process is next in processReadyQueue
        //adds multiple wait times if process was in Process Ready Queue more than once.
        currentRunning->waitTime = currentRunning->waitTime + (sClock - currentRunning->arrivalTime);
        cout << "DEPARTURE HANDLER: Time waited in ProcessQueue: " << currentRunning->waitTime << endl;

        if (userAlgoSelection == 3) {//!!!!if roundrobin scheme, change RR to command line variable!!!/////
            if (currentRunning->burstTime > quantum) {//if the burst time of the process is greater than quantum time
                serviceTime = quantum + sClock;//Assign system time process will complete.
                currentRunning->serviceTime = serviceTime;
                schedule_event(TSO, serviceTime, currentRunning);//schedule a timeslice event.
                pHead = currentRunning->next;//make pHead the next process in the list
                currentRunning->next = NULL;//detach process from processReadyQueue
            }
            else {//if the process will finish before the quantum time
                serviceTime = sClock + currentRunning->burstTime;
                currentRunning->serviceTime = serviceTime;
                schedule_event(DEP, serviceTime, currentRunning);
                pHead = currentRunning->next;//make pHead the next process in the list
                currentRunning->next = NULL;//detach process from processReadyQueue
            }
        }
        else {
            serviceTime = sClock + currentRunning->burstTime;//get service time of the process
            currentRunning->serviceTime = serviceTime;
            schedule_event(DEP, serviceTime,pHead);//schedule departure event, assign service time of process, assign process
            pHead = currentRunning->next;//Make pHead the next process in list
            currentRunning->next = NULL;//detach process from processReadyQueue
        }
    }
    process* p = eve->p;
    free(p);
    p = NULL;//delete process
    return 0;
}
/////////////////////////////////////////////////////////////////
int process_TimeSlice(struct event* eve) {
    process* iterator;
    iterator = pHead;

    cout << "TIMESLICE HANDLER: Process ID: " << eve->p->pid << " preempted and put into Process ready queue" << endl;
    eve->p->burstTime = eve->p->burstTime - quantum;//subtract quantum from burst time and assign new burst time


    //Put process at end of Process Ready Queue
    if (iterator == NULL) {//if list is empty
        pHead = eve->p;

    }
    else {
        while (iterator->next != NULL) {//move iterator to end of Process Queue
            iterator = iterator->next;
        }
        iterator->next = eve->p;//Add process to end of Process Ready Queue
    }
    eve->p->arrivalTime = sClock;//record the arrival time of process in the readyQueue

    iterator = pHead;
    cout << "TIMESLICE: Process Ready Queue is... " << endl;
    iterator = pHead;//reset iterator
    while (iterator != NULL) {//print list of processes in queue
        cout << iterator->pid << ", ";
        iterator = iterator->next;
    }
    cout << endl;


    //Grab new process from head of queue
    cout << "TIMESLICE HANDLER: Process ID: " << pHead->pid << " fetched from ready queue and sent to CPU" << endl;
    currentRunning = pHead;//current running process is next in processReadyQueue
    //adds multiple wait times if process was in Process Ready Queue more than once.
    currentRunning->waitTime = currentRunning->waitTime + (sClock - currentRunning->arrivalTime);
    cout << "TIMESLICE HANDLER: Time waited in ProcessQueue: " << currentRunning->waitTime << endl;

    cout << "TIMESLICE HANDLER: Process " << currentRunning->pid << " burst time is: " << currentRunning->burstTime << endl;
    if (currentRunning->burstTime > quantum) {//if the burst time of the process is greater than quantum time
        serviceTime = quantum + sClock;//Assign system time process will complete.
        currentRunning->serviceTime = serviceTime;
        schedule_event(TSO, serviceTime, currentRunning);//schedule a timeslice event.
        pHead = currentRunning->next;//make pHead the next process in the list
        currentRunning->next = NULL;//detach process from processReadyQueue
    }
    else {//if the process will finish before the quantum time
        serviceTime = sClock + currentRunning->burstTime;
        currentRunning->serviceTime = serviceTime;
        schedule_event(DEP, serviceTime, currentRunning);
        pHead = currentRunning->next;//make pHead the next process in the list
        currentRunning->next = NULL;//detach process from processReadyQueue
    }

}
/////////////////////////////////////////////////////////////////
void firstComeFirstServe(struct event* eve) {
    process* iterator;
    iterator = pHead;

    cout << "Arrival event arrived of time " << eve->time << " with PID of " << eve->p->pid << endl;
    if (currentRunning == NULL) {//if no process is being serviced
        currentRunning = eve->p;//set serverBusy pointer to the process being serviced
        cout << "ARRIVAL HANDLER: Process ID: " << eve->p->pid << " being sent to CPU" << endl;
        serviceTime = sClock + eve->p->burstTime;//get service time of the process by adding burst + clocktime
        eve->p->serviceTime = serviceTime;
        schedule_event(DEP, serviceTime, eve->p);//schedule an event when the process will complete
    }

    else {//if there is process being serviced, put it Process Ready Queue
        if (iterator == NULL) {//if list is empty
            pHead = eve->p;
        }
        else {
            while (iterator->next != NULL) {//move iterator to end of Process Queue
                iterator = iterator->next;
            }
            iterator->next = eve->p;//Add process to end of Process Ready Queue
        }
        eve->p->arrivalTime = sClock;//record the arrival time of process in the readyQueue
        eve->p->startQueueTime = sClock;
    }
}
//////////////////////////////////////////////////////////////////
void shortJobFirst(struct event* eve) {
    process* processIter;
    event* temp;
    event* prev;
    processIter = pHead;
    temp = head;
    process* newProcess = eve->p;

    cout << "Arrival event arrived of time " << eve->time << " with PID of " << eve->p->pid << endl;
    if (currentRunning == NULL) {//if no process is being serviced
        currentRunning = eve->p;//set serverBusy pointer to the process being serviced
        cout << "ARRIVAL HANDLER: Process ID: " << eve->p->pid << " being sent to CPU" << endl;
        serviceTime = sClock + eve->p->burstTime;//get service time of the process by adding burst + clocktime
        eve->p->serviceTime = serviceTime;
        schedule_event(DEP, serviceTime, eve->p);//schedule an event when the process will complete
    }
    else {//CPU busy, if process has less burst time than remaining time of process, preempt, otherwise put in ready queue
        float timeRemaining = currentRunning->serviceTime - sClock;//get remaining run time of running process

        if (newProcess->burstTime < timeRemaining) {
            cout << "Process ID: " << newProcess->pid << " Preempting running Process ID: " << currentRunning->pid
                 << endl;
            currentRunning->burstTime = timeRemaining;
            if (temp != NULL &&
                temp->time == currentRunning->serviceTime) {//find DEP event in linked list and delete it
                head = temp->next;
                delete temp;
            } else {
                while (temp != NULL && temp->time != currentRunning->serviceTime) {
                    prev = temp;
                    temp = temp->next;
                }
                prev->next = temp->next;
                delete temp;
            }
            //Inserting the old process at the head of the list
            if (processIter == NULL || processIter->burstTime > currentRunning->burstTime) {//case for head
                currentRunning->next = processIter;
                pHead = currentRunning;
            } else {//Inserting the old process into sorted process ready queue
                while (processIter->next != NULL && processIter->next->burstTime <= currentRunning->burstTime) {
                    processIter = processIter->next;
                }
                currentRunning->next = processIter->next;
                processIter->next = currentRunning;
            }
            currentRunning->arrivalTime = sClock;//record the arrival of the old process in the readyQueue
            currentRunning = newProcess;//Set new process as currentRunning
            serviceTime = sClock + currentRunning->burstTime;//get service time of the process
            currentRunning->serviceTime = serviceTime;
            schedule_event(DEP, serviceTime,
                           currentRunning);//schedule departure event, assign service time of process, assign process to event
        } else {//Current process remains in CPU and new process gets inserted into sorted Process Ready Queue
            if (processIter == NULL || processIter->burstTime > newProcess->burstTime) {//case for head
                newProcess->next = processIter;
                pHead = newProcess;
            } else {//Inserting the new process into sorted process ready queue
                while (processIter->next != NULL && processIter->next->burstTime <= newProcess->burstTime) {
                    processIter = processIter->next;
                }
                newProcess->next = processIter->next;
                processIter->next = newProcess;
            }
            newProcess->arrivalTime = sClock;
            newProcess->startQueueTime = sClock;
        }
    }
}
//////////////////////////////////////////////////////////////
void roundRobin(struct event* eve) {
    process* iterator;
    iterator = pHead;

    cout << "Arrival event arrived of time " << eve->time << " with PID of " << eve->p->pid << endl;
    if (currentRunning == NULL) {//if no process is being serviced
        currentRunning = eve->p;//set serverBusy pointer to the process being serviced
        cout << "ARRIVAL HANDLER: Process ID: " << eve->p->pid << " being sent to CPU" << endl;

        if (eve->p->burstTime > quantum) {//if the burst time of the process is greater than quantum time
            serviceTime = quantum + sClock;//Assign system time process will complete.
            eve->p->serviceTime = serviceTime;
            schedule_event(TSO, serviceTime, eve->p);//schedule a timeslice event.
            pHead = currentRunning->next;//make pHead the next process in the list
            currentRunning->next = NULL;//detach process from processReadyQueue
        }
        else {//if the process will finish before the quantum time
            serviceTime = sClock + eve->p->burstTime;
            eve->p->serviceTime = serviceTime;
            schedule_event(DEP, serviceTime, eve->p);
            pHead = currentRunning->next;//make pHead the next process in the list
            currentRunning->next = NULL;//detach process from processReadyQueue
        }
    }

    else {//if there is process being serviced, put it Process Ready Queue
        cout << "In roundRobin, process put into ready queue" << endl;
        if (iterator == NULL) {//if list is empty
            pHead = eve->p;
        }
        else {
            while (iterator->next != NULL) {//move iterator to end of Process Queue
                iterator = iterator->next;
            }
            iterator->next = eve->p;//Add process to end of Process Ready Queue
        }
        eve->p->arrivalTime = sClock;//record the arrival time of process in the readyQueue
        eve->p->startQueueTime = sClock;
    }
}