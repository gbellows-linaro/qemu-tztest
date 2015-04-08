#include "el1_common.h"
#include "elf.h"
#include "mem_util.h"

/* Simple ELF loader for loading EL0 image */
void *el1_load_el0(char *elfbase, char *start_va)
{
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)elfbase;
    size_t off;
    int i;

    /* Map the ELF header in so we can determine how much more to map */
    mem_map_pa((uintptr_t)elfbase, (uintptr_t)elfbase, 0, PTE_USER_RW);

    /* Make sure this is an appropriate ELF image */
    if (ehdr->e_ident[EI_MAG0] != ELFMAG0 ||
        ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
        ehdr->e_ident[EI_MAG2] != ELFMAG2 ||
        ehdr->e_ident[EI_MAG3] != ELFMAG3) {
        DEBUG_MSG("Invalid ELF header, exiting...\n");
        SMC_EXIT();
    } else if (ehdr->e_machine != EM_ARM && ehdr->e_machine != EM_AARCH64) {
        DEBUG_MSG("Incorrect ELF type (type = %d, machine = %d), exiting...\n",
                  ehdr->e_type, ehdr->e_machine);
        SMC_EXIT();
    } else {
        DEBUG_MSG("Loading %s EL0 test image...\n",
                  (ehdr->e_machine == EM_ARM) ?  "aarch32" : "aarch64");
    }

    /* Size of the ELF to map */
    size_t elf_len = ehdr->e_shoff + (ehdr->e_shentsize * ehdr->e_shnum);

    /* Finish mapping the remainder of the ELF pages in if any */
    for (off = 0x1000; off < elf_len; off += 0x1000) {
        mem_map_pa((uintptr_t)elfbase + off, (uintptr_t)elfbase + off,
                   0, PTE_USER_RW);
    }

    Elf32_Shdr *shdr = (Elf32_Shdr *)((char *)elfbase + ehdr->e_shoff);

    Elf32_Shdr *strshdr = &shdr[ehdr->e_shstrndx];
    char *strsec = (char *)ehdr + strshdr->sh_offset;
    for (i = 0; i < ehdr->e_shnum; i++) {
        char *secname = strsec + shdr[i].sh_name;
        if (!strcmp(secname, ".text") || !strcmp(secname, ".data")) {
            uintptr_t sect = (uintptr_t)((char *)elfbase + shdr[i].sh_offset);
            char *base_va = start_va + shdr[i].sh_addr;
            DEBUG_MSG("\tloading %s section: 0x%x bytes @ 0x%lx\n",
                      secname, shdr[i].sh_size, base_va);
            for (off = 0; off < shdr[i].sh_size; off += 0x1000) {
                mem_map_va((uintptr_t)(base_va + off));
                memcpy((void *)(base_va + off), (void *)(sect + off), 0x1000);
            }
        }
    }

    /* Unmap the FLASH ELF image */
    for (off = 0; off < elf_len; off += 0x1000) {
        mem_map_va((uintptr_t)elfbase + off);
    }

    return (void *)(start_va + ehdr->e_entry);
}

