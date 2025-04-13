#include <windows.h>
#include <iostream>
#include <vector>
#include <cassert>

using namespace std;

vector<int> arr;
int minVal, maxVal;
float averageVal;

void findMinMax(const vector<int>& data, int& minOut, int& maxOut) {
    minOut = data[0];
    maxOut = data[0];
    for (size_t i = 1; i < data.size(); ++i) {
        if (data[i] < minOut) minOut = data[i];
        if (data[i] > maxOut) maxOut = data[i];
        Sleep(7);
    }
}

float findAverage(const vector<int>& data) {
    int sum = 0;
    for (int val : data) {
        sum += val;
        Sleep(12);
    }
    return static_cast<float>(sum) / data.size();
}





DWORD WINAPI MinMaxThread(LPVOID) {
    findMinMax(arr, minVal, maxVal);
    cout << "Min: " << minVal << endl;
    cout << "Max: " << maxVal << endl;
    return 0;
}

DWORD WINAPI AverageThread(LPVOID) {
    averageVal = findAverage(arr);
    cout << "Average: " << averageVal << endl;
    return 0;
}






void runTests() {
    vector<int> testArr = {1, 5, 3, 9, 2};
    int testMin, testMax;
    findMinMax(testArr, testMin, testMax);
    assert(testMin == 1);
    assert(testMax == 9);

    float avg = findAverage(testArr);
    assert(abs(avg - 4.0f) < 0.001);
    cout << "All tests passed." << endl;
}

int main() {
    runTests(); 

    int n;
    cout << "Enter array size: ";
    cin >> n;

    arr.resize(n);
    cout << "Enter " << n << " integers: ";
    for (int& val : arr) {
        cin >> val;
    }

    HANDLE hMinMax = CreateThread(nullptr, 0, MinMaxThread, nullptr, 0, nullptr);
    HANDLE hAverage = CreateThread(nullptr, 0, AverageThread, nullptr, 0, nullptr);

    WaitForSingleObject(hMinMax, INFINITE);
    WaitForSingleObject(hAverage, INFINITE);

    for (int& val : arr) {
        if (val == minVal || val == maxVal) {
            val = static_cast<int>(averageVal);
        }
    }

    cout << "Modified array: ";
    for (int val : arr) {
        cout << val << " ";
    }
    cout << endl;

    CloseHandle(hMinMax);
    CloseHandle(hAverage);

    return 0;
}
