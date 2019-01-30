// Last Update:2019-01-30 16:49:20
/**
 * @file dev_core.c
 * @brief 
 * @author liyq
 * @version 0.1.00
 * @date 2018-10-19
 */
#include "dev_core.h"
#include "stream.h"

static CoreDevice gCoreDevice, *pCoreDevice = &gCoreDevice;

int CoreDeviceInit( int audioType, int subStreamEnable, VideoFrameCb videoCb, AudioFrameCb audioCb )
{
    if ( pCoreDevice->pCaptureDevice ) {
        pCoreDevice->pCaptureDevice->init( audioType, subStreamEnable,  videoCb, audioCb );
    }

    return 0;
}

int CoreDeviceDeInit()
{
    if ( pCoreDevice->pCaptureDevice 
         && pCoreDevice->pCaptureDevice->deInit )
        pCoreDevice->pCaptureDevice->deInit();

    return 0;
}

int CoreDeviceGetDevId( char *devId )
{
    if ( pCoreDevice->pCaptureDevice 
         && pCoreDevice->pCaptureDevice->getDevId )
        pCoreDevice->pCaptureDevice->getDevId( devId );

    return 0;
}

int CoreDeviceStartStream( int streamType )
{
    if ( pCoreDevice->pCaptureDevice 
         && pCoreDevice->pCaptureDevice->startStream )
        pCoreDevice->pCaptureDevice->startStream( streamType );
    return 0;
}

int CoreDeviceIsAudioEnable()
{
    if ( pCoreDevice->pCaptureDevice 
         && pCoreDevice->pCaptureDevice->isAudioEnable )
        return pCoreDevice->pCaptureDevice->isAudioEnable();

    return -1;
}

int CoreDeviceRegisterAlarmCb( int (*alarmCb)( int alarm, void *data ) )
{
    if ( pCoreDevice->pCaptureDevice 
         && pCoreDevice->pCaptureDevice->registerAlarmCb )
        pCoreDevice->pCaptureDevice->registerAlarmCb( alarmCb );

    return 0;
}

int CoreDeviceCaptureJpeg( int stream, int quality, char *path, char *filename)
{
    if ( pCoreDevice->pCaptureDevice 
         && pCoreDevice->pCaptureDevice->captureJpeg )
        pCoreDevice->pCaptureDevice->captureJpeg( stream, quality, path, filename );
    return 0;
}

int CoreDeviceGetAudioEncodeType()
{
    if ( pCoreDevice->pCaptureDevice 
         && pCoreDevice->pCaptureDevice->getAudioEncodeType )
        return pCoreDevice->pCaptureDevice->getAudioEncodeType();
}

CoreDevice * NewCoreDevice()
{
    pCoreDevice->init = CoreDeviceInit;
    pCoreDevice->deInit = CoreDeviceDeInit;
    pCoreDevice->getDevId = CoreDeviceGetDevId;
    pCoreDevice->startStream = CoreDeviceStartStream;
    pCoreDevice->isAudioEnable = CoreDeviceIsAudioEnable;
    pCoreDevice->registerAlarmCb = CoreDeviceRegisterAlarmCb;
    pCoreDevice->captureJpeg = CoreDeviceCaptureJpeg;
    pCoreDevice->getAudioEncodeType = CoreDeviceGetAudioEncodeType;

    return pCoreDevice;
}

int CaptureDeviceRegister( CaptureDevice *pDev )
{
    pCoreDevice->pCaptureDevice = pDev;
}

