#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h> /* for strncpy */
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <net/if.h>


/// DEFS ///
#define BUFLEN 512
#define INTERFACE "wlan0"
#define PORT 9930
#define SRV_IP "192.168.1.4" // UPDATE VALUE FOR NEW SERVER IP
#define CAP_NAME "Temperature Sensor";
/// END OF DEFS ///


/// UTIL ///
int getTemp()
{
    srand(time(NULL));

    int r = 0;
    do
    {
        r = rand();
    }
    while(r>45 || r<0);
    return r;
}


int setTemp(int t)
{
    return t;
}

void diep(char *s)
{
    perror(s);
    exit(1);
}


char* getIP()
{
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, INTERFACE, IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);

    close(fd);
    return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}
/// END UTIL ///


/// STRUCT ////
struct Sensor
{
    char* ip;
    char* port;
    char* label;
    char* actions[3];
};


struct Sensor *createSensor()
{
    struct Sensor *mySensor = malloc(sizeof(struct Sensor));
    assert(mySensor != NULL);
    mySensor->label = "";
    mySensor->actions[0] = "NotAv";
    mySensor->actions[1] = "NotAv";
    mySensor->actions[2] = "NotAv";
    mySensor->ip = getIP();
    mySensor->port = "NotDEF";
    return mySensor;
}

char * getInitialMessage(struct Sensor *s)
{
    char* msg;
    msg = malloc(512);
    strcpy(msg, s->label);
    strcat(msg, "#");
    strcat(msg, s->actions[0]);
    strcat(msg, "#");
    strcat(msg, s->actions[1]);
    strcat(msg, "#");
    strcat(msg, s->actions[2]);
    strcat(msg, "#");
    strcat(msg, s->ip);
    return msg;
}
/// END STRUCT ///




int main(void)
{
    struct sockaddr_in si_other;
    int s, i, slen=sizeof(si_other);
    char buf[BUFLEN];

    //TEST IF First RUN
    int firstRun = 1;
    //VARS FOR SENSOR
    struct Sensor *mySensor = createSensor();
    mySensor->label = CAP_NAME;
    mySensor->actions[0] = "GetTemp";
    mySensor->actions[1] = "SetTemp";

    char* msg;
    msg = malloc(512);

    msg = getInitialMessage(mySensor);

    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        diep("socket");

    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    if (inet_aton(SRV_IP, &si_other.sin_addr)==0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    if(firstRun!=0)
    {
        printf("FIRST RUN \n");
        printf("%s", mySensor->label);
        firstRun =0;
        //SEND INITIAL DISVOVERY MESSAGE
        if (sendto(s, msg, BUFLEN, 0,(struct sockaddr *) &si_other, slen)==-1)
        {
            diep("sendto()");
        }
    }

    close(s);
    return 0;
}
