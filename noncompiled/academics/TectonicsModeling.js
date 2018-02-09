
// "TectonicsModeling" is the distillation of all raster-based geological sub models within tectonics.js
// All functions are global and stateless. 
// Only the raster data structures and namespaces are used. 
// No other data structure is assumed to exist.
// The idea is to create a core that's invariant to changes across application architecture.

var TectonicsModeling = {};

TectonicsModeling.get_thickness = function(sima, sial, sediment, thickness) {
	ScalarField.add_field(sial, sediment, thickness);
	return ScalarField.add_field(thickness, sima, thickness);
}

TectonicsModeling.get_density = function(sima, sial, sediment, age, density) {
	density = density || Float32Raster(sima.grid);

	// NOTE: density does double duty for performance reasons
	var fraction_of_lifetime = density;
	var sima_density = density;

	Float32RasterInterpolation.smoothstep	(0, 250, age, 						fraction_of_lifetime);
	Float32RasterInterpolation.lerp			(2890, 3300, fraction_of_lifetime, 	density);

	var thickness_i = 0.0
    for (var i = 0, li = density.length; i < li; i++) {
    	thickness_i = sima[i] + sial[i] + sediment[i];
    	density[i] = thickness_i > 0? (sima[i] * sima_density[i] + sial[i] * 2700 + sediment[i] * 2700) / thickness_i : 2890;
    }
    return density;
}

TectonicsModeling.get_subductability = function(density, subductability) {
	var subductability_transition_factor = 1/100;
	var exp = Math.exp;
	for (var i=0, li=subductability.length; i<li; ++i) {
		subductability[i] = 2 / (1 + exp( -(density[i] - 3000) * subductability_transition_factor )) - 1;
	}
	return subductability;
}

// gets surface pressure of the asthenosphere by smoothing a field representing subductability
TectonicsModeling.get_asthenosphere_pressure = function(subductability, pressure, scratch) {
	pressure = pressure || Float32Raster(subductability.grid);
	scratch = scratch || Float32Raster(subductability.grid);

	var diffuse = ScalarField.diffusion_by_constant;

	var smoothing_iterations =  15;
	Float32Raster.copy(subductability, pressure);
	for (var i=0; i<smoothing_iterations; ++i) {
		diffuse(pressure, 1, pressure, scratch);
	}
	return pressure;
}

// gets surface velocity of the asthenosphere as the gradient of pressure
TectonicsModeling.get_asthenosphere_velocity = function(pressure, velocity) {
	velocity = velocity || VectorRaster(subductability.grid);
	ScalarField.gradient(pressure, velocity);
	return velocity;
}

// gets angular velocity of the asthenosphere as the cross product of velocity and position
TectonicsModeling.get_angular_velocity = function(velocity, pos, angular_velocity) {
	return VectorField.cross_vector_field(velocity, pos, angular_velocity);
}
// gets displacement using an isostatic model
TectonicsModeling.get_displacement = function(thickness, density, mantleDensity, displacement) {
 	var thickness_i, rootDepth;
 	var inverse_mantle_density = 1 / mantleDensity;
 	for(var i=0, li = displacement.length; i<li; i++){
 		//Calculates elevation as a function of crust density. 
 		//This was chosen as it only requires knowledge of crust density and thickness,  
 		//which are relatively well known. 
 		thickness_i = thickness[i]; 
 		displacement[i] = thickness_i - thickness_i * density[i] * inverse_mantle_density;
 	}
 	return displacement;
}
TectonicsModeling.get_erosion = function(
		displacement, sealevel, timestep,
		sediment, 		sial, 		sima, 
		sediment_delta, sial_delta, sima_delta, 
		scratch){
	scratch = scratch || Float32Raster(displacement.grid);

	var precipitation = 7.8e5;
	// ^^^ measured in meters of rain per million years
	// global land average from wikipedia
	var erosiveFactor = 1.8e-7; 
	// ^^^ the rate of erosion per the rate of rainfall in that place
	// measured in fraction of height difference per meters of rain per million years

	var sediment_density = 2500;
	var sial_density = 2700;
	var sima_density = 2900;

	// NOTE: erosion array does double duty for performance reasons
	var water_height = scratch;
	ScalarField.max_scalar(displacement, sealevel, water_height);


	var arrows = displacement.grid.arrows;
	var arrow;
	var from = 0;
	var to = 0;
	var height_difference = 0.0;
	var outbound_height_transfer_i = 0.0;

	var outbound_height_transfer = Float32Raster(displacement.grid);
	Float32Raster.fill(outbound_height_transfer, 0);

	var sediment_thickness = 0;
	var sial_thickness = 0;
	var sima_thickness = 0;

	var outbound_sediment_thickness_i;
	var outbound_sial_thickness_i;
	var outbound_sima_thickness_i;

	var outbound_sediment_fraction = Float32Raster(displacement.grid);
	var outbound_sial_fraction = Float32Raster(displacement.grid);
	var outbound_sima_fraction = Float32Raster(displacement.grid);

	// arrow_height_difference = ScalarField.arrow_differential(water_height); 
    // outbound_height_transfer = VectorRaster.OfLength(arrow_height_difference.length, undefined)
    
    // step 1: find outbound height transferred from each cell
	for (var i=0, li=arrows.length; i<li; ++i) {
	    arrow = arrows[i];
	    from = arrow[0];
	    to = arrow[1];
	    height_difference = water_height[from] - water_height[to];
	    outbound_height_transfer[from] += height_difference > 0? height_difference * erosiveFactor * precipitation * timestep : 0;
	}
	// step 2: figure out which pools (sediment/sial/sima) to draw from when we transfer outbound height
	// we prefer to draw from pools that lie on top, e.g. we draw from sediment first, sial second, sima third
	for (var i=0, li=outbound_height_transfer.length; i<li; ++i) {
		sediment_thickness = sediment[i]//     / sediment_density;
		sial_thickness = sial[i]//     / sial_density;
		sima_thickness = sima[i]//     / sima_density;

		outbound_height_transfer_i = outbound_height_transfer[i];
		outbound_sediment_thickness_i = outbound_height_transfer_i > sediment_thickness? sediment_thickness : outbound_height_transfer_i;
		outbound_height_transfer_i -= outbound_sediment_thickness_i;
		outbound_sial_thickness_i = outbound_height_transfer_i > sial_thickness? sial_thickness : outbound_height_transfer_i;
		outbound_height_transfer_i -= outbound_sial_thickness_i;
		outbound_sima_thickness_i = outbound_height_transfer_i > sima_thickness? sima_thickness : outbound_height_transfer_i;
		//TODO: assert outbound_height_transfer_i == 0

		outbound_height_transfer_i = outbound_height_transfer[i];
		outbound_sediment_fraction[i] = outbound_sediment_thickness_i / outbound_height_transfer_i;
		outbound_sial_fraction[i] = outbound_sial_thickness_i / outbound_height_transfer_i;
		outbound_sima_fraction[i] = outbound_sima_thickness_i / outbound_height_transfer_i;
	}

	Float32Raster.fill(sediment_delta, 0);
	Float32Raster.fill(sima_delta, 0);
	Float32Raster.fill(sial_delta, 0);

	var transfer = 0.0;

	// step 3: 
	for (var i=0, li=arrows.length; i<li; ++i) {
	    arrow = arrows[i];
	    from = arrow[0];
	    to = arrow[1];
	    height_difference = water_height[from] - water_height[to];
	    outbound_height_transfer_i = height_difference > 0? height_difference * erosiveFactor * precipitation * timestep : 0;

	    if (!isNaN(outbound_sediment_fraction[from])) { 
	    	transfer = outbound_height_transfer_i * outbound_sediment_fraction[from];
		    sediment_delta[from] -= transfer;
		    sediment_delta[to] += transfer;
	    }

	    if (!isNaN(outbound_sial_fraction[from])) { 
	    	transfer = outbound_height_transfer_i * outbound_sial_fraction[from];
		    sial_delta[from] -= transfer;
		    sial_delta[to] += transfer; // NOTE: sial converts to sediment upon transfer
	    }

	    if (!isNaN(outbound_sima_fraction[from])) { 
	    	transfer = outbound_height_transfer_i * outbound_sima_fraction[from];
		    sima_delta[from] -= transfer;
		    sima_delta[to] += transfer;
	    }
	}
}
TectonicsModeling.get_erosion = function(
		displacement, sealevel, timestep,
		sediment, 		sial, 		sima, 
		sediment_delta, sial_delta, sima_delta, 
		scratch){
	sial_delta = sial_delta || Float32Raster(displacement.grid);
	scratch = scratch || Float32Raster(displacement.grid);

	var precipitation = 7.8e5;
	// ^^^ measured in meters of rain per million years
	// global land average from wikipedia
	var erosiveFactor = 1.8e-7; 
	// ^^^ the rate of erosion per the rate of rainfall in that place
	// measured in fraction of height difference per meters of rain per million years
	var sial_density = 2700;

	// NOTE: erosion array does double duty for performance reasons
	var height_difference = sial_delta;
	var water_height = scratch;
	ScalarField.max_scalar(displacement, sealevel, water_height);
	ScalarField.average_difference(water_height, height_difference);
	ScalarField.mult_scalar(height_difference, precipitation * timestep * erosiveFactor, sial_delta)
	// console.log(Float32Dataset.average(erosion));
	return sial_delta;
	
}
// "weathering" is the process by which rock is converted to sediment
TectonicsModeling.get_weathering_rate = function(sial, sediment, displacement, sealevel, result, scratch){

	return weathering;
}
TectonicsModeling.get_erosion = function(
		displacement, sealevel, timestep,
		sediment, 		sial, 		sima, 
		sediment_delta, sial_delta, sima_delta, 
		scratch){
	sial_delta = sial_delta || Float32Raster(displacement.grid);
	scratch = scratch || Float32Raster(displacement.grid);

	var precipitation = 7.8e5;
	// ^^^ measured in meters of rain per million years
	// global land average from wikipedia
	var erosiveFactor = 1.8e-7; 
	// ^^^ the rate of erosion per the rate of rainfall in that place
	// measured in fraction of height difference per meters of rain per million years
	var sial_density = 2700;

	var water_height = scratch;
	ScalarField.max_scalar(displacement, sealevel, water_height);

	var outbound_height_transfer = Float32Raster(displacement.grid);
	Float32Raster.fill(outbound_height_transfer, 0);

	var outbound_sediment_fraction = Float32Raster(displacement.grid);
	var outbound_sial_fraction = Float32Raster(displacement.grid);
	var outbound_sima_fraction = Float32Raster(displacement.grid);

	var arrows = displacement.grid.arrows;
	var arrow;
	var from = 0;
	var to = 0;
	var height_difference = 0.0;
	var outbound_height_transfer_i = 0.0;
  	var neighbor_count = displacement.grid.neighbor_count;

	for (var i=0, li=arrows.length; i<li; ++i) {
	    arrow = arrows[i];
	    from = arrow[0];
	    to = arrow[1];
	    height_difference = water_height[from] - water_height[to];
	    outbound_height_transfer[from] += height_difference > 0? height_difference * precipitation * timestep * erosiveFactor / neighbor_count[from] : 0;
	}
	for (var i=0, li=outbound_height_transfer.length; i<li; ++i) {
		outbound_height_transfer_i = outbound_height_transfer[i];
		outbound_sediment_fraction[i] = outbound_height_transfer_i > sediment[i]? (sediment[i] > 0.0? sediment[i] / outbound_height_transfer_i : 0.0) : 1.0;
		outbound_height_transfer_i *= 1.0 - outbound_sediment_fraction[i];
		outbound_sial_fraction[i] = outbound_height_transfer_i > sial[i]? (sial[i] > 0.0? sial[i] / outbound_height_transfer_i : 0.0) : 1.0;
	}
	for (var i=0, li=arrows.length; i<li; ++i) {
	    arrow = arrows[i];
	    from = arrow[0];
	    to = arrow[1];
	    height_difference = water_height[from] - water_height[to];
	    outbound_height_transfer_i = height_difference > 0? height_difference * precipitation * timestep * erosiveFactor / neighbor_count[from] : 0;

	    sediment_delta[from] -= outbound_height_transfer_i * outbound_sediment_fraction[from];
	    sediment_delta[to] += outbound_height_transfer_i * outbound_sediment_fraction[from];

	    sial_delta[from] -= outbound_height_transfer_i * outbound_sial_fraction[from];
	    sial_delta[to] += outbound_height_transfer_i * outbound_sial_fraction[from];
	}




	var weathering_factor = 1e-12; 
	// ^^^ the rate of weathering per the rate of rainfall in that place
	// measured in fraction of height difference per meters of rain per million years
	var critical_sediment_thickness = 1;
	// ^^^ the sediment thickness (in meters) at which bedrock weathering no longer occurs

	var earth_surface_gravity = 9.8; // m/s^2
	var surface_gravity = 9.8; // m/s^2
	
	// NOTE: result array does double duty for performance reasons
	weathering = Float32Raster(displacement.grid);
	height_difference = Float32Raster(displacement.grid);
	ScalarField.average_difference(water_height, height_difference);

	ScalarField.mult_scalar(
		height_difference, 
		weathering_factor * 			// apply weathering factor to get height change per unit precip 
		precipitation * 				// apply precip to get height change
		timestep *
		// sial_density * 					// apply density to get mass converted to sediment
		surface_gravity/earth_surface_gravity, //correct for planet's gravity
		weathering)
	
	var bedrock_exposure = Float32Raster(displacement.grid);
	ScalarField.div_scalar(sediment, 
		-critical_sediment_thickness
		// * sediment_density
		, bedrock_exposure);
	ScalarField.add_scalar(bedrock_exposure, 1, bedrock_exposure);
	ScalarField.max_scalar(bedrock_exposure, 0, bedrock_exposure);

	ScalarField.mult_field(weathering, bedrock_exposure, weathering);
	
	ScalarField.min_field(weathering, sial, weathering);
	ScalarField.max_scalar(weathering, 0, weathering);

	ScalarField.sub_field(sial_delta, weathering, sial_delta);
	ScalarField.add_field(sediment_delta, weathering, sediment_delta);
}
// get a map of plates using image segmentation and binary morphology
TectonicsModeling.get_plate_map = function(vector_field, segment_num, min_segment_size, segments) {
  var segments = segments || Uint8Raster(vector_field.grid);
  
  // step 1: run image segmentation algorithm
  VectorImageAnalysis.image_segmentation(vector_field, segment_num, min_segment_size, segments);

  var equals      = Uint8Field.eq_scalar;
  var not_equals  = Uint8Field.ne_scalar;
  var dilation    = BinaryMorphology.dilation;
  var closing     = BinaryMorphology.closing;
  var difference  = BinaryMorphology.difference;
  var fill_ui8    = Uint8RasterGraphics.fill_into_selection;

  // step 2: dilate and take difference with (segments != i)
  var segment = Uint8Raster(vector_field.grid);
  var is_empty = Uint8Raster(vector_field.grid);
  var is_occupied = Uint8Raster(vector_field.grid);
  var plate_ids = Uint8Dataset.unique(segments);
  var plate_id = 0;
  for (var i = 0, li = plate_ids.length; i < li; ++i) {
  	plate_id = plate_ids[i]
    equals      (segments, plate_id,	segment);
    equals      (segments, 0,      	is_empty);
    not_equals  (segments, plate_id,	is_occupied);
    difference  (is_occupied, is_empty, is_occupied);
    dilation    (segment, 5,        	segment);
    closing     (segment, 5,        	segment);
    difference  (segment, is_occupied,  segment);
    fill_ui8    (segments, plate_id, segment, segments);
  }

  return segments;
}