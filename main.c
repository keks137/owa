#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define ELF3_HDR_SIZE 0x78 // ELF header + 1 program header
#define VIRTUAL_OFFSET 0x400000

static void write_elf3(FILE *f, uint8_t *code, size_t codelen, uint64_t entry)
{
	uint8_t hdr[ELF3_HDR_SIZE] = { 0 };

	memcpy(hdr, "\x7f"
		    "ELF",
	       4);
	hdr[4] = 2; // ELF64
	hdr[5] = 1; // little-endian
	hdr[6] = 1; // version
	hdr[7] = 3; // OSABI = Linux

	*(uint16_t *)(hdr + 0x10) = 2; // e_type = ET_EXEC
	*(uint16_t *)(hdr + 0x12) = 0x3e; // e_machine = EM_X86_64
	*(uint32_t *)(hdr + 0x14) = 1; // e_version
	*(uint64_t *)(hdr + 0x18) = entry; // e_entry
	*(uint64_t *)(hdr + 0x20) = 0x40; // e_phoff
	*(uint16_t *)(hdr + 0x34) = 0x40; // e_ehsize
	*(uint16_t *)(hdr + 0x36) = 0x38; // e_phentsize
	*(uint16_t *)(hdr + 0x38) = 1; // e_phnum

	*(uint32_t *)(hdr + 0x40) = 1; // p_type = PT_LOAD
	*(uint32_t *)(hdr + 0x44) = 5; // p_flags = PF_R|PF_X
	*(uint64_t *)(hdr + 0x48) = 0; // p_offset = 0
	*(uint64_t *)(hdr + 0x50) = VIRTUAL_OFFSET; // p_vaddr
	*(uint64_t *)(hdr + 0x58) = 0; // p_paddr
	*(uint64_t *)(hdr + 0x60) = ELF3_HDR_SIZE + codelen; // p_filesz
	*(uint64_t *)(hdr + 0x68) = ELF3_HDR_SIZE + codelen; // p_memsz
	*(uint64_t *)(hdr + 0x70) = 0x1000; // p_align

	fwrite(hdr, 1, ELF3_HDR_SIZE, f);
	fwrite(code, 1, codelen, f);
}

static uint8_t code[] = {
	0x48, 0xC7, 0xC0, 0x3C, 0x00, 0x00, 0x00, 0x48, 0xC7, 0xC7, 0x2A, 0x00, 0x00, 0x00, 0x0F, 0x05
};

int main()
{
	write_elf3(stdout, code, sizeof(code), VIRTUAL_OFFSET + ELF3_HDR_SIZE);
}
