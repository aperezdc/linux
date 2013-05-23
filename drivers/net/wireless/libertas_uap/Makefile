obj-$(CONFIG_LIBERTAS_UAP) += uap8xxx.o

uap8xxx-y += uap_main.o if_sdio.o
uap8xxx-$(CONFIG_PROC_FS) += uap_proc.o uap_debug.o

EXTRA_CFLAGS += -DFPNUM='"52"' -DDEBUG_LEVEL1
