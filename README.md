## แนะนำโครงการ

ศึกษาจากหลักสูตร "ระบบปฏิบัติการ" ของ Fudan University ในภาคเรียนฤดูใบไม้ร่วงปี 2020 สร้างระบบปฏิบัติการอย่างง่ายโดยใช้สถาปัตยกรรม ARM (RASPBERRY PI3B/B+)

- Toolchain
- boot
- ARMv8 exception handling
- ARMv8 MMU Paging
- timer clock interrupt
- ไดรเวอร์ SD CARD
- filesystem
- systemcall
- libc (musl)
- shell

Chaichana Lohasaptawee SUN

### Build musl

Fetch musl by `make init`.

## อ้างอิง
- [Arm® Architecture Reference Manual](https://cs140e.sergio.bz/docs/ARMv8-Reference-Manual.pdf)
- [Arm® Instruction Set Reference Guide](https://ipads.se.sjtu.edu.cn/courses/os/reference/arm_isa.pdf)
- [ARM Cortex-A Series Programmer’s Guide for ARMv8-A](https://cs140e.sergio.bz/docs/ARMv8-A-Programmer-Guide.pdf)
- [ARM GCC Inline Assembler Cookbook](https://www.ic.unicamp.br/~celio/mc404-s2-2015/docs/ARM-GCC-Inline-Assembler-Cookbook.pdf)