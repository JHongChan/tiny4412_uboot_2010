#include <common.h>
#include <command.h>
#include <net.h>
#include <asm/setup.h>	//tagµÄ¶¨Òå

DECLARE_GLOBAL_DATA_PTR;

static struct tag *params;
static void setup_start_tag(void)
{
	params = gd->bd->bi_boot_params;
	params->hdr.tag = ATAG_CORE;
	params->hdr.size = tag_size(tag_core);
	params->u.core.flags = 0;
	params->u.core.pagesize = 4096;
	params->u.core.rootdev = 0;
	params = tag_next(params);
}

static void setup_memory_tags(void)
{
	int i;
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		params->hdr.tag = ATAG_MEM;
		params->hdr.size = tag_size (tag_mem32);
		params->u.mem.start = gd->bd->bi_dram[i].start;
		params->u.mem.size = gd->bd->bi_dram[i].size;
		params = tag_next (params);
	}
}

static void setup_commandline_tag(char * comdline)
{
	if (!comdline)
		return;
	while (*comdline == ' ') comdline++;
	if (*comdline == '\0')
		return;
	params->hdr.tag = ATAG_CMDLINE;
	params->hdr.size =
		(sizeof (struct tag_header) + strlen (comdline) + 1 + 3) >> 2;
	strcpy (params->u.cmdline.cmdline, comdline);
	params = tag_next (params);
}

static void setup_end_tag(void)
{
	params->hdr.tag = ATAG_NONE;
	params->hdr.size = 0;
}

int do_bootz(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	if(argc < 2)
		return cmd_usage(cmdtp);
	setup_start_tag();
	setup_memory_tags();
	char *commandline = getenv("bootargs");
	setup_commandline_tag(commandline);
	setup_end_tag();
	cleanup_before_linux();
	void(*kernel_entry)(int zero, int arch, uint params);
	ulong addr;
	addr = simple_strtol(argv[1], NULL, 16);
	kernel_entry = (void(*)(int, int, uint))addr;
	kernel_entry(0, gd->bd->bi_arch_number, gd->bd->bi_boot_params);
	return 0;
}

U_BOOT_CMD(
	bootz,CONFIG_SYS_MAXARGS,1,do_bootz,
	"start zImage at address 'addr'",
	"addr [arg...]\n	-start zImage at address 'addr'\n"
	"		passing 'arg' as arguments"
);

