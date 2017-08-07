#include <linux/kd.h>
#include <sys/ioctl.h>

#include "car.hpp"

struct Move
{
    int up;
    int down;
    int left;
    int right;
};

int main(int argc, char **argv)
{
    int         res, err    = -1;
    int         fd          = -1;
    int         key_code    = 0;
    int         kbd_mode    = 0;
    string      tty         = "/dev/tty0";
    string      mac         = "00:16:53:13:CA:B4";
    Car         car;
    Move        move;

    memset(&move, 0x00, sizeof(move));
    printw("trying to connect to: '%s' ", mac.c_str());

    // try to connect
    fd = car.doConnect(mac);
    if (fd < 0){
        printw("[ FAILED ]\n");
        goto fail;
    }

    printw("[ SUCCESS ]\n");

    // getBattery(fd);

    fd = open(tty.c_str(), O_RDONLY);
    if (fd < 0){
        printw("cannot open '%s' for read\n",
            tty.c_str());
        goto fail;
    }

    res = ioctl(fd, KDGKBMODE, &kbd_mode);
    if (res < 0){
        printw("ioctl KDGETKEYCODE failed\n");
        goto fail;
    }

    res = ioctl(fd, KDSKBMODE, 0x02);
    if (res < 0){
        printw("ioctl KDSKBMODE 0x02 failed\n");
        goto fail;
    }
    do {
        key_code = 0;
        res = read(fd, &key_code, sizeof(key_code));

        if (0x67 == key_code){
            // up
            move.up = 1;
        } else if (0xe7 == key_code){
            move.up = 0;
        } else if (0x6c == key_code){
            // down
            move.down = 1;
        } else if (0xec == key_code){
            move.down = 0;
        } else if (0x69 == key_code){
            // left
            move.left = -1;
        } else if (0xe9 == key_code){
            move.left = 0;
        } else if (0x6a == key_code){
            // right
            move.right = 1;
        } else if (0xea == key_code){
            move.right = 0;
        } else {
            printw("unsupported key code: '%x'\n", key_code);
        }

        if (move.up && move.left){
            car.moveCar(0x00000000, 100,  Car::MOTOR_RIGHT);
            car.moveCar(0x00000000, 0,    Car::MOTOR_LEFT);
        } else if (move.up && move.right){
            car.moveCar(0x00000000, 0,    Car::MOTOR_RIGHT);
            car.moveCar(0x00000000, 100,  Car::MOTOR_LEFT);
        } else if (move.down && move.left){
            car.moveCar(0x00000000, -100, Car::MOTOR_RIGHT);
            car.moveCar(0x00000000, 0,    Car::MOTOR_LEFT);
        } else if (move.down && move.right){
            car.moveCar(0x00000000, 0,    Car::MOTOR_RIGHT);
            car.moveCar(0x00000000, -100, Car::MOTOR_LEFT);
        } else if (move.down){
            car.moveCar(0x00000000, -100, Car::MOTOR_RIGHT);
            car.moveCar(0x00000000, -100, Car::MOTOR_LEFT);
        } else if (move.up){
            car.moveCar(0x00000000, 100,  Car::MOTOR_RIGHT);
            car.moveCar(0x00000000, 100,  Car::MOTOR_LEFT);
        } else if (move.left){
            car.moveCar(0x00000000,  100, Car::MOTOR_RIGHT);
            car.moveCar(0x00000000, -100, Car::MOTOR_LEFT);
        } else if (move.right){
            car.moveCar(0x00000000,  100, Car::MOTOR_LEFT);
            car.moveCar(0x00000000, -100, Car::MOTOR_RIGHT);
        } else if ( not move.left
            &&      not move.right
            &&      not move.up
            &&      not move.down)
        {
            // key released
            car.moveCar(1, 0, Car::MOTOR_RIGHT);
            car.moveCar(1, 0, Car::MOTOR_LEFT);
        }
    } while (10 != key_code);

    // all ok
    err = 0;

out:
    if (fd > 0){
        close(fd);
        fd = -1;
    }
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

