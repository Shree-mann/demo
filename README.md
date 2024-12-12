# OOPD_Project
This project simulates the performance of three Wi-Fi protocols: Wi-Fi 4 (CSMA/CA), Wi-Fi 5 (MU-MIMO), and Wi-Fi 6 (OFDMA). The simulation calculates key metrics such as throughput, latency, and packet transmission times under varying network conditions.



Wifi4:
Overview
This program simulates a WiFi network based on IEEE 802.11 standards, evaluating latency and throughput with varying numbers of users. It focuses on packet transmission dynamics, including backoff and channel contention mechanisms.

Key Features
Channel Contention: Simulates whether the channel is free using a random probability based on the number of users.
Backoff Mechanism: Implements a random backoff mechanism when the channel is busy.
Performance Metrics: Calculates throughput, average latency, and maximum latency.

Wifi5:
Overview
This program models a WiFi 5 (802.11ac) network with features like sub-channel allocation and user queue management. The simulation includes realistic conditions like queue overflow and packet timeout.

Key Features
Sub-Channel Allocation: Divides bandwidth into sub-channels for parallel transmission.
Queue Management: Simulates user queues with maximum limits and packet dropping.
Timeout Handling: Drops packets that exceed a predefined timeout period.
Performance Metrics: Calculates throughput, average latency, maximum latency, and dropped packets.

Wifi6:
Overview
This program simulates a WiFi 6 (802.11ax) network with advanced features like MU-MIMO and dynamic channel allocation. It focuses on optimizing network performance by leveraging multi-stream capabilities.

Key Features
MU-MIMO: Simulates multiple simultaneous data streams for efficient bandwidth usage.
Dynamic Backoff: Implements random backoff to manage channel contention.
Stream Management: Dynamically assigns available streams to users.
Performance Metrics: Calculates throughput, average latency, maximum latency, and dropped packets.

I have taken a maximal reference of Chatgpt and understood the concepts of WIfi communication module with their latency and throughput calculations.