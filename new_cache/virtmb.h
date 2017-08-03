#ifndef _VIRTMB_H
#define _VIRTMB_H

int virt_mb_filldev( char *sName, int mb_id);
int virt_mb_devread(int ID, int reg_num);
void virt_mb_devlist (void);

#endif