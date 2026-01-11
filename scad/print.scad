use <stemfie.scad>

beam_block(size=[10, 6, 1]);
translate([-50, -50, 0])
  beam_cross([11, 5]);

use <mg996r_mount.scad>

translate([0,70,0])
	mg996r_mount_a();
translate([0,130,0])
	mg996r_mount_b();
translate([0,170,0])
	mg996r_mount_c();
