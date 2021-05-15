// move & rotate
translate_x=-52;
translate_y=-5;
rotate_z=0;
// space for motherboard
space_for_motherboard=true;

rotate(a=[180,0,0])
union()
{
    difference()
    {
        rotate(a=[0,0,rotate_z])
        translate([translate_x,translate_y,0])
        // plate with screw holes
        difference()
        {
            translate([-5,-5,0])
                cube([148,84,5],center=false);
            union()
            {
                if (space_for_motherboard)
                {
                    translate([-6,17,-3])
                        cube([4,63,7],center=false);
                }
                // atx psu screw holes
                cylinder(h=12, r=1.5, $fn=12, center=true);
                translate([114,0,0])
                    cylinder(h=12, r=1.5, $fn=12, center=true);
                translate([138,0,0])
                    cylinder(h=12, r=1.5, $fn=12, center=true);
                translate([0,10,0])
                    cylinder(h=12, r=1.5, $fn=12, center=true);
                translate([138,64,0])
                    cylinder(h=12, r=1.5, $fn=12, center=true);
                translate([0,74,0])
                    cylinder(h=12, r=1.5, $fn=12, center=true);
                translate([24,74,0])
                    cylinder(h=12, r=1.5, $fn=12, center=true);
                translate([138,74,0])
                    cylinder(h=12, r=1.5, $fn=12, center=true);
            }
        }
        
        translate([0,0,-1])
        difference()
        {
            union()
            {
                translate([3,0,0])
                    cube([43,41,7],center=false);
                cube([87,39.2,7],center=false);
            }
            /*
            union()
            {
                translate([82,0,0])
                    cube([5,5,7],center=false);
                translate([82,35,0])
                    cube([5,5,7],center=false);
            }
            */
        }
    }
    
    union()
    {
        translate([82,0,-1.5])
            cube([5,5,6.5],center=false);
        translate([82,35,-1.5])
            cube([5,5,6.5],center=false);
    }
            
    translate([-2,-2,-31])
        difference()
        {
            cube([90.5,45,36],center=false);
            union()
            {
                translate([5,2,-1])
                    cube([43,41,38],center=false);
                translate([2,2,-1])
                    cube([87,39.2,38],center=false);
                translate([51,42.8,-1])
                    cube([41,4,38],center=false);
                // slots
                translate([88.5,13,13])
                    cube([6,2,10],center=false);
                translate([87.5,27.5,13])
                    cube([6,2,10],center=false);
            }
        }
}