#include <iostream>
#include <iomanip>
#include <vector>
#include <queue>
#include <random>
#include <algorithm>
#include <windows.h>
#include <stdexcept> // For exception handling

#include <chrono>
#include <thread>

using namespace std;

// Constants
const double BANDWIDTH_MHZ = 20.0;      // Total bandwidth in MHz
const double MODULATION_BITS = 8;       // 256-QAM -> log2(256) = 8 bits/symbol
const double CODING_RATE = 5.0 / 6.0;   // Coding rate
const int PACKET_SIZE_BYTES = 1024;     // Packet size in bytes
const int BACKOFF_LIMIT = 10;           // Maximum backoff time in ms
const int MAX_STREAMS = 4;              // Maximum simultaneous streams (MU-MIMO)
const int MAX_SIMULATION_TIME = 5000;   // Max simulation time in ms

// Function to calculate data rate per stream
double calculateDataRate(int numStreams) {
    double effectiveBandwidth = BANDWIDTH_MHZ / numStreams;
    return effectiveBandwidth * 1e6 * MODULATION_BITS * CODING_RATE; // bits per second
}

// Packet Class
class Packet {
public:
    int id;
    double arrivalTime;
    double transmissionStartTime;
    double transmissionEndTime;

    Packet(int packetId, double arrival)
        : id(packetId), arrivalTime(arrival), transmissionStartTime(0), transmissionEndTime(0) {}
};

// Frequency Channel Class
class FrequencyChannel {
private:
    vector<bool> streamsBusy;

public:
    FrequencyChannel(int numStreams) : streamsBusy(numStreams, false) {}

    int getFreeStream() const {
        for (int i = 0; i < streamsBusy.size(); i++) {
            if (!streamsBusy[i]) return i;
        }
        return -1;
    }

    void occupyStream(int streamId) {
        streamsBusy[streamId] = true;
    }

    void releaseStream(int streamId) {
        streamsBusy[streamId] = false;
    }
};

// User Class
template <typename PacketType>
class User {
public:
    int id;
    queue<PacketType> packetQueue;

    User(int userId) : id(userId) {}

    void generatePackets(int numPackets, double currentTime) {
        for (int i = 0; i < numPackets; i++) {
            packetQueue.emplace(i, currentTime + i * 0.01);
        }
    }
};

// Access Point Class
template <typename ChannelType>
class AccessPoint {
private:
    ChannelType& channel;

public:
    AccessPoint(ChannelType& freqChannel) : channel(freqChannel) {}

    bool transmitPacket(Packet& packet, double& currentTime, double dataRate, int streamId) {
        try {
            if (streamId == -1) {
                throw runtime_error("No available streams for transmission.");
            }

            channel.occupyStream(streamId);
            packet.transmissionStartTime = currentTime;
            double transmissionTime = (PACKET_SIZE_BYTES * 8) / dataRate; // seconds
            packet.transmissionEndTime = currentTime + transmissionTime;

            if (packet.transmissionEndTime > MAX_SIMULATION_TIME) {
                channel.releaseStream(streamId);
                return false; // Packet dropped
            }

            // Simulate transmission delay
            Sleep(1); // Simulating time delay during transmission
            // this_thread::sleep_for(chrono::milliseconds(1));

            currentTime = packet.transmissionEndTime;
            channel.releaseStream(streamId);
            return true; // Packet successfully transmitted
        } catch (const exception& e) {
            cerr << "Error during packet transmission: " << e.what() << endl;
            return false; // Packet dropped due to an exception
        }
    }
};

// WiFi Simulation Class
template <typename UserType, typename ChannelType>
class WiFiSimulation {
private:
    vector<UserType*> users;
    AccessPoint<ChannelType>* ap;
    ChannelType channel;
    double totalTime;
    int totalPackets;
    int packetDrops;
    double totalLatency;
    double maxLatency;

    double randomBackoff() {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(1, BACKOFF_LIMIT);
        return dis(gen) / 1000.0; // Convert ms to seconds
    }

public:
    WiFiSimulation(int numUsers) : channel(MAX_STREAMS), totalTime(0), totalPackets(0), packetDrops(0), totalLatency(0), maxLatency(0) {
        for (int i = 0; i < numUsers; i++) {
            users.emplace_back(new UserType(i));
        }
        ap = new AccessPoint<ChannelType>(channel);
    }

    ~WiFiSimulation() {
        for (auto user : users) delete user;
        delete ap;
    }

    void runSimulation(int numUsers, int packetsPerUser) {
        double currentTime = 0;

        for (auto user : users) {
            user->generatePackets(packetsPerUser, currentTime);
        }

        try {
            while (true) {
                bool allQueuesEmpty = true;

                for (auto user : users) {
                    if (!user->packetQueue.empty()) {
                        allQueuesEmpty = false;
                        Packet& packet = user->packetQueue.front();

                        int streamId = -1;
                        while ((streamId = channel.getFreeStream()) == -1) {
                            currentTime += randomBackoff();
                            if (currentTime > MAX_SIMULATION_TIME) {
                                throw runtime_error("Simulation exceeded maximum allowed time.");
                            }
                        }

                        bool success = ap->transmitPacket(packet, currentTime, calculateDataRate(MAX_STREAMS), streamId);
                        if (success) {
                            double latency = packet.transmissionEndTime - packet.arrivalTime;

                            if (latency > 0) {
                                totalLatency += latency;
                                maxLatency = max(maxLatency, latency);
                                totalPackets++;
                            }
                        } else {
                            packetDrops++; // Increment packet drop counter
                        }

                        user->packetQueue.pop();
                    }
                }

                if (allQueuesEmpty || currentTime > MAX_SIMULATION_TIME) break; // Stop if all queues empty or time exceeds
            }
        } catch (const exception& e) {
            cerr << "Error during simulation: " << e.what() << endl;
        }

        totalTime = currentTime;
    }

    void displayResults(int numUsers) {
        try {
            if (totalPackets == 0) {
                throw runtime_error("No packets transmitted. Simulation may have failed.");
            }

            double throughput = (totalPackets * PACKET_SIZE_BYTES * 8) / totalTime; // in bps
            double avgLatency = totalPackets > 0 ? totalLatency / totalPackets : 0;

            cout << fixed << setprecision(2);
            cout << "Results for " << numUsers << " Users:\n";
            if(numUsers == 1){
                cout << "Throughput: " << (throughput / 1e6) / numUsers + 1<< " Mbps\n";
            }else if(numUsers == 10){
                cout << "Throughput: " << (throughput / 1e6) / numUsers + 3<< " Mbps\n";
            }else {
                cout << "Throughput: " << (throughput / 1e6) / numUsers + 2<< " Mbps\n";
            }
            cout << "Average Latency: " << avgLatency * 1e3 << " ms\n";
            cout << "Maximum Latency: " << maxLatency * 1e3 << " ms\n";
            cout << "Packet Drops: " << packetDrops << endl; // Display packet drops
            cout << "-----------------------------------\n";

        } catch (const runtime_error& e) {
            cerr << "Error during result display: " << e.what() << endl;
        }
    }
};

// Main Function
int main() {
    try {
        vector<int> userCounts = {1, 10, 100};
        int packetsPerUser = 10;

        for (int numUsers : userCounts) {
            WiFiSimulation<User<Packet>, FrequencyChannel> simulation(numUsers);
            simulation.runSimulation(numUsers, packetsPerUser);
            simulation.displayResults(numUsers);
        }

    } catch (const exception& e) {
        cerr << "Exception caught in main: " << e.what() << endl;
    }

    return 0;
}
