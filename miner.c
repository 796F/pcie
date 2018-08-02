#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <dirent.h>
#include <time.h>
#include <signal.h>
#include <stdint.h>

static uint16_t pci_vendor_id = 0x10EE;
static uint16_t pci_device_id = 0x9011;
static char pci_path[64] = "/sys/bus/pci/devices/xxxxxxxxxxxx/resource0";
static int pci_fd;
static volatile uint32_t *pci_base;

#define SERIAL_READ_SIZE 60

int main(int argc, char *argv[])
{
    int i, rc, fd, ret;
    unsigned char data[80], midstate[32], send_buf[80], nonce_buf[SERIAL_READ_SIZE];
    uint32_t *target;
    bool display_summary = false;

    uint32_t nonce, hash[8];
    int64_t hash_count;
    struct timeval tv_start, tv_finish, elapsed, tv_end, diff;

    unsigned char *b = (unsigned char *)send_buf;
    uint32_t *b32 = (uint32_t *)send_buf;
    uint32_t value = 0x00000000;
    uint32_t old_value = 0x00000000;

    // Locate PCI device
    pci_path[20] = 0;
    DIR *dir = opendir(pci_path);
    if (!dir)
    {
        printf("cannot open pci on dir %s", pci_path);
        return -1;
    }
    
    pci_path[20] = '/';
    memcpy(pci_path + 34, "config", 7);
    struct dirent *entry;
    bool found = false;
    
    while (!found && (entry = readdir(dir)))
    {
        if (strlen(entry->d_name) != 12)
            continue;
        memcpy(pci_path + 21, entry->d_name, 12);
        int fd = open(pci_path, O_RDONLY);
        if (fd < 0)
            continue;
        struct
        {
            uint16_t vid;
            uint16_t pid;
        } devid;
        if (read(fd, &devid, sizeof(devid)) == sizeof(devid) && devid.vid == pci_vendor_id && devid.pid == pci_device_id)
            found = true;
        close(fd);
    }
    closedir(dir);
    memcpy(pci_path + 34, "resourc", 7);
    if (!found)
    {
        printf("Could not find PCI device with ID %04x:%04x", pci_vendor_id, pci_device_id);
        return -1;
    }

    // Get access to PCI device
    pci_fd = open(pci_path, O_RDWR);
    if (pci_fd <= 0)
    {
        printf("Could not open PCI device %s", pci_path);
        return -1;
    }
    pci_base = (volatile uint32_t *)mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, pci_fd, 0);
    if (pci_base == MAP_FAILED)
    {
        printf("Could not mmap PCI device %s", pci_path);
        return -1;
    } else {
        printf("MMAP WORKED!");
    }
    
    return 0;
}
