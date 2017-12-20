'use strict';

// A "Crust" is defined as a set of rasters that represent a planet's crust
// The Crust namespace provides methods that extend the functionality of rasters.js to Crust objects
// It also provides functions for modeling properties of Crust
function Crust(params) {
	this.uuid = params['uuid'] || Uuid.create();
	this.grid = params['grid'] || stop('missing parameter: "grid"');

	// TODO:
	// * rename subductable/unsubductable to subductable/unsubductable
	// * record subductable/unsubductable in metric tons, not meters thickness
	// * switch densities to T/m^3

	// The following are the most fundamental fields to the tectonics model:

	this.unsubductable = Float32Raster(this.grid);
	// "unsubductable" is the thickness of the buoyant, unsubductable component of the crust
	// AKA "unsubductable", "felsic", or "continental" crust
	// Why don't we call it "continental" or some other name? Two reasons:
	//  1.) programmers will immediately understand what it does
	//  2.) we may want this model to simulate planets where alternate names don't apply, e.g. Pluto
	// unsubductable is a conserved quantity - it is never created or destroyed without our explicit say-so
	// This is to provide our model with a way to check for errors

	this.subductable = Float32Raster(this.grid);
	// "subductable" is the thickness of the denser, subductable component of the crust
	// AKA "subductable", "mafsic", or "oceanic" crust
	// Why don't we call it "oceanic" or some other name? Two reasons:
	//  1.) programmers will immediately understand what it does
	//  2.) we may want this model to simulate planets where alternate names don't apply, e.g. Pluto

	this.subductable_age = Float32Raster(this.grid);
	// the subductable_age of the subductable component of the crust
	// we don't track the subductable_age of unsubductable crust because it doesn't affect model behavior


	// The following are fields that are derived from other fields:
	this.displacement = Float32Raster(this.grid);
	// "displacement is the height of the crust relative to an arbitrary datum level
	// It is not called "elevation" to emphasize that it is not relative to sea level
	this.thickness = Float32Raster(this.grid);
	// the thickness of the crust in km
	this.density = Float32Raster(this.grid);
	// the average density of the crust, in kg/m^3
}
Crust.get_value = function(crust, i) {
	return new RockColumn({
		displacement 	:crust.displacement[i],
		thickness 		:crust.thickness[i],
		density 		:crust.density[i],
		subductable 			:crust.subductable[i],
		unsubductable 			:crust.unsubductable[i],
		subductable_age 			:crust.subductable_age[i],
	});
}
Crust.set_value = function(crust, i, rock_column) {
	crust.displacement[i]	= rock_column.displacement;
	crust.thickness[i] 		= rock_column.thickness;
	crust.density[i] 		= rock_column.density;
	crust.subductable[i] 			= rock_column.subductable;
	crust.unsubductable[i] 			= rock_column.unsubductable;
	crust.subductable_age[i] 			= rock_column.subductable_age;
}
Crust.copy = function(source, destination) {
	var copy = Float32Raster.copy;
	copy(source.displacement, destination.displacement);
	copy(source.thickness, destination.thickness);
	copy(source.density, destination.density);
	copy(source.subductable, destination.subductable);
	copy(source.unsubductable, destination.unsubductable);
	copy(source.subductable_age, destination.subductable_age);
}
Crust.fill = function(crust, rock_column) {
	var fill = Float32Raster.fill;
	fill(crust.displacement, rock_column.displacement);
	fill(crust.thickness, rock_column.thickness);
	fill(crust.density, rock_column.density);
	fill(crust.subductable, rock_column.subductable);
	fill(crust.unsubductable, rock_column.unsubductable);
	fill(crust.subductable_age, rock_column.subductable_age);
}
Crust.copy_into_selection = function(crust, copied_crust, selection_raster, result_crust) {
	var copy = Float32RasterGraphics.copy_into_selection;
	copy(source.displacement, copied_crust.displacement, selection_raster, result_crust.displacement);
	copy(source.thickness, copied_crust.thickness, selection_raster, result_crust.thickness);
	copy(source.density, copied_crust.density, selection_raster, result_crust.density);
	copy(source.subductable, copied_crust.subductable, selection_raster, result_crust.subductable);
	copy(source.unsubductable, copied_crust.unsubductable, selection_raster, result_crust.unsubductable);
	copy(source.subductable_age, copied_crust.subductable_age, selection_raster, result_crust.subductable_age);
}
Crust.fill_into_selection = function(crust, rock_column, selection_raster, result_crust) {
	var fill = Float32RasterGraphics.fill_into_selection;
	var fill_ui8 = Uint8Raster.fill;
	fill(crust.displacement, rock_column.displacement, selection_raster, result_crust.displacement);
	fill(crust.thickness, rock_column.thickness, selection_raster, result_crust.thickness);
	fill(crust.density, rock_column.density, selection_raster, result_crust.density);
	fill(crust.subductable, rock_column.subductable, selection_raster, result_crust.subductable);
	fill(crust.unsubductable, rock_column.unsubductable, selection_raster, result_crust.unsubductable);
	fill(crust.subductable_age, rock_column.subductable_age, selection_raster, result_crust.subductable_age);
}
