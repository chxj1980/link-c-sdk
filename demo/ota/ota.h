// Last Update:2018-11-22 17:51:02
/**
 * @file ota.h
 * @brief 
 * @author felix
 * @version 0.1.00
 * @date 2018-11-22
 */

#ifndef OTA_H
#define OTA_H

#define OTA_START_UPGRADE_EVENT 5

typedef struct {
    long type;
    int event;
} msg_t;

extern void StartUpgradeTask();


#endif  /*OTA_H*/
