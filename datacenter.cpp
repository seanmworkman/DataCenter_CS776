#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
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

// helper function to convert GB to MB
double getGBtoMB(double gb) {
    return gb * 1000;
}

// helper function to convert MB to GB
double getMBtoGB(double mb) {
    return mb / 1000;
}

// Helper function to convert bytes into bits
int bytesToBits(int bytes) {
    return bytes * 8;
}

// Print function for vehicles 
void printVehicles(vector<vehicle> vehicles, vector<job> jobs) {
    // for (int i = 0; i < vehicles.size(); i++) {
    for (int i = 0; i < 2560; i++) {
        cout << "Vehicle: " << vehicles[i].name << endl;
        cout << "   - Parking Spot: " << vehicles[i].ps << endl;
        cout << "   - Status: " << vehicles[i].status << endl;
        cout << "   - Arrival Time: " << vehicles[i].arrivalTime << endl;
        cout << "   - Departure Time: " << vehicles[i].departureTime << endl;
        cout << "   - Active Job: " << vehicles[i].activeJob << endl;
        if (vehicles[i].activeJob != -1) {
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

// helper function for simulating transfer latencies
void transferDelay(int transferTime) {
    struct timespec timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = transferTime * 1000;
    nanosleep(&timeout, NULL);
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
    for (int i = 0; i < 23040; i++) {
        job curJob;
        curJob.name = i;
        curJob.generatedDataSize = getGeneratedDataSizeInMB();
        curJob.duration = getJobDuration();

        jobs.push_back(curJob);
    }
    return jobs;
}

// Assign a job to the specified vehicle
void assignJobToVehicle(vector<job> &jobs, vehicle &v) {
    // If there are no jobs remaining create more
    if (jobs.empty()) {
        jobs = createJobs();
    }

    for (int i = 0; i < jobs.size(); i++) {
        // Find an open job, assign it to the vehicle and set the vehicle's status to busy
        if (jobs[i].open) {
            v.activeJob = jobs[i].name;
            v.status = BUSY;
            jobs[i].open = false;
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
            assignJobToVehicle(jobs, vehicles[i]);
        }
    }
}

int getDepartureTime(int arrivalTime) {
    int dt = arrivalTime + 8;
    if (dt >= 24) {
        dt -= 24;
    }
    return dt;
}


void migrateVM(vector<parkingSpot> &parkingSpots, vector<vehicle> &vehicles, int vIndex, vector<job> &jobs) {
    // Try to find an available vehicle in the same cluster to migrate to
    // Best case the one that has an arrival time closest to the current time
    for (int i = 0; i < parkingSpots.size(); i++) {
        // TODO: Implement time closest to current time (migration strategy 2??????????????????????????????????????????????????????)
        // Check for available vehicle in same cluster
        if (parkingSpots[i].name != vehicles[vIndex].ps &&
            parkingSpots[i].ap == parkingSpots[vehicles[vIndex].ps].ap &&
            parkingSpots[i].vehicle != -1 &&
            vehicles[parkingSpots[i].vehicle].status == AVAILABLE) {
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
                vehicles[vIndex].arrivalTime = -1;
                vehicles[vIndex].status = AVAILABLE;

                break;
        }
        // TODO: Cover situation where no options inside cluster OR group OR region
    }
}



// Transfer a completed job to the vehicle's respective datacenter manager (co-located with the DC)
void transferCompletedJobToDC(vector<parkingSpot> &parkingSpots, vector<vehicle> &vehicles, int vIndex, vector<job> &jobs, vector<job> &completedJobs) {
    // Get the parking spot that the vehicle is in
    parkingSpot ps = parkingSpots[vehicles[vIndex].ps];
    int dataSize = 0;
    if (vehicles[vIndex].activeJob != -1) {
        dataSize = jobs[vehicles[vIndex].activeJob].generatedDataSize;
    }
    
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
    if (vehicles[vIndex].activeJob != -1) {
        completedJobs.push_back(jobs[vehicles[vIndex].activeJob]);
        jobs.erase(jobs.begin() + vehicles[vIndex].activeJob);
    }
    // The vehicle is now available
    vehicles[vIndex].status = AVAILABLE;
    vehicles[vIndex].activeJob = -1;
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

    auto start = high_resolution_clock::now();
    



    migrateVM(parkingSpots, vehicles, 2558, jobs);
    // transferCompletedJobToDC(parkingSpots, vehicles, 2558, jobs, completedJobs);
    printVehicles(vehicles, jobs);
    // for (int i = 0; i < completedJobs.size(); i++) {
    //     cout << completedJobs[i].name << endl;
    //     cout << completedJobs[i].generatedDataSize << endl;
    //     cout << completedJobs[i].duration << endl;
    // }
    auto stop = high_resolution_clock::now(); 
    auto duration = duration_cast<microseconds>(stop - start); 
    cout << duration.count() << endl; 
    return 0;
}