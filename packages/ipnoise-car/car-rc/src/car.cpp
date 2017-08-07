#include "car.hpp"

Car::Car()
{

}
Car::~Car()
{

}

int Car::resetMotor(
    Car::Motor  a_motor)
{
    int     ret         = 0;
    int     res         = 0;
    bool    response    = 1;

    unsigned char cmd[] = {
        0x04,                   // | telegram len
        0x00,                   // |
        response
            ? 0x00              // response required
            : 0x80,             // response not required
        0x0a,                   // command type
        (unsigned char)a_motor, // output port (0-2) 0xff - all
        0x01                    // relative?
                                // 1 - last position,
                                // 0 - absolute position
    };

    char answer[] = { 0x00, 0x00, 0x00 };

    if (m_fd < 0){
        fprintf(stderr, "we are not connected\n");
        goto fail;
    }

    res = write(m_fd, cmd, sizeof(cmd));
    // printw("wrote: '%d'\n", res);
    if (res < 0){
        goto fail;
    }

    if (response){
        res = read(m_fd, answer, sizeof(answer));
        // printw("read: '%d'\n", res);
        if (res < 0){
            goto fail;
        }

        // printw("%2.2x\n", answer[0]);
        // printw("%2.2x\n", answer[1]);
        // printw("%2.2x\n", answer[2]);
    }
out:
    return ret;

fail:
    if (ret >= 0){
        ret = -1;
    }
    goto out;
}

int Car::getMotorState(
    Car::Motor      a_motor,
    Car::RunState   &a_run_state)
{
    int         ret      = 0;
    int         res      = 0;
    bool        response = 1;
    OutputState state;

    memset(&state, 0x00, sizeof(state));

    unsigned char cmd[] = {
        0x03,                  // | telegram len
        0x00,                  // |
        response
            ? 0x00             // 0x00 - response required
            : 0x80,            // 0x00 - response not required
        0x06,                  // command type
        (unsigned char)a_motor // output port (0-2) 0xff - all
    };

    if (m_fd < 0){
        fprintf(stderr, "we are not connected\n");
        goto fail;
    }

    res = write(m_fd, cmd, sizeof(cmd));
    // printw("wrote: '%d'\n", res);
    if (res < 0){
        goto fail;
    }

    if (response){
        res = read(m_fd, &state, sizeof(state));
        // printw("read: '%d'\n", res);
        if (res < 0){
            goto fail;
        }

        // printw("%2.2x\n", answer[0]);
        // printw("%2.2x\n", answer[1]);
        // printw("%2.2x\n", answer[2]);
    }

    a_run_state = (RunState)state.run_state;

out:
    return ret;

fail:
    if (ret >= 0){
        ret = -1;
    }
    goto out;
}

int Car::moveCar(
    unsigned int    a_len,
    int             a_power,
    Car::Motor      a_motor)
{
    int             ret         = 0;
    int             res         = 0;
    bool            response    = 1;

    unsigned char cmd[] = {
        0x0C,       // | telegram len
        0x00,       // |
        response    // 0x00 - response required, 0x80 - not
            ? 0x00
            : 0x80,
        0x04,       // command type
        a_motor,    // output port (0-2) 0xff - all
        a_power,    // power -100 .. +100
        0x01,       // mode byte (bit field)
        0x00,       // regulation mode (enum)
        0,          // turn ration -100 .. +100
        0x20,       // run state (enum)
        (a_len & 0xff000000) >> 24, // |
        (a_len & 0x00ff0000) >> 16, // | tacho limit, 0 - forever
        (a_len & 0x0000ff00) >> 8,  // |
        (a_len & 0x000000ff)        // |
    };
    char answer[] = { 0x00, 0x00, 0x00 };

    if (m_fd < 0){
        fprintf(stderr, "we are not connected\n");
        goto fail;
    }

    res = write(m_fd, cmd, sizeof(cmd));
    // printw("wrote: '%d'\n", res);
    if (res < 0){
        goto fail;
    }

    if (response){
        res = read(m_fd, answer, sizeof(answer));
        // printw("read: '%d'\n", res);
        if (res < 0){
            goto fail;
        }

        // printw("%2.2x\n", answer[0]);
        // printw("%2.2x\n", answer[1]);
        // printw("%2.2x\n", answer[2]);
    }
out:
    return ret;

fail:
    if (ret >= 0){
        ret = -1;
    }
    goto out;
}

int Car::getBattery()
{
    int     ret         = 0;
    int     res         = 0;
    char    cmd[]       = { 0x02, 0x00, 0x00, 0x0b };
    char    answer[]    = { 0x00, 0x00, 0x00, 0x00 };

    if (m_fd < 0){
        fprintf(stderr, "we are not connected\n");
        goto fail;
    }

    res = write(m_fd, cmd, sizeof(cmd));
    printw("wrote: '%d'\n", res);
    if (res < 0){
        goto fail;
    }

    res = read(m_fd, answer, sizeof(answer));
    printw("read: '%d'\n", res);
    if (res < 0){
        goto fail;
    }

    printw("%2.2x\n", answer[0]);
    printw("%2.2x\n", answer[1]);
    printw("%2.2x\n", answer[2]);
    printw("%2.2x\n", answer[3]);

out:
    return ret;

fail:
    if (ret >= 0){
        ret = -1;
    }
    goto out;
}

int Car::doConnect(const string &a_mac)
{
    int res;
    struct sockaddr_rc addr = { 0 };

    // allocate a socket
    m_fd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (m_fd < 0){
        printw("cannot open socket\n");
        goto fail;
    }

    // set the connection parameters (who to connect to)
    addr.rc_family  = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t)1;
    str2ba(a_mac.c_str(), &addr.rc_bdaddr);

    // connect to server
    res = connect(m_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (res < 0){
        goto fail;
    }

out:
    return m_fd;

fail:
    if (m_fd >= 0){
        close(m_fd);
        m_fd = -1;
    }
    goto out;
}

