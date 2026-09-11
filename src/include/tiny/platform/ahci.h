#ifndef AHCI_H
#define AHCI_H

#include <tiny/types.h>

struct hba_port {

};

struct hba_mem {
    struct hba_port ports[32];
};

void init_ahci();

#endif // AHCI_H
