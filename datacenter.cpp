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
};

vector<int> availableSpots;
// 500MB
const int JOB_INPUT = 500;
// 1Gbps normalized to Mb
const int WIRED_BANDWIDTH = 1000;
// 54Mbps 
const int WIRELESS_BANDWIDTH = 54;

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
void printVehicles(vector<vehicle> vehicles) {
    // for (int i = 0; i < vehicles.size(); i++) {
    for (int i = 0; i < 2560; i++) {
        cout << vehicles[i].name << endl;
        cout << "   - Parking Spot: " << vehicles[i].ps << endl;
        cout << "   - Status: " << vehicles[i].status << endl;
        cout << "   - Arrival Time: " << vehicles[i].arrivalTime << endl;
        cout << "   - Departure Time: " << vehicles[i].departureTime << endl;
    }
}

// Get generated data uniformly distributed in [0.25, 1.0] GB
int getGeneratedDataSizeInMB() {
    double lower = 0.25;
    double upper = 1;
    uniform_real_distribution<double> unif(lower, upper);
    default_random_engine re;
    double dataSize = unif(re); 
    cout << dataSize << endl; 
    int dsInMB = getGBtoMB(dataSize);
    return (int) dsInMB;
}

// Uniformaly distributed in [3, 24] hours
int getJobDuration() {
    int duration = rand() % (24 - 3 + 1);
    return duration;
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
        v.status = "available";
        v.arrivalTime = -1;
        v.departureTime = -1;
        vehicles.push_back(v);
    }

    return vehicles;
}

void setUpSimulation() {

}

// At the beginning of the simulation the parking lot is full 
// Add a vehicle to every parking spot randomly from the vector of vehicles 
// Each set of 320 vehicles should have an arrival time that is offset by an hour for each group
//      This reperesents each 320 vehicle group arriving for their 8 hour shifts at different times
void setupSimulation(vector<parkingSpot> &parkingSpots, vector<vehicle> &vehicles) {
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

    // Setup the simulation by filling the parking lot with vehicles randomly
    setupSimulation(parkingSpots, vehicles);


    auto start = high_resolution_clock::now();
    
    auto stop = high_resolution_clock::now(); 
    auto duration = duration_cast<microseconds>(stop - start); 
    cout << duration.count() << endl; 
    return 0;
}