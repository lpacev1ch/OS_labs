#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>
#include <chrono>
#include <atomic>
#include <map>
#include <set>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

using namespace std;

mutex mtx;
condition_variable cv_start, cv_resume;
atomic<bool> start_signal{false};
atomic<int> ready_markers{0};

vector<int> shared_array;
vector<thread> marker_threads;
vector<condition_variable*> marker_cvs;
vector<mutex*> marker_mutexes;
vector<bool> marker_continue;
vector<bool> marker_should_finish;
vector<atomic<bool>> marker_waiting;
vector<set<int>> marker_marked_indices;

int array_size = 0;
int marker_count = 0;

void marker_func(int id) {
    srand(id);
    unique_lock<mutex> lk(*marker_mutexes[id]);
    cv_start.wait(lk, [] { return start_signal.load(); });
    lk.unlock();

    while (true) {
        int index = rand() % array_size;

        mtx.lock();
        if (shared_array[index] == 0) {
            this_thread::sleep_for(chrono::milliseconds(5));
            shared_array[index] = id + 1;
            marker_marked_indices[id].insert(index);
            this_thread::sleep_for(chrono::milliseconds(5));
            mtx.unlock();
            continue;
        } else {
            cout << "Marker " << id + 1 << " cannot mark index " << index
                 << ". Total marked: " << marker_marked_indices[id].size() << "\n";
            mtx.unlock();
            marker_waiting[id] = true;
            unique_lock<mutex> l(*marker_mutexes[id]);
            cv_resume.wait(l, [id] {
                return marker_continue[id] || marker_should_finish[id];
            });
            if (marker_should_finish[id]) {
                lock_guard<mutex> lock(mtx);
                for (int i : marker_marked_indices[id]) {
                    shared_array[i] = 0;
                }
                return;
            }
            marker_continue[id] = false;
            marker_waiting[id] = false;
        }
    }
}

void print_array() {
    lock_guard<mutex> lock(mtx);
    for (int val : shared_array) {
        cout << val << " ";
    }
    cout << "\n";
}












TEST_CASE("Thread marker logic works as expected") {
    array_size = 10;
    marker_count = 3;

    shared_array.assign(array_size, 0);
    marker_cvs.resize(marker_count);
    marker_mutexes.resize(marker_count);
    marker_continue.assign(marker_count, false);
    marker_should_finish.assign(marker_count, false);
    marker_waiting.resize(marker_count);
    marker_marked_indices.resize(marker_count);

    for (int i = 0; i < marker_count; ++i) {
        marker_cvs[i] = new condition_variable();
        marker_mutexes[i] = new mutex();
        marker_waiting[i] = false;
        marker_threads.emplace_back(marker_func, i);
    }

    start_signal = true;
    cv_start.notify_all();

    this_thread::sleep_for(chrono::seconds(2));

    for (int i = 0; i < marker_count; ++i) {
        while (!marker_waiting[i]) {
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }

    print_array();

    int finish_id = 0;
    marker_should_finish[finish_id] = true;
    marker_cvs[finish_id]->notify_one();
    marker_threads[finish_id].join();

    print_array();

    for (int i = 1; i < marker_count; ++i) {
        marker_continue[i] = true;
        marker_cvs[i]->notify_one();
    }

    this_thread::sleep_for(chrono::seconds(2));

    for (int i = 1; i < marker_count; ++i) {
        while (!marker_waiting[i]) {
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }

    for (int i = 1; i < marker_count; ++i) {
        marker_should_finish[i] = true;
        marker_cvs[i]->notify_one();
        marker_threads[i].join();
    }

    print_array();

    for (int i = 0; i < marker_count; ++i) {
        delete marker_cvs[i];
        delete marker_mutexes[i];
    }
}
