// Last Update:2018-11-28 15:00:57
/**
 * @file ota.c
 * @brief 
 * @author liyq
 * @version 0.1.00
 * @date 2018-11-19
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "md5.h"
#include "cfg.h"
#include "dbg.h"
#include "main.h"
#include "ota.h"
#include "httptools.h"

#define OTA_FILE_MAX_SIZE 3096*1024 // 3M

int Download( const char *url, char *filename )
{
    char *buffer = ( char * ) malloc ( OTA_FILE_MAX_SIZE );// 3M
    int len = 0, ret = 0;
    FILE *fp = NULL;
    
    if ( !buffer ) {
        DBG_ERROR("malloc buffer error\n");
        return -1;
    }
    ret = LinkSimpleHttpGet( url, buffer, OTA_FILE_MAX_SIZE, &len ); 
    if ( LINK_SUCCESS != ret ) {
        DBG_ERROR("LinkSimpleHttpGet() error, ret = %d\n", ret );
        return -1;
    }

    if ( len <= 0 ) {
        DBG_ERROR("check length error, len = %d\n", len );
        return -1;
    }

    DBG_LOG("len = %d\n", len );
    fp = fopen( filename, "w+" );
    if ( !fp ) {
        DBG_ERROR("open file %s error\n", filename );
        return -1;
    }

    fwrite( buffer, len, 1, fp );
    fclose( fp );
    free( buffer );

    return 0;
}


int CheckUpdate( char *versionFile )
{
    int ret = 0;
    char *version = NULL;
    struct cfg_struct *cfg = NULL;
    char *version_key = "version_number";
    char versionFileUrl[512] = { 0 };

    sprintf( versionFileUrl, "%s/version.txt", gIpc.config.ota_url );
    DBG_LOG("start to download %s\n", versionFileUrl );
    ret = Download( versionFileUrl, versionFile );
    if ( ret != 0 ) {
        DBG_ERROR("get %s error, url : %s\n", versionFile, versionFileUrl );
        return -1;
    }

    DBG_LOG("get versionFile %s success,load it\n", versionFile );
    cfg = cfg_init();
    DBG_LOG("cfg = %p\n", cfg );
    if ( !cfg ) {
        DBG_ERROR("cfg is null\n");
        return -1;
    }
    if (cfg_load( cfg, versionFile ) < 0) {
        DBG_ERROR("Unable to load %s\n", versionFile );
        goto err;
    }

    DBG_LOG("start to parse the version number\n");
    version = cfg_get( cfg, version_key );
    if ( !version ) {
        DBG_ERROR("get version error\n");
        goto err;
    }

    DBG_LOG("the new version of remote is %s, current version is %s\n", version, gIpc.version );
    if ( strncmp( version, gIpc.version, strlen(version) ) == 0 ) {
        cfg_free( cfg );
        return 0;
    }

    cfg_free( cfg );

    return 1;
err:
    cfg_free( cfg );
    return -1;
}

void dump_buf( char *buf, int len, char *name )
{
    int i = 0;
    DBG_LOG("dump %s :\n", name);

    for ( i=0; i<len; i++ ) {
        printf("0x%02x ", buf[i] );
    }
    printf("\n");
}

int CheckMd5sum( char *versionFile, char *binFile )
{
    FILE *fp;
    unsigned char buffer[4096];
    size_t n;
    MD5_CONTEXT ctx;
    int i;
    char *remoteMd5 = NULL;
    struct cfg_struct *cfg;
    char *md5_key = "md5";
    char str_md5[16] = { 0 };

    cfg = cfg_init();
    if ( !cfg ) {
        DBG_ERROR("cfg init error\n");
        return -1;
    }
    if (cfg_load( cfg, versionFile ) < 0) {
        DBG_ERROR("Unable to load %s\n", versionFile );
        goto err;
    }

    remoteMd5 = cfg_get( cfg, md5_key );
    if ( !remoteMd5 ) {
        DBG_ERROR("get remoteMd5 error\n");
        goto err;
    }

    DBG_LOG("the md5 of remote is %s\n", remoteMd5 );
    fp = fopen ( binFile, "rb");
    if (!fp) {
        DBG_ERROR( "can't open `%s': %s\n", binFile, strerror (errno));
        goto err;
    }
    md5_init (&ctx);
    while ( (n = fread (buffer, 1, sizeof buffer, fp)))
        md5_write (&ctx, buffer, n);
    if (ferror (fp))
    {
        DBG_ERROR( "error reading `%s': %s\n", binFile, strerror (errno));
        goto err;
    }
    md5_final (&ctx);
    fclose (fp);

    dump_buf( (char *)ctx.buf, 16, "ctx.buf" );
    for ( i=0; i<16; i++ ) {
        sprintf( str_md5 + strlen(str_md5), "%02x", ctx.buf[i] );
    }

    DBG_LOG("str_md5 = %s, remoteMd5 = %s\n", str_md5, remoteMd5 );
    if ( memcmp( remoteMd5, str_md5, 32 ) == 0 ) {
        free( cfg );
        return 1;
    }

    free( cfg );
    return 0;
err:
    free( cfg );
    return -1;
}

void * UpgradeTask( void *arg )
{
    int ret = 0;
    char *binFile = "/tmp/AlarmProxy";
    char *versionFile = "/tmp/version.txt";
    char *target = "/tmp/oem/app/AlarmProxy";
    char cmdBuf[256] = { 0 };
    char binUrl[1024] = { 0 };


    for (;;) {
        if ( !gIpc.config.ota_enable ) {
            DBG_LOG("ota function not enable\n");
            sleep( gIpc.config.ota_check_interval );
        }

        if ( !gIpc.config.ota_url ) {
            DBG_ERROR("OTA_URL not set, please modify /tmp/oem/app/ip.conf and add OTA_URL\n");
            sleep( gIpc.config.ota_check_interval );
            continue;
        }

        sprintf( binUrl, "%s/AlarmProxy", gIpc.config.ota_url );
        DBG_LOG("start upgrade process\n");
        ret = CheckUpdate( versionFile );
        if ( ret <= 0 ) {
            DBG_LOG("there is no new version in server\n");
            sleep( gIpc.config.ota_check_interval );
            continue;
        } 

        DBG_LOG("start to download %s\n", binUrl );
        ret = Download( binUrl, binFile );
        if ( ret < 0 ) {
            DBG_ERROR("download file %s, url : %s error\n", binFile, binUrl );
            sleep( 5 );
            continue;
        }

        ret = CheckMd5sum( versionFile, binFile );
        if ( ret <= 0 ) {
            DBG_ERROR("check md5 error\n");
            sleep( 5 );
            continue;
        }

        DBG_LOG("check the md5 of file %s ok\n", binFile);
        if ( access( target, R_OK ) == 0 ) {
            ret = remove( target );
            if ( ret != 0 ) {
                DBG_ERROR("remove file %s error\n", target );
                sleep( 5 );
                continue;
            }
            break;
        } else {
            break;
        }
    }

    DBG_LOG("copy %s to %s\n", binFile, target );
    sprintf( cmdBuf, "cp %s %s", binFile, target );
    system( cmdBuf );

    DBG_LOG("chmod +x %s\n", target );
    memset( cmdBuf, 0, sizeof(cmdBuf) );
    sprintf( cmdBuf, "chmod +x %s", target );
    system( cmdBuf );

    DBG_LOG("the ota update success!!!!\n");

    /* notify main thread to exit */
    gIpc.running = 0;

    return NULL;
}

void StartUpgradeTask()
{
    pthread_t thread = 0;

    pthread_create( &thread, NULL, UpgradeTask, NULL );
}

