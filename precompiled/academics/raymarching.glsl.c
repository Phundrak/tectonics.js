CONST(float) BIG = 1e20;
CONST(float) SMALL = 1e-20;
CONST(int)   MAX_LIGHT_COUNT = 9;

struct maybe_vec2
{
    vec2 value; 
    bool  exists; 
};
struct maybe_float
{
    float value;  
    bool  exists; 
};

maybe_float get_distance_along_3d_line_to_plane(
    in vec3 A0,
    in vec3 A,
    in vec3 B0,
    in vec3 N
){
    return maybe_float( -dot(A0 - B0, N) / dot(A, N), abs(dot(A, N)) < SMALL);
}

maybe_vec2 get_distances_along_3d_line_to_sphere(
    in vec3 A0,
    in vec3 A,
    in vec3 B0,
    in float r
){
    float xz = dot(B0 - A0, A);
    float z = length(A0 + A * xz - B0);
    float y2 = r * r - z * z;
    float dxr = sqrt(max(y2, 1e-10));
    return maybe_vec2(
        vec2(xz - dxr, xz + dxr), 
        y2 > 0.
    );
}


maybe_vec2 get_distances_along_line_to_negation(
    in maybe_vec2 positive,
    in maybe_vec2 negative
) {
    // as long as intersection with positive exists, 
    // and negative doesn't completely surround it, there will be an intersection
    bool exists = 
        positive.exists && !(negative.value.x < positive.value.x && positive.value.y < negative.value.y);
    // find the first region of intersection
    float entrance = !negative.exists ? positive.value.x : min(negative.value.y, positive.value.x);
    float exit     = !negative.exists ? positive.value.y : min(negative.value.x, positive.value.y);
    // if the first region is behind us, find the second region
    if (exit < 0. && 0. < positive.value.y)
    {
        entrance = negative.value.y;
        exit     = positive.value.y;
    }
    return maybe_vec2( vec2(entrance, exit), exists );
}

// "oplus" is the o-plus operator,
//   or the reciprocal of the sum of reciprocals.
// It's a handy function that comes up a lot in some physics problems.
// It's pretty useful for preventing division by zero.
FUNC(float) oplus(IN(float) a, IN(float) b){
    return 1. / (1./a + 1./b);
}

// "approx_air_column_density_ratio_along_2d_ray_for_curved_world" 
//   calculates the distance you would need to travel 
//   along the surface to encounter the same number of particles in the column. 
// It does this by finding an integral using integration by substitution, 
//   then tweaking that integral to prevent division by 0. 
// All distances are recorded in scale heights.
// "a" and "b" are distances along the ray from closest approach.
//   The ray is fired in the positive direction.
//   If there is no intersection with the planet, 
//   a and b are distances from the closest approach to the upper bound.
// "z2" is the closest distance from the ray to the center of the world, squared.
// "r0" is the radius of the world.
FUNC(float) approx_air_column_density_ratio_along_2d_ray_for_curved_world(
    IN(float) a, 
    IN(float) b, 
    IN(float) z2, 
    IN(float) R
){
    // GUIDE TO VARIABLE NAMES:
    //  capital letters indicate surface values, e.g. "R" is planet radius
    //  "x*" distance along the ray from closest approach
    //  "z*" distance from the center of the world at closest approach
    //  "R*" distance ("radius") from the center of the world
    //  "h*" distance ("height") from the center of the world
    //  "*0" variable at reference point
    //  "*1" variable at which the top of the atmosphere occurs
    //  "*2" the square of a variable
    //  "d*dx" a derivative, a rate of change over distance along the ray
    float X  = sqrt(max(R*R -z2, 0.));
    float div0_fix = 1./sqrt((X*X+R) * 0.5*PI);
    float ra = sqrt(a*a+z2);
    float rb = sqrt(b*b+z2);
    float sa = 1./(abs(a)/ra + div0_fix) *     exp(R-ra);
    float sb = 1./(abs(b)/rb + div0_fix) *     exp(R-rb);
    float S  = 1./(abs(X)/R  + div0_fix) * min(exp(R-sqrt(z2)),1.);
    return sign(b)*(S-sb) - sign(a)*(S-sa);
}

// "approx_air_column_density_ratio_along_2d_ray_for_curved_world" 
//   calculates column density ratio of air for a ray emitted from the surface of a world to a desired distance, 
//   taking into account the curvature of the world.
// It does this by making a quadratic approximation for the height above the surface.
// The derivative of this approximation never reaches 0, and this allows us to find a closed form solution 
//   for the column density ratio using integration by substitution.
// "x_start" and "x_stop" are distances along the ray from closest approach.
//   If there is no intersection, they are the distances from the closest approach to the upper bound.
//   Negative numbers indicate the rays are firing towards the ground.
// "z2" is the closest distance from the ray to the center of the world, squared.
// "r" is the radius of the world.
// "H" is the scale height of the atmosphere.
FUNC(float) approx_air_column_density_ratio_along_2d_ray_for_curved_world(
    IN(float) x_start, 
    IN(float) x_stop, 
    IN(float) z2, 
    IN(float) r, 
    IN(float) H
){
    float X = sqrt(max(r*r -z2, 0.));
    // if ray is obstructed
    if (x_start < X && -X < x_stop && z2 < r*r)
    {
        // return ludicrously big number to represent obstruction
        return BIG;
    }
    float sigma = 
        H * approx_air_column_density_ratio_along_2d_ray_for_curved_world(
            x_start / H,
            x_stop  / H,
            z2      /(H*H),
            r       / H
        );
    // NOTE: we clamp the result to prevent the generation of inifinities and nans, 
    // which can cause graphical artifacts.
    return min(abs(sigma),BIG);
}

// "try_approx_air_column_density_ratio_along_ray" is an all-in-one convenience wrapper 
//   for approx_air_column_density_ratio_along_ray_2d() and approx_reference_air_column_density_ratio_along_ray.
// Just pass it the origin and direction of a 3d ray and it will find the column density ratio along its path, 
//   or return false to indicate the ray passes through the surface of the world.
FUNC(float) approx_air_column_density_ratio_along_3d_ray_for_curved_world (
    IN(vec3)  P, 
    IN(vec3)  V,
    IN(float) x,
    IN(float) r, 
    IN(float) H
){
    float xz = dot(-P,V);           // distance ("radius") from the ray to the center of the world at closest approach, squared
    float z2 = dot( P,P) - xz * xz; // distance from the origin at which closest approach occurs
    return approx_air_column_density_ratio_along_2d_ray_for_curved_world( 0.-xz, x-xz, z2, r, H );
}

FUNC(vec3) get_rgb_fraction_of_light_transmitted_through_air_for_curved_world(
    IN(vec3)  segment_origin, IN(vec3)  segment_direction, IN(float) segment_length,
    IN(vec3)  world_position, IN(float) world_radius,      IN(float) atmosphere_scale_height,
    IN(vec3)  beta_ray,       IN(vec3)  beta_mie,          IN(vec3)  beta_abs
){
    vec3  O = world_position;
    float r = world_radius;
    float H = atmosphere_scale_height;
    // "sigma" is the column density of air, relative to the surface of the world, that's along the light's path of travel,
    //   we use it to estimate the amount of light that's filtered by the atmosphere before reaching the surface
    //   see https://www.alanzucconi.com/2017/10/10/atmospheric-scattering-1/ for an awesome introduction
    float sigma  = approx_air_column_density_ratio_along_3d_ray_for_curved_world (segment_origin-world_position, segment_direction, segment_length, r, H);
    // "I_surface" is the intensity of light that reaches the surface after being filtered by atmosphere
    return exp(-sigma * (beta_ray + beta_mie + beta_abs));
}

// TODO: multiple scattering events
// TODO: support for light sources from within atmosphere
FUNC(vec3) get_rgb_intensity_of_light_scattered_from_air_for_curved_world(
    IN(vec3)  view_origin,     IN(vec3) view_direction, IN(float) view_start_length, IN(float) view_stop_length,
    IN(vec3)  world_position,  IN(float) world_radius,
    IN(vec3)  light_direction, IN(vec3)  light_rgb_intensity,
    IN(float) atmosphere_scale_height,
    IN(vec3) beta_ray, IN(vec3) beta_mie, IN(vec3)  beta_abs
){
    // For an excellent introduction to what we're try to do here, see Alan Zucconi: 
    //   https://www.alanzucconi.com/2017/10/10/atmospheric-scattering-3/
    // We will be using most of the same terminology and variable names.

    // GUIDE TO VARIABLE NAMES:
    //  Uppercase letters indicate vectors.
    //  Lowercase letters indicate scalars.
    //  Going for terseness because I tried longhand names and trust me, you can't read them.
    //  "z*"     distance from the center of the world to closest approach
    //  "r*"     a distance ("radius") from the center of the world
    //  "h*"     a distance ("height") from the surface of the world
    //  "*v*"    property of the view ray, the ray cast from the viewer to the object being viewed
    //  "*l*"    property of the light ray, the ray cast from the object to the light source
    //  "*2"     the square of a variable
    //  "*i"    property of an iteration within the raymarch
    //  "beta*"  a scattering coefficient, the number of e-foldings in light intensity per unit distance
    //  "gamma*" a phase factor, the fraction of light that's scattered in a certain direction
    //  "rho*"   a density ratio, the density of air relative to surface density
    //  "sigma*" a column density ratio, the density of a column of air relative to surface density
    //  "I*"     intensity of source lighting for each color channel
    //  "E*"     intensity of light cast towards the viewer for each color channel
    //  "*_ray"  property of rayleigh scattering
    //  "*_mie"  property of mie scattering
    //  "*_abs"  property of absorption

    vec3  V0= view_origin;
    vec3  V = view_direction;
    float v0= view_start_length;
    float v1= view_stop_length;
    vec3  P = view_origin - world_position;
    vec3  L = light_direction;     // unit vector pointing to light source
    vec3  I = light_rgb_intensity; // vector indicating intensity of light source for each color channel
    vec3  O = world_position;
    float r = world_radius;
    float H = atmosphere_scale_height;


    float v  = dot(-P,V);         // distance from view ray origin to closest approach
    float z2 = dot( P,P) - v * v; // squared distance from the view ray to the center of the world at closest approach

    // cosine of angle between view and light directions
    float VL; 

    // "gamma_*" indicates the fraction of scattered sunlight that scatters to a given angle (indicated by its cosine, A.K.A. "VL").
    // It only accounts for a portion of the sunlight that's lost during the scatter, which is irrespective of wavelength or density
    float gamma_ray;
    float gamma_mie;

    // "beta_*" indicates the rest of the fractional loss.
    // it is dependant on wavelength, and the density ratio, which is dependant on height
    // So all together, the fraction of sunlight that scatters to a given angle is: beta(wavelength) * gamma(angle) * density_ratio(height)
    vec3  beta_sum   = beta_ray + beta_mie + beta_abs;
    vec3  beta_gamma;
    
    const float STEP_COUNT = 16.; // number of steps taken while marching along the view ray
    float dx = (v1 - v0) / STEP_COUNT;
    float vi = v0 - v + 0.5 * dx;

    float l;           // distance from light ray origin to closest approach
    float zl2;         // squared distance ("radius") of the light ray at closest for a single iteration of the view ray march
    float r2;          // squared distance ("radius") from the center of the world for a single iteration of the view ray march
    float h;           // distance ("height") from the surface of the world for a single iteration of the view ray march
    float sigma_v;     // columnar density encountered along the view ray,  relative to surface density
    float sigma_l;     // columnar density encountered along the light ray, relative to surface density
    vec3  E = vec3(0); // total intensity for each color channel, found as the sum of light intensities for each path from the light source to the camera

    for (float i = 0.; i < STEP_COUNT; ++i)
    {
        r2  = vi*vi+z2;
        h   = sqrt(r2) - r;
        sigma_v = approx_air_column_density_ratio_along_2d_ray_for_curved_world(-v, vi, z2, r, H );

        VL  = dot(V, -L);
        l   = dot(P+V*(vi+v),-L);
        zl2 = r2 - l*l; 
        sigma_l    = approx_air_column_density_ratio_along_2d_ray_for_curved_world(-l, 3.*r, zl2, r, H );
        gamma_ray  = get_fraction_of_rayleigh_scattered_light_scattered_by_angle(VL);
        gamma_mie  = get_fraction_of_mie_scattered_light_scattered_by_angle(VL);
        beta_gamma = beta_ray * gamma_ray + beta_mie * gamma_mie;

        E += I
            // incoming fraction: the fraction of light that scatters towards camera
            * exp(-h/H) * beta_gamma * dx
            // outgoing fraction: the fraction of light that scatters away from camera
            * exp(-beta_sum * (sigma_l + sigma_v));

        vi  += dx;
    }

    return E;
}


FUNC(vec3) get_rgb_intensity_of_light_scattered_from_fluid_for_flat_world(
    IN(float) cos_view_angle, 
    IN(float) cos_light_angle, 
    IN(float) cos_scatter_angle, 
    IN(float) ocean_depth,
    IN(vec3)  refracted_light_rgb_intensity,
    IN(vec3)  beta_ray,       IN(vec3)  beta_mie,          IN(vec3)  beta_abs
){
    float NV = cos_view_angle;
    float NL = cos_light_angle;
    float LV = cos_scatter_angle;

    vec3 I = refracted_light_rgb_intensity;

    // "gamma_*" variables indicate the fraction of scattered sunlight that scatters to a given angle (indicated by its cosine).
    // it is also known as the "phase factor"
    // It varies
    // see mention of "gamma" by Alan Zucconi: https://www.alanzucconi.com/2017/10/10/atmospheric-scattering-3/
    float gamma_ray = get_fraction_of_rayleigh_scattered_light_scattered_by_angle(LV);
    float gamma_mie = get_fraction_of_mie_scattered_light_scattered_by_angle(LV);

    vec3  beta_gamma = beta_ray * gamma_ray + beta_mie * gamma_mie;
    vec3  beta_sum   = beta_ray + beta_mie + beta_abs;

    // "sigma_v"  is the column density, relative to the surface, that's along the view ray.
    // "sigma_l" is the column density, relative to the surface, that's along the light ray.
    // "sigma_ratio" is the column density ratio of the full path of light relative to the distance along the incoming path
    // Since water is treated as incompressible, the density remains constant, 
    //   so they are effectively the distances traveled along their respective paths.
    // TODO: model vector of refracted light within ocean
    float sigma_v  = ocean_depth / NV;
    float sigma_l = ocean_depth / NL;
    float sigma_ratio = 1. + NV/NL;

    return I 
        // incoming fraction: the fraction of light that scatters towards camera
        *     beta_gamma
        // outgoing fraction: the fraction of light that scatters away from camera
        * (exp(-sigma_v * sigma_ratio * beta_sum) - 1.)
        /               (-sigma_ratio * beta_sum);
}

FUNC(vec3) get_rgb_fraction_of_light_transmitted_through_fluid_for_flat_world(
    IN(float) cos_incident_angle, IN(float) ocean_depth,
    IN(vec3)  beta_ray,           IN(vec3)  beta_mie,          IN(vec3)  beta_abs
){
    float sigma  = ocean_depth / cos_incident_angle;
    return exp(-sigma * (beta_ray + beta_mie + beta_abs));
}