#include "log.hpp"
#include "utils.hpp"

void Utils::removeSpacesFromStart(
    string  &a_data)
{
    removeSpaces(a_data, 1, 0);
}

void Utils::removeSpacesFromEnd(
    string  &a_data)
{
    removeSpaces(a_data, 0, 1);
}

void Utils::removeSpaces(
    string  &a_data,
    int32_t a_from_start,
    int32_t a_from_end)
{
    string::size_type size = 0;

    if (a_from_start){
        while (1){
            size = a_data.size();
            if (!size){
                break;
            }
            if (' ' != a_data.at(0)){
                break;
            }
            a_data = a_data.substr(1);
        }
    }
    if (a_from_end){
        while (1){
            size = a_data.size();
            if (!size){
                break;
            }
            if (' ' != a_data.at(size - 1)){
                break;
            }
            a_data = a_data.substr(0, size - 1);
        }
    }
}

void Utils::chomp(
    string &a_str)
{
    while (a_str.size()){
        char c = a_str.at(a_str.size() - 1);
        if (    '\r' == c
            ||  '\n' == c)
        {
            a_str = a_str.substr(0, a_str.size() - 1);
            continue;
        }
        break;
    }
}

void Utils::chomp2(
    string &a_str)
{
    ChompState state = CHOMP_STATE_FIRST_N;

    while (a_str.size()){
        char c = a_str.at(a_str.size() - 1);
        switch (state){
            case CHOMP_STATE_DONE:
                break;

            case CHOMP_STATE_FIRST_R:
                if ('\r' == c){
                    a_str.erase(a_str.size() - 1);
                }
                state = CHOMP_STATE_DONE;
                break;

            case CHOMP_STATE_FIRST_N:
                if ('\n' == c){
                    a_str.erase(a_str.size() - 1);
                    state = CHOMP_STATE_FIRST_R;
                } else {
                    state = CHOMP_STATE_DONE;
                }
                break;

            default:
                PERROR("invalid state: '%d'\n", state);
                break;
        }
        if (CHOMP_STATE_DONE == state){
            break;
        }
    }
}

double Utils::fRand(
    const double &a_min,
    const double &a_max)
{
    double rand = (double)::rand() / RAND_MAX;
    double ret  = a_min + rand * (a_max - a_min);

    return ret;
}

