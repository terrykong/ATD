//

//  
//
//  Created by Terry Kong on 2/12/14.
//
//

#include <iostream>
#include <sys/time.h>

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::high_resolution_clock::time_point Time;
using namespace std::chrono;

int main()
{
    Time t1 = Clock::now();
    
    std::cout << "printing out 1000 stars...\n";
    for (int i=0; i<1; ++i) std::cout << "*";
    std::cout << std::endl;
    
    Time t2 = Clock::now();
    
    duration<double> time_span = duration_cast<duration<double> >(t2 - t1);
    //nanoseconds time_span = duration_cast<nanoseconds>(t2 - t1);
    
    std::cout << "It took me " << time_span.count() << " seconds.";
    std::cout << std::endl;
    
    std::cout << "It took me " << time_span.count()*1e6 << " seconds.";
    std::cout << std::endl;
    
    return 0;
}
