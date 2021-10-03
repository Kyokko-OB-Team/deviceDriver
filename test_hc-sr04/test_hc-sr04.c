#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "../include/hc-sr04.h"

#define TEST_HCSR04_MAJ_VER (0x00)
#define TEST_HCSR04_MIN_VER (0x01)

#define DEVICE_FILE "/dev/hc-sr040"  //GPIOドライバ

int main (int argc, char *argv[]) {
  int fd;
  drv_rq_t rq;

  printf("%s ver.%d.%d\n", argv[0], TEST_HCSR04_MAJ_VER, TEST_HCSR04_MIN_VER);

  fd = open(DEVICE_FILE, O_RDWR);
  if (0 > fd) {
    fprintf(stderr, "Can't open " DEVICE_FILE "\n");
    exit(1);
  }
  if (0 > ioctl(fd, GPIO_HCSR04_EXEC_MEASURE_DISTANCE, &rq)) {
    perror("GPIO_HCSR04_EXEC_MEASURE_DISTANCE");
  } else {
    printf("exec measure distance.\n");
  }
  sleep(1);
  if (0 > ioctl(fd, GPIO_HCSR04_GET_DISTANCE, &rq)) {
    perror("GPIO_HCSR04_GET_DISTANCE");
  } else {
    printf("exec get distance.\ndistance: %dcm\n", rq.value);
  }

  close(fd);

  return 0;
}

