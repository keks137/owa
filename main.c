#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define ELF643_HDR_SIZE 0x78 // ELF header + 1 program header
#define VIRTUAL_OFFSET 0x400000

#define ALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))

static void write_elf643(FILE *f, uint8_t *code, size_t codelen, uint64_t entry)
{
	uint8_t hdr[ELF643_HDR_SIZE] = { 0 };

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
	*(uint64_t *)(hdr + 0x18) = VIRTUAL_OFFSET + ELF643_HDR_SIZE + entry; // e_entry
	*(uint64_t *)(hdr + 0x20) = 0x40; // e_phoff
	*(uint16_t *)(hdr + 0x34) = 0x40; // e_ehsize
	*(uint16_t *)(hdr + 0x36) = 0x38; // e_phentsize
	*(uint16_t *)(hdr + 0x38) = 1; // e_phnum

	*(uint32_t *)(hdr + 0x40) = 1; // p_type = PT_LOAD
	*(uint32_t *)(hdr + 0x44) = 5; // p_flags = PF_R|PF_X
	*(uint64_t *)(hdr + 0x48) = 0; // p_offset = 0
	*(uint64_t *)(hdr + 0x50) = VIRTUAL_OFFSET; // p_vaddr
	*(uint64_t *)(hdr + 0x58) = 0; // p_paddr
	*(uint64_t *)(hdr + 0x60) = ELF643_HDR_SIZE + codelen; // p_filesz
	*(uint64_t *)(hdr + 0x68) = ELF643_HDR_SIZE + codelen; // p_memsz
	*(uint64_t *)(hdr + 0x70) = 0x1000; // p_align

	fwrite(hdr, 1, ELF643_HDR_SIZE, f);
	fwrite(code, 1, codelen, f);
}

typedef struct {
	uint8_t *data;
	size_t cap;
	size_t lvl;
} BinArray;

static void binarr_push_byte(BinArray *arr, uint8_t data)
{
	arr->data[arr->lvl] = data;
	arr->lvl++;
}

static void binarr_push(BinArray *arr, const void *data, size_t len)
{
	memcpy(&arr->data[arr->lvl], data, len);
	arr->lvl += len;
}

static void movm64imm32(BinArray *arr, uint8_t reg, uint32_t imm)
{
	const uint8_t rexw_prefix = 0x48;
	binarr_push(arr, &rexw_prefix, sizeof(rexw_prefix));
	const uint8_t opcode = 0xC7;
	binarr_push(arr, &opcode, sizeof(opcode));
	binarr_push(arr, &reg, sizeof(reg));
	binarr_push(arr, &imm, sizeof(imm));
}

static void syscall(BinArray *arr)
{
	binarr_push_byte(arr, 0x0F);
	binarr_push_byte(arr, 0x05);
}

int main()
{
	BinArray code = { 0 };
	{
		static uint8_t data[512];
		code.data = &data[0];
	}

	movm64imm32(&code, 0xC0, 60);
	movm64imm32(&code, 0xC7, 69);
	syscall(&code);

	write_elf643(stdout, code.data, code.lvl, 0);
}
