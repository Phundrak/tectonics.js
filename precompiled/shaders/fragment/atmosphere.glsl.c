#define GL_ES
#include "precompiled/cross_platform_macros.glsl.c"
#include "precompiled/academics/units.glsl.c"
#include "precompiled/academics/math/constants.glsl.c"
#include "precompiled/academics/math/geometry.glsl.c"
#include "precompiled/academics/physics/constants.glsl.c"
#include "precompiled/academics/physics/emission.glsl.c"
#include "precompiled/academics/physics/scattering.glsl.c"
#include "precompiled/academics/physics/reflectance.glsl.c"
#include "precompiled/academics/raymarching.glsl.c"
#include "precompiled/academics/psychophysics.glsl.c"
#include "precompiled/academics/electronics.glsl.c"


varying vec2  vUv;
uniform sampler2D background_rgb_signal_texture;


// Determines the length of a unit of distance within the view, in meters, 
// it is generally the radius of whatever world's the focus for the scene.
// The view uses different units for length to prevent certain issues with
// floating point precision. 

// VIEW PROPERTIES -----------------------------------------------------------
uniform mat4  projection_matrix_inverse;
uniform mat4  view_matrix_inverse;
uniform float reference_distance;
uniform float shaderpass_visibility;

// WORLD PROPERTIES ------------------------------------------------------------
uniform vec3  world_position;
uniform float world_radius;

// LIGHT SOURCE PROPERTIES -----------------------------------------------------
uniform vec3  light_rgb_intensities [MAX_LIGHT_COUNT];
uniform vec3  light_directions      [MAX_LIGHT_COUNT];
uniform int   light_count;
uniform float insolation_max;

// ATMOSPHERE PROPERTIES -------------------------------------------------------
uniform float atmosphere_scale_height;
uniform vec3  surface_air_rayleigh_scattering_coefficients; 
uniform vec3  surface_air_mie_scattering_coefficients; 
uniform vec3  surface_air_absorption_coefficients; 


bool isnan(float x)
{
    return !(0. <= x || x <= 0.);
}
bool isbig(float x)
{
    return abs(x)>BIG;
}

void main() {
    vec2  screenspace   = vUv;
    vec2  clipspace     = 2.0 * screenspace - 1.0;

    // "V0" is view origin
    vec3  V0 = view_matrix_inverse[3].xyz * reference_distance;
    // "V" is view direction
    vec3  V  = normalize(view_matrix_inverse * projection_matrix_inverse * vec4(clipspace, 1, 1)).xyz;
    // "O" is world origin
    vec3  O  = vec3(0);
    // "r" is world radius
    float r  = world_radius;
    // "H" is atmospheric scale height
    float H  = atmosphere_scale_height;
    // "I_back" is background light intensity
    vec3  I_back = insolation_max * get_rgb_intensity_of_rgb_signal(
        texture2D( background_rgb_signal_texture, vUv ).rgb
    );
    // "beta_air_*" variables are the scattering coefficients for the atmosphere at sea level
    vec3  beta_ray = surface_air_rayleigh_scattering_coefficients;
    vec3  beta_mie = surface_air_mie_scattering_coefficients;
    vec3  beta_abs = surface_air_absorption_coefficients; 

    // check for intersection with the atmosphere
    // We only set it to 12 scale heights because we are using this parameter for raymarching, and not a closed form solution
    maybe_vec2 v_atmosphere_region = get_distances_along_3d_line_to_sphere(V0, V, O, r + 12.*H);  
    maybe_vec2 v_obstructed_region = get_distances_along_3d_line_to_sphere(V0, V, O, r);
    maybe_vec2 v_scatter_region    = get_distances_along_line_to_negation(v_atmosphere_region, v_obstructed_region);

    // "E" is the rgb intensity of light emitted towards the camera
    vec3 E = vec3(0);
    // if view ray does not interact with the atmosphere
    // don't bother running the raymarch algorithm
    if (v_scatter_region.exists)
    { 
        // start of march along view ray
        float v0  = max(v_scatter_region.value.x, 0.0);
        // end of march along view ray
        float v1  = max(v_scatter_region.value.y, 0.0);

        for (int i = 0; i < MAX_LIGHT_COUNT; ++i)
        {
            if (i >= light_count) { break; }
            E += get_rgb_intensity_of_light_scattered_from_air_for_curved_world(
                V0, V, v0, v1, O, r,
                light_directions[i], light_rgb_intensities[i],
                H, beta_ray, beta_mie, beta_abs
            );
        }
        // now calculate the intensity of light that traveled straight in from the background, and add it to the total
        E += I_back * get_rgb_fraction_of_light_transmitted_through_air_for_curved_world(
            V0, V, v1*0.999, O, r, H, beta_ray, beta_mie, beta_abs
        );
    }
    else 
    {
        E = I_back;
    }

    // TODO: move this to a separate shader pass!
    // see https://learnopengl.com/Advanced-Lighting/HDR for an intro to tone mapping
    float exposure_intensity = 150.; // Watts/m^2
    vec3  ldr_tone_map = 1.0 - exp(-E/exposure_intensity);

    gl_FragColor = vec4(get_rgb_signal_of_rgb_intensity(ldr_tone_map), 1);
}