#ifndef ROUTE_CLASS_H
#define ROUTE_CLASS_H

/*
 *  Roman E. Chechnev
 */

#include <sys/time.h>
#include <string.h>
#include <string>
#include <vector>

#include "log.h"
#include "shttp.h"
#include "net/icmp.h"

#define TMP_BUF_SIZE 512

using namespace std;

/*
 * routeRecordClass
 *
 * XXX подумать над этим... возможно все не верно TODO
 *
 * Данный класс описывает маршрут до клиента и предоставляет
 * возможности проверки его работоспособности.
 *
 * На данный момент Jun 4, 2010 предполагается следующие использование
 * маршрута:
 *
 * 1. описание канального маршрута
 *    обязательные поля:
 *    dev       - канальное устройство для маршрута
 *    lladdr    - канальный адрес маршрута
 *    src_huid  - huid того, кто запрашивает маршрут, он нужен
 *                для того чтобы конечная сторона знала куда
 *                отправлять ответы. Попытаюсь объяснить:
 *                представьте хост A и B, хост A строит
 *                предположительный маршрут до хоста B,
 *                к примеру через udp транспорт и отправляет ему icmp
 *                пакет от имени src_huid.
 *                Сторона B получает udp пакет, но к примеру
 *                её udp транспорт не работает для отправки
 *                (из-за провайдера|админа etc.)
 *                зная src_huid icmp пакета, сторона B
 *                принимает все усилия для доставки icmp ответа
 *                через другие транспорты и дружественные хосты.
 *                К примеру ей это удалось и хост A получил ответ
 *                через транспорт icq, хост A помечает, что
 *                данный маршрут жив, а трафик будет ходить
 *                ассиметрично, в одну сторону одним транспортом,
 *                в другую - другим, но ведь будет!
 *
 * 2. описание сетевого маршрута
 *    обязательные поля:
 *    src_huid  - huid того, кто запрашивает маршрут, см. выше.
 *    dst_huid  - huid промежуточного хоста.
 *                будет сделана попытка установить маршрут через
 *                третий хост. По dst_huid будут получены канальные
 *                адреса и далее как в пункте 1.
 *
 */
class routeRecordClass
{
    public:
        routeRecordClass(
            const char *_dev,
            const char *_lladdr,
            const char *_src_huid,
            const char *_dst_huid)
        {
            dev               = _dev;
            lladdr            = _lladdr;
            src_huid          = _src_huid;
            dst_huid          = _dst_huid;
            last_ping.tv_sec  = 0;
            last_ping.tv_usec = 0;
            clearStats();
            PDEBUG(5, "MORIK !!! src_huid: %s\n", _src_huid);
            PDEBUG(5, "MORIK !!! dst_huid: %s\n", _dst_huid);
        };
        ~routeRecordClass(){};

        const char *getDev(){
            // ping via dev
            return dev.c_str();
        };

        const char *getLladdr(){
            // ping to hw addr
            return lladdr.c_str();
        };

        const char *getSrcHuid(){
            // ping from
            return src_huid.c_str();
        };

        const char *getDstHuid(){
            // ping to
            return dst_huid.c_str();
        };

        void ping(){
            icmpClass icmp = icmpClass(dev);
            gettimeofday(&last_ping, NULL);
            icmp.ping(
                src_huid,                   // src huid
                dst_huid,                   // dst huid
                (uint8_t*)lladdr.c_str(),   // dst hw address
                (uint32_t)(char *)this      // ID
            );
        };

        void clearStats(){
            pings_count     = 0;
            sum_ping_time   = 0;
            min_ping_time   = 0;
            max_ping_time   = 0;
            avg_ping_time   = 0;
        };

        static void dump_header(){
            char buf[TMP_BUF_SIZE];
            snprintf(buf, sizeof(buf) - 1, "%-10.10s %-20.20s"
                " %-10.10s %-4.4s %-6.6s %-6.6s %-6.6s",
                "DEV", "LLADDR",
                "LAST_PING", "PCONT", "PMIN", "PAVG", "PMAX"
            );
            PDEBUG(5, "%s\n", buf);
            return;
        }
        void dump(){
            char buf[TMP_BUF_SIZE];
            snprintf(buf, sizeof(buf) - 1, "%-10.10s %-20.20s"
                " %10.10d %4.4d %4.4f %4.4f %4.4f",
                dev.c_str(),
                lladdr.c_str(),
                last_ping.tv_sec,
                pings_count,
                min_ping_time,
                avg_ping_time,
                max_ping_time
            );
            PDEBUG(5, "%s\n", buf);
        }

        void addPingRes(struct timeval *ping_res){
            pings_count++;
            double last_ping_time = last_ping.tv_sec
                + (last_ping.tv_usec/1e6);
            memcpy(&last_ping, ping_res, sizeof(last_ping));
            double time = (ping_res->tv_sec + (ping_res->tv_usec/1e6))
                - last_ping_time;
            sum_ping_time += time;
            avg_ping_time = sum_ping_time/pings_count;
            if (!min_ping_time)       min_ping_time = time;
            if (time > max_ping_time) max_ping_time = time;
            if (time < min_ping_time) min_ping_time = time;
        };

    private:
        string  lladdr;         //!< канальный адрес
        string  dev;            //!< канальное устройство
        string  src_huid;       //!< сетевой адрес отправителя
        string  dst_huid;       //!< сетевой адрес назначения

        int     pings_count;    //!< количество пингов
        double  sum_ping_time;  //!< суммарное время по всем пингам
        double  min_ping_time;  //!< минимальное время пинга
        double  max_ping_time;  //!< максимальное время пинга
        double  avg_ping_time;  //!< среднее время пинга
        struct  timeval last_ping; //!< время последней проверки
};

typedef vector <routeRecordClass> Routes;

class routeClass
{
    public:
            routeClass();
            ~routeClass();

            void icmpReply(__u8 *buf, uint32_t len);
            void checkRoutes();
            void addRoute(routeRecordClass &route);
            void dump();

    private:
            Routes  routes; //!< все маршруты
};

#endif



