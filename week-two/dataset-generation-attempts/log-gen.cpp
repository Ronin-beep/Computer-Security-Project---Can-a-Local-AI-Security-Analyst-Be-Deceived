/*
    C++23

    Why did I abandon this? It would take too long to properly implement everything I wanted.
*/

#include <iostream>
#include <cstdint>
#include <random>
#include <format>

void timestamp(std::ostream& out);

int main(void){
    timestamp(std::cout);

    return 0;
}

/* Does the general timestamp stuff. */
void timestamp(std::ostream& out){
    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::uniform_int_distribution<std::size_t> dist(1, 60);

    std::size_t year = 2026;
    std::size_t month = 11;
    std::size_t day = 18;

    std::size_t hour = 0;
    std::size_t min = dist(gen);
    std::size_t sec = dist(gen);

    while ((hour + 1) < 2){
        if (sec >= 60){
            min++;
            sec = 0;
        } if (min >= 60){
            hour++;
            min = 0;
        } if (hour >= 24){
            day++;
            hour = 0;
        }

        out << year << "-" << month << "-" << day << "T" 
            << std::format("{:02}", hour) << ":" << std::format("{:02}", min) << ":" << std::format("{:02}", sec)
            << " \n";

        sec += dist(gen);
    }
    out << std::endl;
}
