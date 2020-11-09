#pragma once
#include "iostream"
#include "tbb/atomic.h"
struct __attribute__((packed)) record0{
long auction;
long bidder;
long price;
long dateTime;
};
struct __attribute__((packed)) record1{
tbb::atomic<double> price_avg;
tbb::atomic<long> price_sum;
tbb::atomic<long> count;
};
