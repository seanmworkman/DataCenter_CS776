# DataCenter_CS776

To make the simulation run faster the specified times are broken down to a ratio of 3,600:1
- 1 hour (3,600,000 ms; 3,600,000,000 microseconds) = 1 second (1,000 ms; 1,000,000 microseconds)
- 1 second (1,000 ms; 1,000,000 microseconds) = 2.7778 ms (~2778 microseconds)


Strategy 1 - Ample lead up time to ensure everything is migrated:
- At the 30 minute (simulation: 0.5 seconds) mark everyone in that shift begins to dump there progress to another available car or to another car's hard drive if none are available.
- As new cars come in those left over jobs are picked up and completed 

Strategy 2 - 
- Every 10 minutes (simulation: 0.1 seconds) every car in the shift that is about to leave is checked to see how much time the migration will take
- If one is already within 1 minute (simulation: 0.01 seconds) of not being able to get everything transferred the transfer will begin 
- The rest will schedule themselves based on who needs the most time (i.e. how much data they have)



TODO:
- When migrating to a different VM add check for closest arrival time to current time
    - Possibly another strategy for VM migration????????
- When migrating to a different VM add checks for if there is no option in the same cluster 
    - Check other clusters in the GC
    - Check other GCs
    - Check other RCs
    - Check entire DC

VM Migration Strategy 3:
- Store all available cars in a vector and select from there to migrate to


Don't assign jobs to vehicles if there is under a certain percentage of available vehicles 
- Start at 50%



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





Problems:
- Jobs stop finishing at a certain point