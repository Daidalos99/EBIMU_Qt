#include <Windows.h>
#include <iostream>
#include <string>
#include <cmath>
#include "serial.cpp"

#define MY_SERIALPORT  8  // 연결된 시리얼 포트번호
#define SBUF_SIZE 2000

using namespace std;

char sbuf[SBUF_SIZE];
signed int sbuf_cnt = 0;

struct Quaternion {
    double w, x, y, z;
};

struct EulerAngles {
    double roll, pitch, yaw;
};

// this implementation assumes normalized quaternion
// converts to Euler angles in 3-2-1 sequence
EulerAngles ToEulerAngles(Quaternion q) {
    EulerAngles angles;

    // roll (x-axis rotation)
    double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
    double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
    angles.roll = std::atan2(sinr_cosp, cosr_cosp) * 57.2958; // rad to deg

    // pitch (y-axis rotation)
    double sinp = std::sqrt(1 + 2 * (q.w * q.y - q.x * q.z));
    double cosp = std::sqrt(1 - 2 * (q.w * q.y - q.x * q.z));
    angles.pitch = 2 * std::atan2(sinp, cosp) - M_PI / 2 * 57.2958; // rad to deg

    // yaw (z-axis rotation)
    double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
    double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
    angles.yaw = std::atan2(siny_cosp, cosy_cosp) * 57.2958; // rad to deg

    return angles;
}

char* my_strtok(char * str, char dm, int *result)
{
    int n;

    *result = 0;

    for (n = 0; n<100; n++)
    {
        if (str[n] == dm) { *result = 1;  break; }
        if (str[n] == NULL) break;
    }

    return &str[n+1];
}

int EBimuAsciiParser(int *id, float *item, unsigned int number_of_item)
{
    SERIALREADDATA srd;
    unsigned int n, i;
    char *addr;
    int result = 0;
    int ret;

    //	char *context;

    if (ReadSerialPort(MY_SERIALPORT, &srd) == ERR_OK)	// -7 == 0
    {
        if (srd.nSize) // 100
        {
            for (n = 0; n<srd.nSize; n++)
            {
                //printf("%d ", sbuf_cnt);
                //////////////////////////////////////////////////////////////////////
                sbuf[sbuf_cnt] = srd.szData[n];	//  initial sbuf_cnt = 0
                //printf("%c", srd.szData[n]);
                if (sbuf[sbuf_cnt] == '\r')  // 1줄 수신완료
                {
                    {
                        addr = my_strtok(sbuf, '-' ,&ret);
                        if (ret)
                        {
                            *id = (float)atoi(addr);

                            addr = my_strtok(sbuf, ',', &ret);
                            for (i = 0; i<number_of_item; i++)
                            {
                                item[i] = (float)atof(addr);
                                addr = my_strtok(addr, ',', &ret);
                            }

                            result = 1;
                        }
                    }
                }
                else if (sbuf[sbuf_cnt] == '\n')
                {
                    sbuf_cnt = -1;
                }

                sbuf_cnt++;
                if (sbuf_cnt >= SBUF_SIZE) sbuf_cnt = 0;
                ///////////////////////////////////////////////////////////////////////
            }
        }
    }

    return result;
}

int main()
{
    int id;
    float item[100];

    if (OpenSerialPort(MY_SERIALPORT, 115200, NOPARITY, 8, ONESTOPBIT) != ERR_OK)
    {
        printf("\n\rSerialport Error...");
        Sleep(2000);
        return 0;

    }

    Quaternion q;
    EulerAngles e;

    while (1)
    {
        if (EBimuAsciiParser(&id,item, 5))
        {
            q.z = item[0];
            q.y = item[1];
            q.x = item[2];
            q.w = item[3];
            printf("Quaternion: z:%f y:%f x:%f w:%f,  ", q.z, q.y, q.x, q.w);

            e = ToEulerAngles(q);
            printf("Yaw: %f\n", e.yaw);
        }
    }
    return 0;
}
