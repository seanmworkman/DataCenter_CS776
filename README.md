# DataCenter_CS776

## Description
This program simulates a datacenter that uses rotating shifts of vehicles parked in parking spots that walk back up to a datacenter controller. It handles the shifts switching and migrating all active VMs of departing cars to VMs of arriving cars, along with any active jobs. The simulation runs for until a total of ~1500 jobs have been completed. The job durations and data sizes are generated using C++ library uniform distributions. 

## How to execute the program
- Compile the program from the datacenter.cpp file
- Run the executable created
- When prompted to select a migration strategy, type `0` `1` `2` OR `3` to run the respective strategies then hit enter
- A running clock and tally of completed jobs will be desplayed as the program runs
- At the end you will be presented with the total number of jobs completed and the total time it took in microseconds

## Technical Aspects
To make the simulation run faster the specified times are broken down to a ratio of 3,600:1
- 1 hour (3,600,000 ms; 3,600,000,000 microseconds) = 1 second (1,000 ms; 1,000,000 microseconds)
- 1 second (1,000 ms; 1,000,000 microseconds) = 2.7778 ms (~2778 microseconds)


Different VM migration strategies are defined in the Report as well as comments above the individual functions
- migrateVM_S0: Strategy0
- migrateVM_S1: Strategy1
- migrateVM_S2: Strategy2
- migrateVM_S3: Strategy3


Defined structs
// Datacenter Controller
struct datacenterController
{
    int name;
    // int rc[4];
};

// Region Controller
struct regionController
{
    int name;
    int dc;
    // int gc[4];
};

// Group Controller
struct groupController
{
    int name;
    int rc;
    int dc;
    // int ap[4];
};

// Access Point
struct accessPoint
{
    int name;
    int gc;
    int rc;
    int dc;
    // int ps[40];
};

// Parking Spot
struct parkingSpot
{
    int name;
    int ap;
    int gc;
    int rc;
    int dc;
    int vehicle;
};

// Vehicle 
struct vehicle
{
    int name;
    int ps;
    string status; // available or busy
    // Arrival/Departure times are by the hour based on a 24 hour clock
    int arrivalTime;
    int departureTime;
    int activeJob;
};

// Job
struct job
{
    int name;
    int generatedDataSize;
    int duration;
    bool open;
    int startTime;
};

// Datacenter Manager
struct datacenterManager
{
    int ps;
    int vehicle;
};