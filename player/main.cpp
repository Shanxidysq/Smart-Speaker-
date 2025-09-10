#include <iostream>
#include <thread>
using namespace std;
void Handler1()
{
    for (int i = 0; i < 100; ++i)
    {
        cout << i << endl;
    }
}

void Handler2()
{
    for (int i = 110; i < 200; ++i)
    {
        cout << i << endl;
    }
}


int main()
{
    std::thread th1(Handler1);
    std::thread th2(Handler2);

    th1.join();
    th2.join();
    return 0;
}