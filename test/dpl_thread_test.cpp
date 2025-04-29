

#include <stdio.h>
#include <chrono>
#include "src/dpl_thread.h"

using namespace dpl;

class ThreadTest: public DplThread
{
public:
	ThreadTest() {}
	virtual ~ThreadTest() {}

protected:
    virtual int OnStart() 
    { 
    	printf("thread OnStart\n");
    	return 0; 
    };

    virtual void OnStop()
    {
    	printf("thread OnStop\n");
    };

    virtual int BeforeRun() 
    { 
    	printf("thread BeforeRun\n");
    	return 0; 
    };

    virtual int Run()
    {
    	for(int count = 0; count < 10 && running_; count++)
    	{
    		printf("thread run count=%d\n", count++);

    		std::this_thread::sleep_for(std::chrono::milliseconds(100));
    	}

    	return 0;
    }

    virtual void AfterRun()
    { 
    	printf("thread AfterRun\n");
    };

};

int main(int argc, char* argv[])
{
    ThreadTest test;


    test.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(400));

    test.Stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(400));

}
