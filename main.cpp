#include <iostream>
#include "src/thread_pool.h"

using namespace std;

void task_one() {
    for(int i=0; i<100; i++) {
        cout << "ThreadID = "<< this_thread::get_id() << ", print " << i << "." << endl;
    }
}

int main() {
    thread_pool pool(4);
    for(int i=0; i<10; i++) {
        pool.commit(task_one);
    }
    return 0;
}
