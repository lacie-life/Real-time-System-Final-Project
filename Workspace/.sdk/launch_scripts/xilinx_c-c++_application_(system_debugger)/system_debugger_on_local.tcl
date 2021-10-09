connect -url tcp:127.0.0.1:3121
targets -set -filter {jtag_cable_name =~ "Digilent JTAG-SMT1 210203A3D319A" && level==0} -index 0
fpga -file G:/Workspace/system_top_hw_platform_1/download.bit
targets -set -nocase -filter {name =~ "microblaze*#1" && bscan=="USER2"  && jtag_cable_name =~ "Digilent JTAG-SMT1 210203A3D319A"} -index 0
loadhw G:/Workspace/system_top_hw_platform_1/system.hdf
targets -set -nocase -filter {name =~ "microblaze*#1" && bscan=="USER2"  && jtag_cable_name =~ "Digilent JTAG-SMT1 210203A3D319A"} -index 0
rst -processor
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent JTAG-SMT1 210203A3D319A"} -index 0
rst -processor
targets -set -nocase -filter {name =~ "microblaze*#1" && bscan=="USER2"  && jtag_cable_name =~ "Digilent JTAG-SMT1 210203A3D319A"} -index 0
dow G:/Workspace/driver_HDMI/Debug/driver_HDMI.elf
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent JTAG-SMT1 210203A3D319A"} -index 0
dow G:/Workspace/MB1_RTOS/Debug/MB1_RTOS.elf
bpadd -addr &main
