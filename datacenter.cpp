#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <cstdlib>
using namespace std::chrono; 
using namespace std;

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

// vector<int> availableSpots;
// 500MB
const int JOB_INPUT = 500;
// 1Gbps normalized to Mb
const int WIRED_BANDWIDTH = 1000;
// 54Mbps 
const int WIRELESS_BANDWIDTH = 54;
// 1 hour (simulated)
const int ONE_HOUR = 1000000;
// 1 second (simulated)
const int ONE_SECOND = 2778;

const string AVAILABLE = "AVAILABLE";

const string BUSY = "BUSY";

const int MIGRATION_THRESHOLD = 900000;

// helper function to convert GB to MB
double getGBtoMB(double gb) {
    return gb * 1000;
}

// helper function to convert MB to GB
double getMBtoGB(double mb) {
    return mb / 1000;
}

// helper function to convert bytes into bits
int bytesToBits(int bytes) {
    return bytes * 8;
}

// helper function for finding the arrival time closest to the target time
int getClosest(vector<int> arrivalTimes, int target) {
    vector<int> diffs;
    for (int i = 0; i < arrivalTimes.size(); i++) {
        diffs.push_back(abs(arrivalTimes[i] - target));
    }
    int min = diffs[0];
    int minIndex = 0;
    for (int i = 0; i < diffs.size(); i++) {
        if (diffs[i] < min) {
            min = diffs[i];
            minIndex = i;
        }
    }
    return minIndex;
}

// helper function to check what percent of active vehicles are available
int getPercentOfAvailableVehicles(vector<vehicle> vehicles) {
    int active_available = 0;
    int active = 0;
    for (int i = 0; i < vehicles.size(); i++) {
        if (vehicles[i].arrivalTime != -1) {
            active++;
            if (vehicles[i].status == AVAILABLE) {
                active_available++;
            }
        }
        
    }
    return (int) ((active_available * 100) / active);
}

// helper function for simulating transfer latencies
void transferDelay(int transferTime) {
    struct timespec timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = transferTime * 1000;
    nanosleep(&timeout, NULL);
}

// Print function for vehicles 
void printVehicles(vector<vehicle> vehicles, vector<job> jobs) {
    // for (int i = 0; i < vehicles.size(); i++) {
    for (int i = 0; i < 1; i++) {
        cout << "Vehicle: " << vehicles[i].name << endl;
        cout << "   - Parking Spot: " << vehicles[i].ps << endl;
        cout << "   - Status: " << vehicles[i].status << endl;
        cout << "   - Arrival Time: " << vehicles[i].arrivalTime << endl;
        cout << "   - Departure Time: " << vehicles[i].departureTime << endl;
        cout << "   - Active Job: " << vehicles[i].activeJob << endl;
        if (vehicles[i].activeJob != -1) {
            cout << "   Active Job: " << vehicles[i].activeJob << " | Job: " << jobs[vehicles[i].activeJob].name << endl;
            cout << "Job: " << jobs[vehicles[i].activeJob].name << endl;
            cout << "   - Data Size: " << jobs[vehicles[i].activeJob].generatedDataSize << endl;
            cout << "   - Duration: " << jobs[vehicles[i].activeJob].duration << endl;
            cout << "   - Open: " << jobs[vehicles[i].activeJob].open << endl;
        }
        else {
            cout << "Job: NONE" << endl;
        }
    }
}

// Get generated data uniformly distributed in [0.25, 1.0] GB
int getGeneratedDataSizeInMB() {
    double lower = 0.25;
    double upper = 1;
    uniform_real_distribution<double> unif(lower, upper);
    default_random_engine re;
    double dataSize = unif(re); 
    int dsInMB = getGBtoMB(dataSize);
    return (int) dsInMB;
}

// Uniformaly distributed in [3, 24] hours
int getJobDuration() {
    int duration = rand() % (24 - 3 + 1);
    return duration;
}

// Takes the data size and the transfer type (wired or wireless) 
//      and gives the time it will take to transfer
int getTransferTime(int dataSize, bool wired) {
    int transferTime = 0;
    if (wired) {
        transferTime = ((double) dataSize / (double) WIRED_BANDWIDTH) * (double) ONE_SECOND;
    }
    else {
        transferTime = ((double) dataSize / (double) WIRELESS_BANDWIDTH) * (double) ONE_SECOND;
    }
    return transferTime;
}

// Creates a dc 
datacenterController createDC() {
    datacenterController dc;
    dc.name = 0;
    // dc.rc = [0, 1, 2, 3];
    return dc;
}

// Creates a vector of rcs with the previously created dc as its dc
vector<regionController> createRCs(datacenterController dc) {
    vector<regionController> rcs;
    
    for (int i = 0; i < 4; i++) {
        regionController rc;
        rc.name = i;
        rc.dc = dc.name;
        rcs.push_back(rc);
    }

    return rcs;
}

// Creates a vector of gcs with the previously created rcs as its respective rcs
vector<groupController> createGCs(vector<regionController> rcs) {
    vector<groupController> gcs;
    int gcCounter = 0;
    for (int i = 0; i < rcs.size(); i++) {
        for (int j = 0; j < 4; j++) {
            groupController gc;
            gc.name = gcCounter;
            gc.rc = rcs[i].name;
            gc.dc = rcs[i].dc;
            gcs.push_back(gc);
            gcCounter++;
        }
    }

    return gcs;
}

// Creates a vector of aps with the previously created gcs as its respective gcs
vector<accessPoint> createAPs(vector<groupController> gcs) {
    vector<accessPoint> aps;
    int apCounter = 0;
    for (int i = 0; i < gcs.size(); i++) {
        for (int j = 0; j < 4; j++) {
            accessPoint ap;
            ap.name = apCounter;
            ap.gc = gcs[i].name;
            ap.rc = gcs[i].rc;
            ap.dc = gcs[i].dc;
            aps.push_back(ap);
            apCounter++;
        }
    }

    return aps;
}


vector<parkingSpot> createPSs(vector<accessPoint> aps) {
    vector<parkingSpot> pss;
    int psCounter = 0;
    for (int i = 0; i < aps.size(); i++) {
        for (int j = 0; j < 40; j++) {
            parkingSpot ps;
            ps.name = psCounter;
            ps.ap = aps[i].name;
            ps.gc = aps[i].gc;
            ps.rc = aps[i].rc;
            ps.dc = aps[i].dc;
            ps.vehicle = -1;
            pss.push_back(ps);
            psCounter++;
        }
    }

    return pss;
}

vector<vehicle> createVehicles() {
    vector<vehicle> vehicles;
    for (int i = 0; i < 7680; i++) {
        vehicle v;
        v.name = i;
        v.ps = -1;
        v.status = AVAILABLE;
        v.arrivalTime = -1;
        v.departureTime = -1;
        v.activeJob = -1;
        vehicles.push_back(v);
    }

    return vehicles;
}


// Create enough jobs to avoid having to create more later
vector<job> createJobs() {
    vector<job> jobs;
    for (int i = 0; i < 7680; i++) {
        job curJob;
        curJob.name = i;
        curJob.generatedDataSize = getGeneratedDataSizeInMB();
        curJob.duration = getJobDuration();
        curJob.startTime = -1;

        jobs.push_back(curJob);
    }
    return jobs;
}

// Assign a job to the specified vehicle
void assignJobToVehicle(vector<job> &jobs, vector<vehicle> &vehicles, int vIndex, int curTime) {
    // If there are no jobs remaining create more
    // if (jobs.empty()) {
    //     jobs = createJobs();
    // }

    for (int i = 0; i < jobs.size(); i++) {
        // Find an open job, assign it to the vehicle and set the vehicle's status to busy
        if (jobs[i].open) {
            // vehicles[vIndex].activeJob = jobs[i].name;
            vehicles[vIndex].activeJob = i;
            vehicles[vIndex].status = BUSY;
            jobs[i].open = false;
            jobs[i].startTime = curTime;
            // TODO: add delay for JOB_INPUT download
            break;
        }
    }
}

// At the beginning of the simulation the parking lot is full 
// Add a vehicle to every parking spot randomly from the vector of vehicles 
// Each set of 320 vehicles should have an arrival time that is offset by an hour for each group
//      This reperesents each 320 vehicle group arriving for their 8 hour shifts at different times
void setupSimulation(vector<parkingSpot> &parkingSpots, vector<vehicle> &vehicles, vector<job> &jobs) {
    vector<int> numbers;
    // Create a vector of numbers 0-7679
    for (int i = 0; i < parkingSpots.size(); i++) {
        numbers.push_back(i);
    }
    // Shuffle those numbers 
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(numbers.begin(), numbers.end(), std::default_random_engine(seed));
    // Start at -1 because 0 % 320 == 0 and we want arrival to begin at 0
    int arrival = -1;
    for (int i = 0; i < parkingSpots.size(); i++) {
        // Increase the arrival time if it is a new group (i.e. every 320)
        if (i % 320 == 0) {
            arrival++;
        }
        // Add the vehicle to a random parking spot
        parkingSpots[numbers[i]].vehicle = i; 
        vehicles[i].ps = numbers[i];
        vehicles[i].arrivalTime = arrival;
        // Assign jobs to half the vehicles 
        if (i % 2 == 0) {
            // At the beginning of the simulation assume that jobs are started at arrival time
            assignJobToVehicle(jobs, vehicles, i, arrival);
        }
    }
}

int getDepartureTime(int arrivalTime) {
    int dt = arrivalTime + 8;
    return dt;
}

// Transfer a completed job to the vehicle's respective datacenter manager (co-located with the DC)
void transferCompletedJobToDM(vector<parkingSpot> &parkingSpots, vector<vehicle> &vehicles, int vIndex, vector<job> &jobs, vector<job> &completedJobs) {
    // Get the parking spot that the vehicle is in
    parkingSpot ps = parkingSpots[vehicles[vIndex].ps];
    int dataSize = 0;
    // cout << "transferCompletedJobToDM(1) " << dataSize << endl;
    if (vehicles[vIndex].activeJob != -1) {
        dataSize = jobs[vehicles[vIndex].activeJob].generatedDataSize;
    }
    // cout << "transferCompletedJobToDM(2) " << dataSize << endl;
    
    // Parking Spot to Access Point (wireless)
    transferDelay(getTransferTime(dataSize, false));

    // Access Point to Group Controller (wired)
    transferDelay(getTransferTime(dataSize, true));

    // Group Controller to Region Controller (wired)
    transferDelay(getTransferTime(dataSize, true));

    // Region Controller to Datacenter Manager (wired)
    transferDelay(getTransferTime(dataSize, true));

    // The job is completed and transfered to the datacenter manager
    // Remove the job from the jobs vector and add it to the completedJobs vector
    // Make sure the vehicle has a job (shouldn't not have a job, just to be safe)
    // cout << "Vehivle.activeJob: " << vehicles[vIndex].activeJob << endl;
    if (vehicles[vIndex].activeJob != -1) {
        completedJobs.push_back(jobs[vehicles[vIndex].activeJob]);
        // jobs.erase(jobs.begin() + vehicles[vIndex].activeJob);
        jobs[vehicles[vIndex].activeJob].open = false;
    }
    // cout << "transferCompletedJobToDM(3) " << dataSize << endl;
    // cout << "Job " << vehicles[vIndex].activeJob << " transfered to DM" << endl;
    // The vehicle is now available
    vehicles[vIndex].status = AVAILABLE;
    vehicles[vIndex].activeJob = -1;
    // cout << "transferCompletedJobToDM(4) " << dataSize << endl;
}

// Strategy 0 for migrating VMs; grab the first available vehicle
// return true if migration was successful, false otherwise
bool migrateVM_S0(vector<parkingSpot> &parkingSpots, vector<vehicle> &vehicles, int vIndex, vector<job> &jobs) {
    // Try to find an available vehicle in the datacenter to migrate to
    for (int i = 0; i < parkingSpots.size(); i++) {
        // Check for available vehicle in same cluster
        if (parkingSpots[i].name != vehicles[vIndex].ps &&
            parkingSpots[i].dc == parkingSpots[vehicles[vIndex].ps].dc &&
            parkingSpots[i].vehicle != -1 &&
            vehicles[parkingSpots[i].vehicle].status == AVAILABLE &&
            vehicles[parkingSpots[i].vehicle].arrivalTime != -1) {
                // Migrate job to new vehicle
                int dataSize = 0;
                if (vehicles[vIndex].activeJob != -1) {
                    dataSize = jobs[vehicles[vIndex].activeJob].generatedDataSize;
                }
                // In the same Cluster
                if (parkingSpots[i].ap == parkingSpots[vehicles[vIndex].ps].ap) {
                    // UPLOAD Parking Spot to Access Point (wireless)
                    transferDelay(getTransferTime(dataSize, false));
                    // DOWNLOAD Access Point to Parking Spot (wireless)
                    transferDelay(getTransferTime(dataSize, false));
                }
                // In the same Group
                else if (parkingSpots[i].gc == parkingSpots[vehicles[vIndex].ps].gc) {
                    // UPLOAD Parking Spot to Access Point (wireless)
                    transferDelay(getTransferTime(dataSize, false));
                    // UPLOAD Access Point to Group Controller (wired)
                    transferDelay(getTransferTime(dataSize, true));
                    // DOWNLOAD Group Controller to Access Point (wired)
                    transferDelay(getTransferTime(dataSize, true));
                    // DOWNLOAD Access Point to Parking Spot (wireless)
                    transferDelay(getTransferTime(dataSize, false));
                }
                // In the same Region
                else if (parkingSpots[i].rc == parkingSpots[vehicles[vIndex].ps].rc) {
                    // UPLOAD Parking Spot to Access Point (wireless)
                    transferDelay(getTransferTime(dataSize, false));
                    // UPLOAD Access Point to Group Controller (wired)
                    transferDelay(getTransferTime(dataSize, true));
                    // UPLOAD Group Controller to Region Controller (wired)
                    transferDelay(getTransferTime(dataSize, true));

                    // DOWNLOAD Region Controller to Group Controller (wired)
                    transferDelay(getTransferTime(dataSize, true));
                    // DOWNLOAD Group Controller to Access Point (wired)
                    transferDelay(getTransferTime(dataSize, true));
                    // DOWNLOAD Access Point to Parking Spot (wireless)
                    transferDelay(getTransferTime(dataSize, false));
                }
                // In the datacenter
                else {
                    // UPLOAD Parking Spot to Access Point (wireless)
                    transferDelay(getTransferTime(dataSize, false));
                    // UPLOAD Access Point to Group Controller (wired)
                    transferDelay(getTransferTime(dataSize, true));
                    // UPLOAD Group Controller to Region Controller (wired)
                    transferDelay(getTransferTime(dataSize, true));
                    // UPLOAD Region Controller to Datacenter Controller (wired)
                    transferDelay(getTransferTime(dataSize, true));

                    // DOWNLOAD Datacenter Controller to Region Controller (wired)
                    transferDelay(getTransferTime(dataSize, true));
                    // DOWNLOAD Region Controller to Group Controller (wired)
                    transferDelay(getTransferTime(dataSize, true));
                    // DOWNLOAD Group Controller to Access Point (wired)
                    transferDelay(getTransferTime(dataSize, true));
                    // DOWNLOAD Access Point to Parking Spot (wireless)
                    transferDelay(getTransferTime(dataSize, false));
                }
                
                vehicles[parkingSpots[i].vehicle].status = BUSY;
                vehicles[parkingSpots[i].vehicle].ps = parkingSpots[i].name;
                vehicles[parkingSpots[i].vehicle].activeJob = vehicles[vIndex].activeJob;


                // Clear out vehicle leaving
                vehicles[vIndex].activeJob = -1;
                vehicles[vIndex].departureTime = getDepartureTime(vehicles[vIndex].arrivalTime);
                // vehicles[vIndex].arrivalTime = -1;
                vehicles[vIndex].status = AVAILABLE;
                vehicles[vIndex].ps = -1;

                return true;
        }
    }

    // Clear out parking spot
    parkingSpots[vehicles[vIndex].ps].vehicle = -1;

    // Migration failed so clear out vehicle leaving, reset job, and open up job
    jobs[vehicles[vIndex].activeJob].startTime = -1;
    jobs[vehicles[vIndex].activeJob].open = true;
    vehicles[vIndex].activeJob = -1;
    vehicles[vIndex].departureTime = getDepartureTime(vehicles[vIndex].arrivalTime);
    // vehicles[vIndex].arrivalTime = -1;
    vehicles[vIndex].status = AVAILABLE;
    vehicles[vIndex].ps = -1;

    return false;
}

// Strategy 1 for migrating VMs; grab the first available vehicle 
//      in the same cluster, 
//      If none available check the rest of the GC,
//      If none available check the rest of the RC,
//      If none available check the rest of the DC
// return true if migration was successful, false otherwise
bool migrateVM_S1(
        vector<parkingSpot> &parkingSpots, 
        vector<vehicle> &vehicles, 
        int vIndex, 
        vector<job> &jobs
    ) {
    // If the vehicle has no job there is nothing to migrate
    if (vehicles[vIndex].status == AVAILABLE || vehicles[vIndex].activeJob == -1) {
        if (vehicles[vIndex].ps != -1) parkingSpots[vehicles[vIndex].ps].vehicle = -1;
        // Clear out vehicle leaving
        vehicles[vIndex].activeJob = -1;
        vehicles[vIndex].departureTime = getDepartureTime(vehicles[vIndex].arrivalTime);
        // vehicles[vIndex].arrivalTime = -1;
        vehicles[vIndex].status = AVAILABLE;
        vehicles[vIndex].ps = -1;

        return true;
    }
    // Try to find an available vehicle in the same cluster to migrate to
    for (int i = 0; i < parkingSpots.size(); i++) {
        // Check for available vehicle in same cluster
        if (parkingSpots[i].name != vehicles[vIndex].ps &&
            parkingSpots[i].ap == parkingSpots[vehicles[vIndex].ps].ap &&
            parkingSpots[i].vehicle != -1 &&
            vehicles[parkingSpots[i].vehicle].status == AVAILABLE &&
            vehicles[parkingSpots[i].vehicle].arrivalTime != -1) {
                // Migrate job to new vehicle
                int dataSize = 0;
                if (vehicles[vIndex].activeJob != -1) {
                    dataSize = jobs[vehicles[vIndex].activeJob].generatedDataSize;
                }
                // UPLOAD Parking Spot to Access Point (wireless)
                transferDelay(getTransferTime(dataSize, false));
                // DOWNLOAD Access Point to Parking Spot (wireless)
                transferDelay(getTransferTime(dataSize, false));
                vehicles[parkingSpots[i].vehicle].status = BUSY;
                vehicles[parkingSpots[i].vehicle].ps = parkingSpots[i].name;
                vehicles[parkingSpots[i].vehicle].activeJob = vehicles[vIndex].activeJob;
                

                // Clear out vehicle leaving
                vehicles[vIndex].activeJob = -1;
                vehicles[vIndex].departureTime = getDepartureTime(vehicles[vIndex].arrivalTime);
                // vehicles[vIndex].arrivalTime = -1;
                vehicles[vIndex].status = AVAILABLE;
                vehicles[vIndex].ps = -1;

                

                return true;
        }
    }
    
    // Try to find an available vehicle in the same group to migrate to
    for (int i = 0; i < parkingSpots.size(); i++) {
        // Check for available vehicle in same cluster
        if (parkingSpots[i].name != vehicles[vIndex].ps &&
            parkingSpots[i].gc == parkingSpots[vehicles[vIndex].ps].gc &&
            parkingSpots[i].vehicle != -1 &&
            vehicles[parkingSpots[i].vehicle].status == AVAILABLE &&
            vehicles[parkingSpots[i].vehicle].arrivalTime != -1) {
                // Migrate job to new vehicle
                int dataSize = 0;
                if (vehicles[vIndex].activeJob != -1) {
                    dataSize = jobs[vehicles[vIndex].activeJob].generatedDataSize;
                }
                // UPLOAD Parking Spot to Access Point (wireless)
                transferDelay(getTransferTime(dataSize, false));
                // UPLOAD Access Point to Group Controller (wired)
                transferDelay(getTransferTime(dataSize, true));
                // DOWNLOAD Group Controller to Access Point (wired)
                transferDelay(getTransferTime(dataSize, true));
                // DOWNLOAD Access Point to Parking Spot (wireless)
                transferDelay(getTransferTime(dataSize, false));
                
                vehicles[parkingSpots[i].vehicle].status = BUSY;
                vehicles[parkingSpots[i].vehicle].ps = parkingSpots[i].name;
                vehicles[parkingSpots[i].vehicle].activeJob = vehicles[vIndex].activeJob;


                // Clear out vehicle leaving
                vehicles[vIndex].activeJob = -1;
                vehicles[vIndex].departureTime = getDepartureTime(vehicles[vIndex].arrivalTime);
                // vehicles[vIndex].arrivalTime = -1;
                vehicles[vIndex].status = AVAILABLE;
                vehicles[vIndex].ps = -1;

                return true;
        }
    }

    // Try to find an available vehicle in the same region to migrate to
    for (int i = 0; i < parkingSpots.size(); i++) {
        // Check for available vehicle in same cluster
        if (parkingSpots[i].name != vehicles[vIndex].ps &&
            parkingSpots[i].rc == parkingSpots[vehicles[vIndex].ps].rc &&
            parkingSpots[i].vehicle != -1 &&
            vehicles[parkingSpots[i].vehicle].status == AVAILABLE &&
            vehicles[parkingSpots[i].vehicle].arrivalTime != -1) {
                // Migrate job to new vehicle
                int dataSize = 0;
                if (vehicles[vIndex].activeJob != -1) {
                    dataSize = jobs[vehicles[vIndex].activeJob].generatedDataSize;
                }
                // UPLOAD Parking Spot to Access Point (wireless)
                transferDelay(getTransferTime(dataSize, false));
                // UPLOAD Access Point to Group Controller (wired)
                transferDelay(getTransferTime(dataSize, true));
                // UPLOAD Group Controller to Region Controller (wired)
                transferDelay(getTransferTime(dataSize, true));

                // DOWNLOAD Region Controller to Group Controller (wired)
                transferDelay(getTransferTime(dataSize, true));
                // DOWNLOAD Group Controller to Access Point (wired)
                transferDelay(getTransferTime(dataSize, true));
                // DOWNLOAD Access Point to Parking Spot (wireless)
                transferDelay(getTransferTime(dataSize, false));
                
                vehicles[parkingSpots[i].vehicle].status = BUSY;
                vehicles[parkingSpots[i].vehicle].ps = parkingSpots[i].name;
                vehicles[parkingSpots[i].vehicle].activeJob = vehicles[vIndex].activeJob;

                // Clear out vehicle leaving
                vehicles[vIndex].activeJob = -1;
                vehicles[vIndex].departureTime = getDepartureTime(vehicles[vIndex].arrivalTime);
                // vehicles[vIndex].arrivalTime = -1;
                vehicles[vIndex].status = AVAILABLE;
                vehicles[vIndex].ps = -1;

                return true;
        }
    }

    // Try to find an available vehicle in the datacenter to migrate to
    for (int i = 0; i < parkingSpots.size(); i++) {
        // Check for available vehicle in same cluster
        if (parkingSpots[i].name != vehicles[vIndex].ps &&
            parkingSpots[i].dc == parkingSpots[vehicles[vIndex].ps].dc &&
            parkingSpots[i].vehicle != -1 &&
            vehicles[parkingSpots[i].vehicle].status == AVAILABLE &&
            vehicles[parkingSpots[i].vehicle].arrivalTime != -1) {
                // Migrate job to new vehicle
                int dataSize = 0;
                if (vehicles[vIndex].activeJob != -1) {
                    dataSize = jobs[vehicles[vIndex].activeJob].generatedDataSize;
                }
                // UPLOAD Parking Spot to Access Point (wireless)
                transferDelay(getTransferTime(dataSize, false));
                // UPLOAD Access Point to Group Controller (wired)
                transferDelay(getTransferTime(dataSize, true));
                // UPLOAD Group Controller to Region Controller (wired)
                transferDelay(getTransferTime(dataSize, true));

                // DOWNLOAD Region Controller to Group Controller (wired)
                transferDelay(getTransferTime(dataSize, true));
                // DOWNLOAD Group Controller to Access Point (wired)
                transferDelay(getTransferTime(dataSize, true));
                // DOWNLOAD Access Point to Parking Spot (wireless)
                transferDelay(getTransferTime(dataSize, false));
                
                vehicles[parkingSpots[i].vehicle].status = BUSY;
                vehicles[parkingSpots[i].vehicle].ps = parkingSpots[i].name;
                vehicles[parkingSpots[i].vehicle].activeJob = vehicles[vIndex].activeJob;


                // Clear out vehicle leaving
                vehicles[vIndex].activeJob = -1;
                vehicles[vIndex].departureTime = getDepartureTime(vehicles[vIndex].arrivalTime);
                // vehicles[vIndex].arrivalTime = -1;
                vehicles[vIndex].status = AVAILABLE;
                vehicles[vIndex].ps = -1;

                return true;
        }
    }

    // Clear out parking spot
    parkingSpots[vehicles[vIndex].ps].vehicle = -1;

    // Migration failed so clear out vehicle leaving, reset job, and open up job
    jobs[vehicles[vIndex].activeJob].startTime = -1;
    jobs[vehicles[vIndex].activeJob].open = true;
    vehicles[vIndex].activeJob = -1;
    vehicles[vIndex].departureTime = getDepartureTime(vehicles[vIndex].arrivalTime);
    // vehicles[vIndex].arrivalTime = -1;
    vehicles[vIndex].status = AVAILABLE;
    vehicles[vIndex].ps = -1;

    // cout << "Migration Failed" << endl;

    return false;
}



// Strategy 2 for migrating VMs; grab the vehicle with the closest arrival time to the current time
//      in the same cluster, 
//      If none available check the rest of the GC,
//      If none available check the rest of the RC,
//      If none available check the rest of the DC
// return true if migration was successful, false otherwise
bool migrateVM_S2(
        vector<parkingSpot> &parkingSpots, 
        vector<vehicle> &vehicles, 
        int vIndex, 
        vector<job> &jobs,
        int curTime
    ) {
    
    // If the vehicle has no job there is nothing to migrate
    if (vehicles[vIndex].status == AVAILABLE || vehicles[vIndex].activeJob == -1) {
        if (vehicles[vIndex].ps != -1) parkingSpots[vehicles[vIndex].ps].vehicle = -1;
        // Clear out vehicle leaving
        vehicles[vIndex].activeJob = -1;
        vehicles[vIndex].departureTime = getDepartureTime(vehicles[vIndex].arrivalTime);
        // vehicles[vIndex].arrivalTime = -1;
        vehicles[vIndex].status = AVAILABLE;
        vehicles[vIndex].ps = -1;

        return true;
    }
    vector<int> options;
    vector<int> arrivalTimes;

    // Try to find an available vehicle in the same cluster to migrate to
    for (int i = 0; i < parkingSpots.size(); i++) {
        // Check for available vehicle in same cluster
        if (parkingSpots[i].name != vehicles[vIndex].ps &&
            parkingSpots[i].ap == parkingSpots[vehicles[vIndex].ps].ap &&
            parkingSpots[i].vehicle != -1 &&
            vehicles[parkingSpots[i].vehicle].status == AVAILABLE &&
            vehicles[parkingSpots[i].vehicle].arrivalTime != -1) {
                // Store all available vehicles arrival times 
                options.push_back(i);
                arrivalTimes.push_back(vehicles[parkingSpots[i].vehicle].arrivalTime);
        }
    }

    if (!options.empty() && !arrivalTimes.empty()) {
        int idealIndex = options[getClosest(arrivalTimes, curTime)];
        // Migrate job to new vehicle
        int dataSize = 0;
        if (vehicles[vIndex].activeJob != -1) {
            dataSize = jobs[vehicles[vIndex].activeJob].generatedDataSize;
        }
        // UPLOAD Parking Spot to Access Point (wireless)
        transferDelay(getTransferTime(dataSize, false));
        // DOWNLOAD Access Point to Parking Spot (wireless)
        transferDelay(getTransferTime(dataSize, false));
        vehicles[parkingSpots[idealIndex].vehicle].status = BUSY;
        vehicles[parkingSpots[idealIndex].vehicle].ps = parkingSpots[idealIndex].name;
        vehicles[parkingSpots[idealIndex].vehicle].activeJob = vehicles[vIndex].activeJob;


        // Clear out vehicle leaving
        vehicles[vIndex].activeJob = -1;
        vehicles[vIndex].departureTime = getDepartureTime(vehicles[vIndex].arrivalTime);
        // vehicles[vIndex].arrivalTime = -1;
        vehicles[vIndex].status = AVAILABLE;
        vehicles[vIndex].ps = -1;

        return true;
    }

    options.clear();
    arrivalTimes.clear();

    // Try to find an available vehicle in the same group to migrate to
    for (int i = 0; i < parkingSpots.size(); i++) {
        // Check for available vehicle in same cluster
        if (parkingSpots[i].name != vehicles[vIndex].ps &&
            parkingSpots[i].gc == parkingSpots[vehicles[vIndex].ps].gc &&
            parkingSpots[i].vehicle != -1 &&
            vehicles[parkingSpots[i].vehicle].status == AVAILABLE &&
            vehicles[parkingSpots[i].vehicle].arrivalTime != -1) {
                // Store all available vehicles arrival times 
                options.push_back(i);
                arrivalTimes.push_back(vehicles[parkingSpots[i].vehicle].arrivalTime);
        }
    }

    if (!options.empty() && !arrivalTimes.empty()) {
        int idealIndex = options[getClosest(arrivalTimes, curTime)];
        // Migrate job to new vehicle
        int dataSize = 0;
        if (vehicles[vIndex].activeJob != -1) {
            dataSize = jobs[vehicles[vIndex].activeJob].generatedDataSize;
        }
        // UPLOAD Parking Spot to Access Point (wireless)
        transferDelay(getTransferTime(dataSize, false));
        // UPLOAD Access Point to Group Controller (wired)
        transferDelay(getTransferTime(dataSize, true));
        // DOWNLOAD Group Controller to Access Point (wired)
        transferDelay(getTransferTime(dataSize, true));
        // DOWNLOAD Access Point to Parking Spot (wireless)
        transferDelay(getTransferTime(dataSize, false));
        vehicles[parkingSpots[idealIndex].vehicle].status = BUSY;
        vehicles[parkingSpots[idealIndex].vehicle].ps = parkingSpots[idealIndex].name;
        vehicles[parkingSpots[idealIndex].vehicle].activeJob = vehicles[vIndex].activeJob;


        // Clear out vehicle leaving
        vehicles[vIndex].activeJob = -1;
        vehicles[vIndex].departureTime = getDepartureTime(vehicles[vIndex].arrivalTime);
        // vehicles[vIndex].arrivalTime = -1;
        vehicles[vIndex].status = AVAILABLE;
        vehicles[vIndex].ps = -1;

        return true;
    }

    options.clear();
    arrivalTimes.clear();

    // Try to find an available vehicle in the same region to migrate to
    for (int i = 0; i < parkingSpots.size(); i++) {
        // Check for available vehicle in same cluster
        if (parkingSpots[i].name != vehicles[vIndex].ps &&
            parkingSpots[i].rc == parkingSpots[vehicles[vIndex].ps].rc &&
            parkingSpots[i].vehicle != -1 &&
            vehicles[parkingSpots[i].vehicle].status == AVAILABLE &&
            vehicles[parkingSpots[i].vehicle].arrivalTime != -1) {
                // Store all available vehicles arrival times 
                options.push_back(i);
                arrivalTimes.push_back(vehicles[parkingSpots[i].vehicle].arrivalTime);
        }
    }

    if (!options.empty() && !arrivalTimes.empty()) {
        int idealIndex = options[getClosest(arrivalTimes, curTime)];
        // Migrate job to new vehicle
        int dataSize = 0;
        if (vehicles[vIndex].activeJob != -1) {
            dataSize = jobs[vehicles[vIndex].activeJob].generatedDataSize;
        }
        // UPLOAD Parking Spot to Access Point (wireless)
        transferDelay(getTransferTime(dataSize, false));
        // UPLOAD Access Point to Group Controller (wired)
        transferDelay(getTransferTime(dataSize, true));
        // UPLOAD Group Controller to Region Controller (wired)
        transferDelay(getTransferTime(dataSize, true));

        // DOWNLOAD Region Controller to Group Controller (wired)
        transferDelay(getTransferTime(dataSize, true));
        // DOWNLOAD Group Controller to Access Point (wired)
        transferDelay(getTransferTime(dataSize, true));
        // DOWNLOAD Access Point to Parking Spot (wireless)
        transferDelay(getTransferTime(dataSize, false));
        vehicles[parkingSpots[idealIndex].vehicle].status = BUSY;
        vehicles[parkingSpots[idealIndex].vehicle].ps = parkingSpots[idealIndex].name;
        vehicles[parkingSpots[idealIndex].vehicle].activeJob = vehicles[vIndex].activeJob;


        // Clear out vehicle leaving
        vehicles[vIndex].activeJob = -1;
        vehicles[vIndex].departureTime = getDepartureTime(vehicles[vIndex].arrivalTime);
        // vehicles[vIndex].arrivalTime = -1;
        vehicles[vIndex].status = AVAILABLE;
        vehicles[vIndex].ps = -1;

        return true;
    }

    options.clear();
    arrivalTimes.clear();

    // Try to find an available vehicle in the datacenter to migrate to
    for (int i = 0; i < parkingSpots.size(); i++) {
        // Check for available vehicle in same cluster
        if (parkingSpots[i].name != vehicles[vIndex].ps &&
            parkingSpots[i].dc == parkingSpots[vehicles[vIndex].ps].dc &&
            parkingSpots[i].vehicle != -1 &&
            vehicles[parkingSpots[i].vehicle].status == AVAILABLE &&
            vehicles[parkingSpots[i].vehicle].arrivalTime != -1) {
                // Store all available vehicles arrival times 
                options.push_back(i);
                arrivalTimes.push_back(vehicles[parkingSpots[i].vehicle].arrivalTime);
        }
    }

    if (!options.empty() && !arrivalTimes.empty()) {
        int idealIndex = options[getClosest(arrivalTimes, curTime)];
        // Migrate job to new vehicle
        int dataSize = 0;
        if (vehicles[vIndex].activeJob != -1) {
            dataSize = jobs[vehicles[vIndex].activeJob].generatedDataSize;
        }
        // UPLOAD Parking Spot to Access Point (wireless)
        transferDelay(getTransferTime(dataSize, false));
        // UPLOAD Access Point to Group Controller (wired)
        transferDelay(getTransferTime(dataSize, true));
        // UPLOAD Group Controller to Region Controller (wired)
        transferDelay(getTransferTime(dataSize, true));

        // DOWNLOAD Region Controller to Group Controller (wired)
        transferDelay(getTransferTime(dataSize, true));
        // DOWNLOAD Group Controller to Access Point (wired)
        transferDelay(getTransferTime(dataSize, true));
        // DOWNLOAD Access Point to Parking Spot (wireless)
        transferDelay(getTransferTime(dataSize, false));
        vehicles[parkingSpots[idealIndex].vehicle].status = BUSY;
        vehicles[parkingSpots[idealIndex].vehicle].ps = parkingSpots[idealIndex].name;
        vehicles[parkingSpots[idealIndex].vehicle].activeJob = vehicles[vIndex].activeJob;


        // Clear out vehicle leaving
        vehicles[vIndex].activeJob = -1;
        vehicles[vIndex].departureTime = getDepartureTime(vehicles[vIndex].arrivalTime);
        // vehicles[vIndex].arrivalTime = -1;
        vehicles[vIndex].status = AVAILABLE;
        vehicles[vIndex].ps = -1;

        return true;
    }

    // Clear out parking spot
    parkingSpots[vehicles[vIndex].ps].vehicle = -1;

    // Migration failed so clear out vehicle leaving, reset job, and open up job
    jobs[vehicles[vIndex].activeJob].startTime = -1;
    jobs[vehicles[vIndex].activeJob].open = true;
    vehicles[vIndex].activeJob = -1;
    vehicles[vIndex].departureTime = getDepartureTime(vehicles[vIndex].arrivalTime);
    // vehicles[vIndex].arrivalTime = -1;
    vehicles[vIndex].status = AVAILABLE;
    vehicles[vIndex].ps = -1;

    return false;
}

// Strategy 3 for migrating VMs; 
// TODO: First check if its job can be transfered
// grab the vehicle with the closest arrival time to the current time
//      in the same cluster, 
//      If none available check the rest of the GC,
//      If none available check the rest of the RC,
//      If none available check the rest of the DC
// return true if migration was successful, false otherwise
bool migrateVM_S3(
        vector<parkingSpot> &parkingSpots, 
        vector<vehicle> &vehicles, 
        int vIndex, 
        vector<job> &jobs,
        int curTime,
        vector<job> &completedJobs,
        vector<int> transfers
    ) {
    

    // If the vehicle has no job there is nothing to migrate
    if (vehicles[vIndex].status == AVAILABLE || vehicles[vIndex].activeJob == -1) {
        if (vehicles[vIndex].ps != -1) parkingSpots[vehicles[vIndex].ps].vehicle = -1;
        // Clear out vehicle leaving
        vehicles[vIndex].activeJob = -1;
        vehicles[vIndex].departureTime = getDepartureTime(vehicles[vIndex].arrivalTime);
        // vehicles[vIndex].arrivalTime = -1;
        vehicles[vIndex].status = AVAILABLE;
        vehicles[vIndex].ps = -1;

        return true;
    }

    vector<int> options;
    vector<int> arrivalTimes;

    // Try to find an available vehicle in the same cluster to migrate to
    for (int i = 0; i < parkingSpots.size(); i++) {
        // Check for available vehicle in same cluster
        if (parkingSpots[i].name != vehicles[vIndex].ps &&
            parkingSpots[i].ap == parkingSpots[vehicles[vIndex].ps].ap &&
            parkingSpots[i].vehicle != -1 &&
            vehicles[parkingSpots[i].vehicle].status == AVAILABLE &&
            vehicles[parkingSpots[i].vehicle].arrivalTime != -1) {
                // Store all available vehicles arrival times 
                options.push_back(i);
                arrivalTimes.push_back(vehicles[parkingSpots[i].vehicle].arrivalTime);
        }
    }

    if (!options.empty() && !arrivalTimes.empty()) {
        int idealIndex = options[getClosest(arrivalTimes, curTime)];
        // Migrate job to new vehicle
        int dataSize = 0;
        if (vehicles[vIndex].activeJob != -1) {
            dataSize = jobs[vehicles[vIndex].activeJob].generatedDataSize;
        }
        // UPLOAD Parking Spot to Access Point (wireless)
        transferDelay(getTransferTime(dataSize, false));
        // DOWNLOAD Access Point to Parking Spot (wireless)
        transferDelay(getTransferTime(dataSize, false));
        vehicles[parkingSpots[idealIndex].vehicle].status = BUSY;
        vehicles[parkingSpots[idealIndex].vehicle].ps = parkingSpots[idealIndex].name;
        vehicles[parkingSpots[idealIndex].vehicle].activeJob = vehicles[vIndex].activeJob;


        // Clear out vehicle leaving
        vehicles[vIndex].activeJob = -1;
        vehicles[vIndex].departureTime = getDepartureTime(vehicles[vIndex].arrivalTime);
        // vehicles[vIndex].arrivalTime = -1;
        vehicles[vIndex].status = AVAILABLE;
        vehicles[vIndex].ps = -1;

        return true;
    }

    options.clear();
    arrivalTimes.clear();

    // Try to find an available vehicle in the same group to migrate to
    for (int i = 0; i < parkingSpots.size(); i++) {
        // Check for available vehicle in same cluster
        if (parkingSpots[i].name != vehicles[vIndex].ps &&
            parkingSpots[i].gc == parkingSpots[vehicles[vIndex].ps].gc &&
            parkingSpots[i].vehicle != -1 &&
            vehicles[parkingSpots[i].vehicle].status == AVAILABLE &&
            vehicles[parkingSpots[i].vehicle].arrivalTime != -1) {
                // Store all available vehicles arrival times 
                options.push_back(i);
                arrivalTimes.push_back(vehicles[parkingSpots[i].vehicle].arrivalTime);
        }
    }

    if (!options.empty() && !arrivalTimes.empty()) {
        int idealIndex = options[getClosest(arrivalTimes, curTime)];
        // Migrate job to new vehicle
        int dataSize = 0;
        if (vehicles[vIndex].activeJob != -1) {
            dataSize = jobs[vehicles[vIndex].activeJob].generatedDataSize;
        }
        // UPLOAD Parking Spot to Access Point (wireless)
        transferDelay(getTransferTime(dataSize, false));
        // UPLOAD Access Point to Group Controller (wired)
        transferDelay(getTransferTime(dataSize, true));
        // DOWNLOAD Group Controller to Access Point (wired)
        transferDelay(getTransferTime(dataSize, true));
        // DOWNLOAD Access Point to Parking Spot (wireless)
        transferDelay(getTransferTime(dataSize, false));
        vehicles[parkingSpots[idealIndex].vehicle].status = BUSY;
        vehicles[parkingSpots[idealIndex].vehicle].ps = parkingSpots[idealIndex].name;
        vehicles[parkingSpots[idealIndex].vehicle].activeJob = vehicles[vIndex].activeJob;


        // Clear out vehicle leaving
        vehicles[vIndex].activeJob = -1;
        vehicles[vIndex].departureTime = getDepartureTime(vehicles[vIndex].arrivalTime);
        // vehicles[vIndex].arrivalTime = -1;
        vehicles[vIndex].status = AVAILABLE;
        vehicles[vIndex].ps = -1;

        return true;
    }

    options.clear();
    arrivalTimes.clear();

    // Try to find an available vehicle in the same region to migrate to
    for (int i = 0; i < parkingSpots.size(); i++) {
        // Check for available vehicle in same cluster
        if (parkingSpots[i].name != vehicles[vIndex].ps &&
            parkingSpots[i].rc == parkingSpots[vehicles[vIndex].ps].rc &&
            parkingSpots[i].vehicle != -1 &&
            vehicles[parkingSpots[i].vehicle].status == AVAILABLE &&
            vehicles[parkingSpots[i].vehicle].arrivalTime != -1) {
                // Store all available vehicles arrival times 
                options.push_back(i);
                arrivalTimes.push_back(vehicles[parkingSpots[i].vehicle].arrivalTime);
        }
    }

    if (!options.empty() && !arrivalTimes.empty()) {
        int idealIndex = options[getClosest(arrivalTimes, curTime)];
        // Migrate job to new vehicle
        int dataSize = 0;
        if (vehicles[vIndex].activeJob != -1) {
            dataSize = jobs[vehicles[vIndex].activeJob].generatedDataSize;
        }
        // UPLOAD Parking Spot to Access Point (wireless)
        transferDelay(getTransferTime(dataSize, false));
        // UPLOAD Access Point to Group Controller (wired)
        transferDelay(getTransferTime(dataSize, true));
        // UPLOAD Group Controller to Region Controller (wired)
        transferDelay(getTransferTime(dataSize, true));

        // DOWNLOAD Region Controller to Group Controller (wired)
        transferDelay(getTransferTime(dataSize, true));
        // DOWNLOAD Group Controller to Access Point (wired)
        transferDelay(getTransferTime(dataSize, true));
        // DOWNLOAD Access Point to Parking Spot (wireless)
        transferDelay(getTransferTime(dataSize, false));
        vehicles[parkingSpots[idealIndex].vehicle].status = BUSY;
        vehicles[parkingSpots[idealIndex].vehicle].ps = parkingSpots[idealIndex].name;
        vehicles[parkingSpots[idealIndex].vehicle].activeJob = vehicles[vIndex].activeJob;


        // Clear out vehicle leaving
        vehicles[vIndex].activeJob = -1;
        vehicles[vIndex].departureTime = getDepartureTime(vehicles[vIndex].arrivalTime);
        // vehicles[vIndex].arrivalTime = -1;
        vehicles[vIndex].status = AVAILABLE;
        vehicles[vIndex].ps = -1;

        return true;
    }

    options.clear();
    arrivalTimes.clear();

    // Try to find an available vehicle in the datacenter to migrate to
    for (int i = 0; i < parkingSpots.size(); i++) {
        // Check for available vehicle in same cluster
        if (parkingSpots[i].name != vehicles[vIndex].ps &&
            parkingSpots[i].dc == parkingSpots[vehicles[vIndex].ps].dc &&
            parkingSpots[i].vehicle != -1 &&
            vehicles[parkingSpots[i].vehicle].status == AVAILABLE &&
            vehicles[parkingSpots[i].vehicle].arrivalTime != -1) {
                // Store all available vehicles arrival times 
                options.push_back(i);
                arrivalTimes.push_back(vehicles[parkingSpots[i].vehicle].arrivalTime);
        }
    }

    if (!options.empty() && !arrivalTimes.empty()) {
        int idealIndex = options[getClosest(arrivalTimes, curTime)];
        // Migrate job to new vehicle
        int dataSize = 0;
        if (vehicles[vIndex].activeJob != -1) {
            dataSize = jobs[vehicles[vIndex].activeJob].generatedDataSize;
        }
        // UPLOAD Parking Spot to Access Point (wireless)
        transferDelay(getTransferTime(dataSize, false));
        // UPLOAD Access Point to Group Controller (wired)
        transferDelay(getTransferTime(dataSize, true));
        // UPLOAD Group Controller to Region Controller (wired)
        transferDelay(getTransferTime(dataSize, true));

        // DOWNLOAD Region Controller to Group Controller (wired)
        transferDelay(getTransferTime(dataSize, true));
        // DOWNLOAD Group Controller to Access Point (wired)
        transferDelay(getTransferTime(dataSize, true));
        // DOWNLOAD Access Point to Parking Spot (wireless)
        transferDelay(getTransferTime(dataSize, false));
        vehicles[parkingSpots[idealIndex].vehicle].status = BUSY;
        vehicles[parkingSpots[idealIndex].vehicle].ps = parkingSpots[idealIndex].name;
        vehicles[parkingSpots[idealIndex].vehicle].activeJob = vehicles[vIndex].activeJob;


        // Clear out vehicle leaving
        vehicles[vIndex].activeJob = -1;
        vehicles[vIndex].departureTime = getDepartureTime(vehicles[vIndex].arrivalTime);
        // vehicles[vIndex].arrivalTime = -1;
        vehicles[vIndex].status = AVAILABLE;
        vehicles[vIndex].ps = -1;

        return true;
    }

    // Clear out parking spot
    parkingSpots[vehicles[vIndex].ps].vehicle = -1;

    // Migration failed so clear out vehicle leaving, reset job, and open up job
    jobs[vehicles[vIndex].activeJob].startTime = -1;
    jobs[vehicles[vIndex].activeJob].open = true;
    vehicles[vIndex].activeJob = -1;
    vehicles[vIndex].departureTime = getDepartureTime(vehicles[vIndex].arrivalTime);
    // vehicles[vIndex].arrivalTime = -1;
    vehicles[vIndex].status = AVAILABLE;
    vehicles[vIndex].ps = -1;

    return false;
}


// helper function to determine the current time in hours (simulated: seconds) 
int getCurTimeInHours(int time) {
    // Offset by 7 because we are starting at 7
    int curTime = (time / 1000000) + 7;
    return curTime;
}

// helper function to determine if it is time to migrate
//      threshold should be changed to bring them closer to the end time
int isTimeToMigrate(int duration, int threshold) {
    return (duration % ONE_HOUR >= threshold);
} 

// Determines which vehicles are departing
vector<int> getVehiclesDeparting(vector<vehicle> &vehicles, int curTime) {
    vector<int> departing;
    for (int i = 0; i < vehicles.size(); i++) {
        if ((int) ceil((vehicles[i].arrivalTime + 8) - curTime) <= 0) {
            departing.push_back(vehicles[i].name);
        }
        if (departing.size() == 320) {
            break;
        }
    }
    // cout << "Number of vehicles departing: " << departing.size() << endl;
    return departing;
}

// Determines which vehicles are arriving
vector<int> getVehiclesArriving(vector<vehicle> &vehicles, int curTime) {
    vector<int> arriving;
    for (int i = 0; i < vehicles.size(); i++) {
        if (vehicles[i].ps == -1 && ((int) ceil((vehicles[i].departureTime + 8) - curTime) <= 0 || vehicles[i].departureTime == -1)) {
            arriving.push_back(vehicles[i].name);
        }
        if (arriving.size() == 320) {
            break;
        }
    }
    return arriving;
}

// Checks for completed jobs
vector<int> getVehiclesToTransfer(vector<job> jobs, vector<vehicle> &vehicles, int curTime) {
    vector<int> transfer;
    for (int i = 0; i < vehicles.size(); i++) {
        if (vehicles[i].activeJob != -1) {
            int amountCompleted = jobs[vehicles[i].activeJob].startTime + curTime;
            if (jobs[vehicles[i].activeJob].duration <= amountCompleted) {
                transfer.push_back(i);
            }
        }
    }

    return transfer;
}

// Gathers all available parking spots
vector<int> getAvailableParkingSpots(vector<parkingSpot> &parkingSpots) {
    vector<int> availableSpots;
    for (int i = 0; i < parkingSpots.size(); i++) {
        if (parkingSpots[i].vehicle == -1) {
            availableSpots.push_back(parkingSpots[i].name);
        }
    }
    // cout << "line 905: number of available spots: " << availableSpots.size() << endl;
    return availableSpots;
}
// Randomly assign vehicle to parking spot
void assignVehicleParkingSpot(vector<vehicle> &vehicles, int vIndex, vector<parkingSpot> &parkingSpots, vector<int> availableSpots, int curTime) {
    // Shuffle available parking spots
    // unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    // std::shuffle(availableSpots.begin(), availableSpots.end(), std::default_random_engine(seed));

    for (int i = 0; i < availableSpots.size(); i++) {
        // Add vehicle to random parking spot and set its arrival time to curTime
        parkingSpots[availableSpots[i]].vehicle = vehicles[vIndex].name;
        vehicles[vIndex].ps = availableSpots[i];
        vehicles[vIndex].arrivalTime = curTime;
    }    
}






int main() 
{
    srand(time(0));
    // Set up everything for the simulation (parking lot is empty)
    datacenterController dc = createDC();
    vector<regionController> rcs = createRCs(dc);
    vector<groupController> gcs = createGCs(rcs);
    vector<accessPoint> aps = createAPs(gcs);
    vector<parkingSpot> parkingSpots = createPSs(aps);
    vector<vehicle> vehicles = createVehicles();
    vector<job> jobs = createJobs();

    vector<job> completedJobs;

    // Setup the simulation by filling the parking lot with vehicles randomly
    setupSimulation(parkingSpots, vehicles, jobs);
    // printVehicles(vehicles, jobs);

    int successfulMigrations = 0;
    int failedMigrations = 0;
    string migrationStrategy = "1";
    cout << "Please Select a migration strategy (type 0 OR 1 OR 2 OR 3):" << endl;
    cin >> migrationStrategy;

    auto start = high_resolution_clock::now();
    bool migrateTime = false;
    while (completedJobs.size() < 1500) {
        cout << "Completed Jobs: " << completedJobs.size() << endl;
        auto interim = high_resolution_clock::now(); 
        auto span = duration_cast<microseconds>(interim - start);
        int curTime = getCurTimeInHours(span.count());
        cout << "Current Time(1): " << curTime << endl;
        // Get and transfer completed jobs to the datacenter manager
        vector<int> transfers = getVehiclesToTransfer(jobs, vehicles, curTime);
        if (!transfers.empty()) {
            for (int i = 0; i < transfers.size(); i++) {
                interim = high_resolution_clock::now(); 
                span = duration_cast<microseconds>(interim - start);
                curTime = getCurTimeInHours(span.count());
                // start threshold at 30 minutes (simulation: 0.5 seconds) before departure 
                migrateTime = isTimeToMigrate(span.count(), MIGRATION_THRESHOLD);
                if (migrateTime) break;
                transferCompletedJobToDM(parkingSpots, vehicles, transfers[i], jobs, completedJobs);
                assignJobToVehicle(jobs, vehicles, transfers[i], curTime);
            }
        }
        interim = high_resolution_clock::now(); 
        span = duration_cast<microseconds>(interim - start);
        curTime = getCurTimeInHours(span.count());
        // cout << "Current Time(3): " << curTime << endl;
        // start threshold at 30 minutes (simulation: 0.9 seconds) before departure 
        migrateTime = isTimeToMigrate(span.count(), MIGRATION_THRESHOLD);

        if (migrateTime) {
            // Migrate VMs of departing vehicles
            vector<int> departing = getVehiclesDeparting(vehicles, curTime);
            if (!departing.empty()) {
                auto interim1 = high_resolution_clock::now(); 
                auto interim2 = high_resolution_clock::now();
                auto elapsedMigration = duration_cast<microseconds>(interim1 - interim2);
                int departedCount = 0;
                vector<int> temp;
                for (int i = 0; i < departing.size(); i++) {
                    bool migration = false;
                    if (migrationStrategy == "0") {
                        migration = migrateVM_S0(parkingSpots, vehicles, departing[i], jobs);
                    }
                    if (migrationStrategy == "1") {
                        migration = migrateVM_S1(parkingSpots, vehicles, departing[i], jobs);
                    }
                    else if (migrationStrategy == "2") {
                        migration = migrateVM_S2(parkingSpots, vehicles, departing[i], jobs, curTime);
                    }
                    else if (migrationStrategy == "3") {
                        auto it = find(transfers.begin(), transfers.end(), departing[i]);
                        if (it != transfers.end()) {
                            transferCompletedJobToDM(parkingSpots, vehicles, departing[i], jobs, completedJobs);
                        }
                        migration = migrateVM_S3(parkingSpots, vehicles, departing[i], jobs, curTime, completedJobs, transfers);
                    }
                    
                    // printVehicles(vehicles, jobs);
                    // cout << "Available Jobs: " << jobs.size() << endl;
                    temp.push_back(vehicles[departing[i]].ps);
                    // bool migration = migrateVM_S3(parkingSpots, vehicles, departing[i], jobs, curTime, completedJobs, transfers);
                    // bool migration = migrateVM_S2(parkingSpots, vehicles, departing[i], jobs, curTime);
                    // bool migration = migrateVM_S1(parkingSpots, vehicles, departing[i], jobs);
                    parkingSpots[vehicles[departing[i]].ps].vehicle = -1;
                    if (migration) successfulMigrations++;
                    if (!migration) failedMigrations++;
                    departedCount++;
                    interim2 = high_resolution_clock::now();
                    elapsedMigration = duration_cast<microseconds>(interim1 - interim2);
                    if (elapsedMigration.count() < (MIGRATION_THRESHOLD - 500)) break;
                }
                // If not all of the departing vehicles had a chance to migrate they lose that
                if (departedCount < departing.size() - 1) {
                    for (int i = departedCount; i < departing.size(); i++) {
                        temp.push_back(vehicles[departing[i]].ps);
                        // Clear out parking spot
                        parkingSpots[vehicles[departing[i]].ps].vehicle = -1;

                        // Migration failed so clear out vehicle leaving, reset job, and open up job
                        jobs[vehicles[departing[i]].activeJob].startTime = -1;
                        jobs[vehicles[departing[i]].activeJob].open = true;
                        vehicles[departing[i]].activeJob = -1;
                        vehicles[departing[i]].departureTime = getDepartureTime(vehicles[departing[i]].arrivalTime);
                        // vehicles[departing[i]].arrivalTime = -1;
                        vehicles[departing[i]].status = AVAILABLE;
                        vehicles[departing[i]].ps = -1;

                        failedMigrations++;
                    }
                }

                for (int i = 0; i < temp.size(); i++) {
                    parkingSpots[i].vehicle = -1;
                }
            }
            // if (!departing.empty()) {
            //     for (int i = 0; i < departing.size(); i++) {
            //         // bool migration = migrateVM_S2(parkingSpots, vehicles, departing[i], jobs, curTime);
            //         bool migration = migrateVM_S1(parkingSpots, vehicles, departing[i], jobs);
            //         parkingSpots[vehicles[departing[i]].ps].vehicle = -1;
            //         if (migration) successfulMigrations++;
            //         if (!migration) failedMigrations++;
            //     }
            // }
            // Get arriving vehicles and decide whether to give them a job or not
            vector<int> arriving = getVehiclesArriving(vehicles, curTime);
            // cout << "Number of vehicles arriving: " << arriving.size() << endl;
            vector<int> availableSpots = getAvailableParkingSpots(parkingSpots);
            // cout << "Available Spots: " << availableSpots.size() << endl;
            for (int i = 0; i < arriving.size(); i++) {
                assignVehicleParkingSpot(vehicles, i, parkingSpots, availableSpots, curTime);
                int percentAvailableVehicles = getPercentOfAvailableVehicles(vehicles);
                // cout << "Percentage of Available Vehicles: " << percentAvailableVehicles << endl;
                if (percentAvailableVehicles > 25) {
                    assignJobToVehicle(jobs, vehicles, i, curTime);
                }
            }
        }
        
    }
    auto stop = high_resolution_clock::now(); 
    auto duration = duration_cast<microseconds>(stop - start); 
    // cout << failedMigrations << " failed migrations of " << failedMigrations + successfulMigrations << " total migrations" << endl;
    cout << "Total Time complete " << completedJobs.size() << " jobs: " << duration.count() << endl; 
    return 0;
}