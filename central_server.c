#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

/// DEFS ///
#define BUFLEN 512
#define PORT 9930
#define SHMSZ 1024
/// END OF DEFS ///



/// UTIL ///
/**
* @Name str_split
* @Param char* a_str , const char a_delim
* @Return char** result
*/
char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

/**
* @Name diep
* @Param char* s
*/
void diep(char *s)
{
    perror(s);
    exit(1);
}

/// END OF UTIL ///


/// STRUCT ////
struct Sensor
{
    char* ip;
    char* port;
    char* label;
    char* actions[3];
};

/**
* @Name newSensor
* @Return Sensor: mySensor
*/
struct Sensor *newSensor()
{
    struct Sensor *mySensor = malloc(sizeof(struct Sensor));
    assert(mySensor != NULL);
    return mySensor;
}

/**
* @Name createSensor
* @Param char* msg
* @Return Sensor : mySensor
*/
struct Sensor *createSensor(char * msg)
{
    struct Sensor *mySensor = malloc(sizeof(struct Sensor));
    assert(mySensor != NULL);

    char** tokens;
    tokens = str_split(msg, '#');

    if (tokens)
    {
        mySensor->label = *(tokens +0);
        mySensor->actions[0] = *(tokens +1);
        mySensor->actions[1] = *(tokens +2);
        mySensor->actions[2] = *(tokens +3);

        free(tokens);
    }

    return mySensor;
}

/// END OF STRUCT ///



/// BEGIN OF MAIN ///
int main()
{
    /// VARS FOR SOCKET ///
    struct sockaddr_in si_me, si_other;
    int sock, i, slen=sizeof(si_other);
    char buf[BUFLEN];
    /// END VARS FOR SOCKET ///

    /// VARS FOR SHARED MEMORY ///
    int shmid;
    key_t key;
    char *shm, *s;

    key = 5678;
    /// END OF VARS FOR SHARED MEMORY ///


    /// SOCKET PREP ///
    if ((sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        diep("socket");

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock,(struct sockaddr *) &si_me, sizeof(si_me))==-1)
        diep("bind");
    /// END SOCKET PREP ///


    /// SHARED MEMORY PREP ///

    /*
     * Create the segment.
     */
    if ((shmid = shmget(key, 27, IPC_CREAT | 0666)) < 0)
    {
        perror("shmget");
        exit(1);
    }
    /*
    * Now we attach the segment to our data space.
    */
    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1)
    {
        perror("shmat");
        exit(1);
    }

    /*
     * Now put some things into the memory for the
     * other process to read.
     */
    s = shm;

    /// END OF SHARED MEMORY PREP ///



    /// MAIN LOOP ///
    while(1)
    {
        /// RECIVE A MESSAGE FROM SENSORS ///
        if (recvfrom(sock, buf, BUFLEN, 0,(struct sockaddr *) &si_other, &slen)==-1)
        {
            printf("error");
            diep("recvfrom()");
        }else {

            /// add port to data ///
            char tmpPort[5];
            sprintf(tmpPort, "%d", ntohs(si_other.sin_port));
            strcat(buf, "#");
            strcat(buf, tmpPort);
            s = buf;
            printf("FINAL %s \n",buf);
            /// End add port to data ///




        }

    }


    return 0;
}



