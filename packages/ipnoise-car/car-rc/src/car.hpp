#ifndef CAR_HPP
#define CAR_HPP

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string>

#define printw(args...) \
    fprintf(stderr, args);

using namespace std;

class Car
{
    public:
        Car();
        virtual ~Car();

        enum Motor
        {
            MOTOR_RIGHT   = 0x01,
            MOTOR_LEFT    = 0x02,
            MOTOR_ALL     = 0xff
        };

        enum RunState
        {
            MOTOR_RUN_STATE_IDLE        = 0x00,
            MOTOR_RUN_STATE_RAMPUP      = 0x10,
            MOTOR_RUN_STATE_RUNNING     = 0x20,
            MOTOR_RUN_STATE_RAMPDOWN    = 0x40
        };

        struct OutputState
        {
            unsigned char telegram_type;  // reply (0x02)
            unsigned char command_type;   // ostate (0x06)
            unsigned char status;         // status
            unsigned char output_port;    // range 0-2
            unsigned char power;          // -100 .. +100
            unsigned char mode;           // bit field
            unsigned char regulator_mode; // enum
            unsigned char turn_ration;    // -100 .. +100
            unsigned char run_state;      // enum
            unsigned int  tacho_limit;
            unsigned int  block_tacho_limit;
            unsigned int  rotation_count;
        };

        int resetMotor(
            Car::Motor  a_motor
        );

        int getMotorState(
            Car::Motor      a_motor,
            Car::RunState   &a_run_state
        );

        int moveCar(
            unsigned int    a_len,
            int             a_power,
            Car::Motor      a_motor
        );

        int getBattery();
        int doConnect(const string &a_mac);

    private:
        int m_fd;
};

#endif

