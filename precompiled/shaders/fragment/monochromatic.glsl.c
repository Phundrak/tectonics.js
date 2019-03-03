
varying float displacement_v;
varying float plant_coverage_v;
varying float ice_coverage_v;
varying float scalar_v;
varying vec4 position_v;

uniform float sealevel;
uniform float ocean_visibility;

void main() {
	vec4 color_without_ocean = mix( 
		vec4(@MINCOLOR,1.), 
		vec4(@MAXCOLOR,1.), 
		scalar_v
	);
	vec4 color_with_ocean = displacement_v < sealevel * ocean_visibility? mix(vec4(0.), color_without_ocean, 0.5) : color_without_ocean;
	gl_FragColor = color_with_ocean;
}