#ifndef IOCTL_H
#define IOCTL_H

#include <linux/ioctl.h>

#define IOCTL_SET_NUM 2
#define IOCTL_SET _IO(IOCTL_SET_NUM, 0)

// #define IOCTL_GET_NUM 1
// #define IOCTL_GET _IO(IOCTL_GET_NUM, 0)

// #define IOCTL_DEC_NUM 3
// #define IOCTL_DEC _IO(IOCTL_DEC_NUM, 0)

#endif
