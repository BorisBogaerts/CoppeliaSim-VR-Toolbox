#ifndef _EXTAPIPLATFORM__
#define _EXTAPIPLATFORM__

#ifdef __cplusplus
extern "C" {
#endif
#ifdef _WIN32
    /* on older win compilers stdint.h can be missing */
    typedef unsigned char uint8_t;
    typedef short int16_t;
    typedef unsigned short uint16_t;
    typedef int int32_t;
    typedef unsigned int uint32_t;
#else
    #include <stdint.h>
#endif

#define SOCKET_MAX_PACKET_SIZE 1300 /* in bytes. Keep between 200 and 30000 */
#define SOCKET_HEADER_LENGTH 6 /* WORD0=1 (to detect endianness), WORD1=packetSize, WORD2=packetsLeftToRead */
#define SOCKET_TIMEOUT_READ 10000 /* in ms */

typedef char simxChar;              /* always 1 byte */
typedef uint8_t simxUChar;          /* always 1 byte */
typedef int16_t simxShort;          /* always 2 bytes */
typedef uint16_t simxUShort;        /* always 2 bytes */
typedef int32_t simxInt;            /* always 4 bytes */
typedef uint32_t simxUInt;          /* always 4 bytes */
typedef float simxFloat;            /* always 4 bytes */
typedef double simxDouble;          /* always 8 bytes */
typedef void simxVoid;


#ifdef _WIN32
    #define SIMX_THREAD_RET_TYPE simxVoid
    #define SIMX_THREAD_RET_LINE return
#elif defined (__linux) || defined (__APPLE__)
    #define SIMX_THREAD_RET_TYPE simxVoid*
    #define SIMX_THREAD_RET_LINE return(0)
#endif


/* Following functions only needed for testing endianness robustness */
simxShort extApi_endianConversionShort(simxShort shortValue);
simxUShort extApi_endianConversionUShort(simxUShort shortValue);
simxInt extApi_endianConversionInt(simxInt intValue);
simxFloat extApi_endianConversionFloat(simxFloat floatValue);
simxDouble extApi_endianConversionDouble(simxDouble floatValue);

/* Following functions might be platform specific */
simxFloat extApi_getFloatFromPtr(const simxUChar* ptr);
simxInt extApi_getIntFromPtr(const simxUChar* ptr);
simxUChar* extApi_allocateBuffer(simxInt bufferSize);
simxVoid extApi_releaseBuffer(simxUChar* buffer);
simxVoid extApi_createMutexes(simxInt clientID);
simxVoid extApi_deleteMutexes(simxInt clientID);
simxVoid extApi_lockResources(simxInt clientID);
simxVoid extApi_unlockResources(simxInt clientID);
simxVoid extApi_lockSendStart(simxInt clientID);
simxVoid extApi_unlockSendStart(simxInt clientID);
simxVoid extApi_createGlobalMutex();
simxVoid extApi_deleteGlobalMutex();
simxVoid extApi_globalSimpleLock();
simxVoid extApi_globalSimpleUnlock();
simxInt extApi_getTimeInMs();
simxInt extApi_getTimeDiffInMs(simxInt lastTime);
simxVoid extApi_initRand();
simxFloat extApi_rand();
simxVoid extApi_sleepMs(simxInt ms);
simxVoid extApi_switchThread();
simxUChar extApi_areStringsSame(const simxChar* str1,const simxChar* str2);
simxInt extApi_getStringLength(const simxChar* str);
simxUChar* extApi_readFile(const simxChar* fileName,simxInt* len);
simxUChar extApi_launchThread(SIMX_THREAD_RET_TYPE (*startAddress)(simxVoid*));
simxVoid extApi_endThread();
simxUChar extApi_connectToServer_socket(simxInt clientID,const simxChar* theConnectionAddress,simxInt theConnectionPort);
simxVoid extApi_cleanUp_socket(simxInt clientID);
simxInt extApi_send_socket(simxInt clientID,const simxUChar* data,simxInt dataLength);
simxInt extApi_recv_socket(simxInt clientID,simxUChar* data,simxInt maxDataLength);

simxUChar extApi_connectToServer_sharedMem(simxInt clientID,simxInt theConnectionPort);
simxVoid extApi_cleanUp_sharedMem(simxInt clientID);
simxInt extApi_send_sharedMem(simxInt clientID,const simxUChar* data,simxInt dataLength);
simxUChar* extApi_recv_sharedMem(simxInt clientID,simxInt* dataLength);

#ifdef __cplusplus
}
#endif

#endif /* _EXTAPIPLATFORM__ */          
