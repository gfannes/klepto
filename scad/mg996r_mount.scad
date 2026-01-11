//
// MG996R Servo Mount Plate + STEMFIE-compatible hole grid (parametric)
//
// What this provides:
// - Center cutout for MG996R body (servo rests on its mounting flanges/ears)
// - 4 servo ear holes (2 per side), modeled as a rectangular pattern
// - STEMFIE-style mounting grid: holes on a Block Unit (BU) pitch (default 12.5 mm),
//   with “shaft” hole diameter default 7.0 mm (common STEMFIE fit-up guidance). :contentReference[oaicite:0]{index=0}
//
// Notes on dimensions:
// - MG996R overall body dims (40.7 x 19.7) are commonly reported. :contentReference[oaicite:1]{index=1}
// - Exact MG996R ear-hole pattern varies across clones; keep it parametric.
//   Default secondary spacing uses a commonly repeated “10 mm apart” claim. :contentReference[oaicite:2]{index=2}
//
// Save as: mg996r_stemfie_mount.scad
//

$fn = 80;

// ---------------------- STEMFIE parameters ----------------------
BU                = 12.5;   // STEMFIE Block Unit (mm). :contentReference[oaicite:3]{index=3}
stemfie_hole_d    = 7.0;    // STEMFIE-style through hole (mm). :contentReference[oaicite:4]{index=4}
stemfie_plate_th  = BU/4;   // common “thin” STEMFIE thickness convention (mm)

// Grid size in BU (number of holes in X and Y)
grid_nx = 6;  // plate length in holes
grid_ny = 4;  // plate width  in holes

// If true, holes include the outermost grid positions.
// If false, leaves a 1-hole margin (useful if you want a solid rim).
use_outer_ring = true;

// ---------------------- Servo parameters ----------------------
servo_body_len   = 40.7;  // mm :contentReference[oaicite:5]{index=5}
servo_body_w     = 19.7;  // mm :contentReference[oaicite:6]{index=6}

body_clearance   = 0.5;   // clearance on each side of the body cutout (mm)

// 4 ear holes as rectangle: (±span_x/2, ±span_y/2)
//
// span_x: along servo length (front<->back), span_y: across servo width (left<->right).
// Many brackets only need 2 holes; user requested 2 per side => 4 holes.
ear_hole_span_x  = 49.5;  // mm (typical “standard-size” servo mounting length span; adjust as needed)
ear_hole_span_y  = 10.0;  // mm (commonly stated spacing; adjust for your servo) :contentReference[oaicite:7]{index=7}
ear_hole_d       = 3.2;   // mm (M3 clearance)

// If you want the ear holes offset from the plate center (rare), set these:
ear_offset_x = 0.0;
ear_offset_y = 0.0;

// ---------------------- Derived sizes ----------------------
plate_len = (grid_nx - 1) * BU + BU; // overall length, leaves BU/2 margin each side
plate_w   = (grid_ny - 1) * BU + BU;

cutout_len = servo_body_len + 2*body_clearance;
cutout_w   = servo_body_w   + 2*body_clearance;

// ---------------------- Helpers ----------------------
module grid_holes(nx, ny, d, through_h, outer_ring=true) {
  // Places holes with BU spacing, centered on origin.
  // Coordinates are hole centers.
  x0 = -(nx-1)*BU/2;
  y0 = -(ny-1)*BU/2;

  for (ix = [0:nx-1]) for (iy = [0:ny-1]) {
    on_outer = (ix==0 || iy==0 || ix==nx-1 || iy==ny-1);
    if (outer_ring || !on_outer) {
      translate([x0 + ix*BU, y0 + iy*BU, -0.01])
        cylinder(h=through_h + 0.02, d=d);
    }
  }
}

module mg996r_ear_holes(d, through_h) {
  for (sx = [-1, 1]) for (sy = [-1, 1]) {
    translate([ear_offset_x + sx*ear_hole_span_x/2,
               ear_offset_y + sy*ear_hole_span_y/2,
               -0.01])
      cylinder(h=through_h + 0.02, d=d);
  }
}

module mg996r_body_cutout(through_h) {
  translate([-cutout_len/2, -cutout_w/2, -0.01])
    cube([cutout_len, cutout_w, through_h + 0.02]);
}

// ---------------------- Model ----------------------
module mg996r_mount_a(){
  difference() {
    // Plate (centered)
    translate([-plate_len/2, -plate_w/2, 0])
      cube([plate_len, plate_w, stemfie_plate_th]);

    // STEMFIE-compatible grid holes
    grid_holes(grid_nx, grid_ny, stemfie_hole_d, stemfie_plate_th, use_outer_ring);

    // Servo body cutout
    mg996r_body_cutout(stemfie_plate_th);

    // Servo ear holes (4x)
    mg996r_ear_holes(ear_hole_d, stemfie_plate_th);
  }
}
module mg996r_mount_b(){
  difference() {
    // Plate (centered)
    translate([-plate_len/2, -plate_w/2, 0])
      cube([plate_len-0.78*BU, plate_w-BU, stemfie_plate_th]);

    // STEMFIE-compatible grid holes
    grid_holes(grid_nx, grid_ny, stemfie_hole_d, stemfie_plate_th, use_outer_ring);

    // Servo body cutout
    mg996r_body_cutout(stemfie_plate_th);

    // Servo ear holes (4x)
    mg996r_ear_holes(ear_hole_d, stemfie_plate_th);
  }
}
module mg996r_mount_c(){
  difference() {
    // Plate (centered)
    translate([-plate_len/2, -(plate_w-2*BU)/2, 0])
      cube([plate_len, plate_w-2*BU, stemfie_plate_th]);

    // STEMFIE-compatible grid holes
    grid_holes(grid_nx, grid_ny, stemfie_hole_d, stemfie_plate_th, use_outer_ring);

    // Servo body cutout
    mg996r_body_cutout(stemfie_plate_th);

    // Servo ear holes (4x)
    mg996r_ear_holes(ear_hole_d, stemfie_plate_th);
  }
}
