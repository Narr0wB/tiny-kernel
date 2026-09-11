
#include <tiny/mm/vasl.h>
#include <tiny/string.h>

#include <tiny/platform/acpi.h>

struct acpi_rsdp_descriptor *acpi_rsdp;
struct acpi_sdt_header      *acpi_xsdt_header;
paddr_t                     *acpi_tables;


void init_acpi(paddr_t rsdp)
{
    acpi_rsdp        = (struct acpi_rsdp_descriptor *)p_to_v(rsdp);
    acpi_xsdt_header = (struct acpi_sdt_header *)p_to_v(acpi_rsdp->xsdt_address);
    acpi_tables      = (paddr_t*)p_to_v(acpi_rsdp->xsdt_address + sizeof(struct acpi_sdt_header));
}