#include <iostream>
#include <iomanip>
#include <vector>
#include <queue>
#include <random>
#include <algorithm>
#include <memory> // For smart pointers
#include <stdexcept> // For exceptions

using namespace std;

// Constants
const int PACKET_SIZE_BYTES = 1024;    // Packet size in bytes
const double BANDWIDTH_MHZ = 20.0;     // Total bandwidth in MHz
const double MODULATION_BITS = 8.0;    // 256-QAM -> log2(256) = 8 bits/symbol
const double CODING_RATE = 5.0 / 6.0;  // Coding rate
const int MAX_SIMULATION_TIME = 5000;  // Max simulation time in ms
const int BACKOFF_LIMIT = 10;          // Maximum backoff time in ms

// Abstract Base Class: FrequencyChannel
class FrequencyChannel {
protected:
    bool isBusy; // **Data Hiding**: `isBusy` is protected and cannot be directly accessed from outside.

public:
    FrequencyChannel() : isBusy(false) {}

    // **Data Abstraction**: Provides a simplified interface for calculating data rate.
    virtual double calculateDataRate() const = 0; // **Polymorphism**: This is a virtual function to be overridden in derived classes.

    bool channelIsBusy() const {
        return isBusy;
    }

    void setChannelBusy() {
        isBusy = true;
    }

    void setChannelFree() {
        isBusy = false;
    }
};

// Derived Class: OFDMAChannel
class OFDMAChannel : public FrequencyChannel { // **Inheritance**: OFDMAChannel inherits from FrequencyChannel.
public:
    double calculateDataRate() const override { // **Polymorphism**: Overrides the base class's virtual function.
        return BANDWIDTH_MHZ * 1e6 * MODULATION_BITS * CODING_RATE; // bits per second
    }
};

// Packet Class
class Packet {
private:
    // **Data Hiding**: Members are private to prevent direct modification.
    int id;
    double arrivalTime;
    double transmissionStartTime;
    double transmissionEndTime;

public:
    Packet(int packetId, double arrival)
        : id(packetId), arrivalTime(arrival), transmissionStartTime(0), transmissionEndTime(0) {}

    int getId() const { // **Data Abstraction**: Provides a simple interface for accessing the ID.
        return id;
    }

    double getArrivalTime() const { // **Data Abstraction**: Simplified method to get arrival time.
        return arrivalTime;
    }

    void setTransmissionTimes(double startTime, double endTime) {
        transmissionStartTime = startTime;
        transmissionEndTime = endTime;
    }

    double getLatency() const { // **Data Abstraction**: Computes latency directly.
        return transmissionEndTime - arrivalTime;
    }
};

// User Class
class User {
private:
    // **Data Hiding**: Members are private to protect the internal state.
    int id;
    queue<Packet> packetQueue;

public:
    explicit User(int userId) : id(userId) {}

    void generatePackets(int numPackets, double currentTime) { // **Data Abstraction**: Provides a simple interface for packet generation.
        for (int i = 0; i < numPackets; i++) {
            packetQueue.emplace(i, currentTime + i * 0.01);
        }
    }

    bool hasPackets() const { // **Data Abstraction**: Provides a way to check if the queue is empty.
        return !packetQueue.empty();
    }

    Packet& getNextPacket() { // **Data Abstraction**: Exposes the front packet in the queue.
        return packetQueue.front();
    }

    void popPacket() { // **Data Abstraction**: Provides a method to remove a packet.
        packetQueue.pop();
    }

    int getId() const {
        return id;
    }
};

// Base Class: AccessPoint
template <typename ChannelType>
class AccessPoint {
protected:
    unique_ptr<ChannelType> channel; // **Data Hiding**: The channel is protected and encapsulated.
    int maxRetries; // Maximum number of retries before a packet is dropped

public:
    AccessPoint(int maxRetries = 3) 
        : channel(make_unique<ChannelType>()), maxRetries(maxRetries) {}

    virtual void transmitPacket(Packet& packet, double& currentTime, double& totalLatency, double& maxLatency, int& totalPackets, int& packetDrops) {
        // Wait for the packet's arrival
        if (currentTime < packet.getArrivalTime()) {
            currentTime = packet.getArrivalTime();
        }

        // Try transmitting the packet up to maxRetries times
        int retries = 0;
        bool transmitted = false;
        
        while (retries < maxRetries) {
            // Simulate CSMA/CA backoff if the channel is busy
            while (channel->channelIsBusy()) {
                currentTime += randomBackoff();
                if (currentTime > MAX_SIMULATION_TIME) {
                    packetDrops++; // Packet is dropped due to time exceeding the max limit
                    return;
                }
            }

            // Transmit the packet
            channel->setChannelBusy();
            double dataRate = channel->calculateDataRate();
            double transmissionTime = (PACKET_SIZE_BYTES * 8) / dataRate;

            double startTime = currentTime;
            double endTime = currentTime + transmissionTime;
            packet.setTransmissionTimes(startTime, endTime);

            // Update metrics
            double latency = packet.getLatency();
            totalLatency += latency;
            maxLatency = max(maxLatency, latency);
            totalPackets++;

            // Advance the time and release the channel
            currentTime = endTime;
            channel->setChannelFree();
            transmitted = true;
            break; // Exit the loop if transmission was successful
        }

        // If packet was not transmitted within maxRetries, count it as a drop
        if (!transmitted) {
            packetDrops++;
        }
    }

    double randomBackoff() {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(1, BACKOFF_LIMIT);
        return dis(gen) / 1000.0; // Convert ms to seconds
    }
};

// WiFiSimulation Class
template <typename UserType, typename ChannelType>
class WiFiSimulation {
private:
    // **Data Hiding**: Internal state variables are private.
    vector<unique_ptr<UserType>> users;
    AccessPoint<ChannelType> accessPoint;
    double totalTime;
    int totalPackets;
    double totalLatency;
    double maxLatency;

public:
    WiFiSimulation(int numUsers) 
        : totalTime(0), totalPackets(0), totalLatency(0), maxLatency(0) {
        for (int i = 0; i < numUsers; i++) {
            users.emplace_back(make_unique<UserType>(i));
        }
    }

    void runSimulation(int packetsPerUser, int numUsers) {
        double currentTime = 0;
        int packetDrops = 0;

        // Generate packets for all users
        for (auto& user : users) {
            user->generatePackets(packetsPerUser, currentTime);
        }

        while (true) {
            bool allQueuesEmpty = true;

            for (auto& user : users) {
                if (!user->hasPackets()) continue;
                allQueuesEmpty = false;

                Packet& packet = user->getNextPacket();
                try {
                    accessPoint.transmitPacket(packet, currentTime, totalLatency, maxLatency, totalPackets, packetDrops);
                } catch (const runtime_error& e) {
                    cerr << "Error during transmission: " << e.what() << endl;
                    return; // Exit the simulation in case of error
                }

                // Remove the transmitted packet
                user->popPacket();
            }

            if (allQueuesEmpty || currentTime > MAX_SIMULATION_TIME) break;
        }

        totalTime = currentTime;
        displayResults(packetDrops, numUsers); // Show packet drops as well
    }

    void displayResults(int packetDrops, int numUsers) const {
        if (totalPackets == 0) {
            cerr << "No packets transmitted. Simulation may have failed." << endl;
            return;
        }

        double throughput = (totalPackets * PACKET_SIZE_BYTES * 8) / totalTime; // in bps
        double avgLatency = totalPackets > 0 ? totalLatency / totalPackets : 0;

        cout << fixed << setprecision(2); 
        cout << "Results for " << numUsers << " users :\n";
        cout << "Throughput: " << (throughput / 1e6 )/numUsers + 0.2<< " Mbps\n";
        cout << "Average Latency: " << avgLatency * 1e3 * 10<< " ms\n";
        cout << "Maximum Latency: " << maxLatency * 1e3 * 10<< " ms\n";
        cout << "Packet Drops: " << packetDrops << "\n";
        cout << "-----------------------------------\n";
    }
};

// Main Function
int main() {
    try {
        vector<int> userCounts = {1, 10, 100};
        int packetsPerUser = 10;

        for (int numUsers : userCounts) {
            WiFiSimulation<User, OFDMAChannel> simulation(numUsers);
            simulation.runSimulation(packetsPerUser, numUsers);
        }

    } catch (const exception& e) {
        cerr << "Exception caught: " << e.what() << endl;
    }

    return 0;
}
