/*
 *copyright (C) 2014 System S/W Group, Samsung Electronics.
 */

#ifndef __LIB_BLOCK_H__
#define __LIB_BLOCK_H__

#include <sys/types.h>
#include <config.h>

#define BOOTMODE_MMC 0x11
#define BOOTMODE_UFS 0x22

#define SECTOR_SIZE		512
#define SECTOR_SHIFT		9

#define BYTE_TO_BLOCK(x) (((x - 1) >> 9) + 1)

void blk_init(int bootdevice);
int blk_close(void);
u32 blk_card_total_sector(unsigned int lun);
int blk_bread(u8 *p, u32 lun, u32 from, u32 size);
int blk_bread_bootsector(u8 *buffer, u32 start, u32 size);
int blk_bwrite(u8 *p, u32 lun, u32 from, u32 size);
int blk_bwrite_bootsector(u8 *binary, u32 start, u32 size);
int blk_berase_bootsectorB(u32 start, u32 end);
int blk_bread_bootsectorB(u8 *buffer, u32 start, u32 size);
int blk_bwrite_bootsectorB(u8 *binary, u32 start, u32 size);
int blk_berase(u32 lun, u32 from, u32 blknum);
int blk_boot_binary_write(u8 *binary, u32 size);
int blk_bootpart_open(void);
int blk_bootpart_close(void);
int blk_set_boot_wp(int enable);
int blk_set_user_wp(u32 start, u32 size);
void blk_burstmode_enable(void);
int blk_power_notification(u8 value);
int blk_rpmb_open(void);
int blk_rpmb_close(void);
ulong blk_rpmb_read_data(void *dst, ulong start, ulong blkcnt);
ulong blk_rpmb_write_data(u8 *src, ulong start, ulong blkcnt);
ulong blk_rpmb_authentication_key(void);
void blk_pnm_print(void);
void blk_get_serialno(char *uid);

#endif	/* __LIB_BLOCK_H__ */

